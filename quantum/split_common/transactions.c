/* Copyright 2021 QMK
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <stddef.h>

#include "config.h"
#include "matrix.h"
#include "quantum.h"
#include "transactions.h"
#include "transport.h"
#include "transaction_id_define.h"

#define SYNC_TIMER_OFFSET 2

#ifndef FORCED_SYNC_THROTTLE_MS
#    define FORCED_SYNC_THROTTLE_MS 100
#endif  // FORCED_SYNC_THROTTLE_MS

#define sizeof_member(type, member) sizeof(((type *)NULL)->member)

#define trans_initiator2target_initializer_cb(member, cb) \
    { &dummy, sizeof_member(split_shared_memory_t, member), offsetof(split_shared_memory_t, member), 0, 0, cb }
#define trans_initiator2target_initializer(member) trans_initiator2target_initializer_cb(member, NULL)

#define trans_target2initiator_initializer_cb(member, cb) \
    { &dummy, 0, 0, sizeof_member(split_shared_memory_t, member), offsetof(split_shared_memory_t, member), cb }
#define trans_target2initiator_initializer(member) trans_target2initiator_initializer_cb(member, NULL)

#define trans_bidirectional_initializer_cb(i2t_member, t2i_member, cb) \
    { &dummy, sizeof_member(split_shared_memory_t, i2t_member), offsetof(split_shared_memory_t, i2t_member), sizeof_member(split_shared_memory_t, t2i_member), offsetof(split_shared_memory_t, t2i_member), cb }
#define trans_bidirectional_initializer(i2t_member, t2i_member) trans_target2initiator_initializer_cb(i2t_member, t2i_member, NULL)

static uint8_t crc8(const void *data, size_t len) {
    const uint8_t *p   = (const uint8_t *)data;
    uint8_t        crc = 0xff;
    size_t         i, j;
    for (i = 0; i < len; i++) {
        crc ^= p[i];
        for (j = 0; j < 8; j++) {
            if ((crc & 0x80) != 0)
                crc = (uint8_t)((crc << 1) ^ 0x31);
            else
                crc <<= 1;
        }
    }
    return crc;
}

uint8_t                  dummy;
split_transaction_desc_t split_transaction_table[NUM_TOTAL_TRANSACTIONS] = {
    // Set defaults
    [0 ...(NUM_TOTAL_TRANSACTIONS - 1)] = {NULL, 0, 0, 0, 0, 0},

    [GET_SLAVE_MATRIX_CHECKSUM] = trans_target2initiator_initializer(smatrix.checksum),
    [GET_SLAVE_MATRIX_DATA]     = trans_target2initiator_initializer(smatrix.matrix),

#ifdef SPLIT_TRANSPORT_MIRROR
    [PUT_MASTER_MATRIX] = trans_initiator2target_initializer(mmatrix.matrix),
#endif  // SPLIT_TRANSPORT_MIRROR

#ifdef ENCODER_ENABLE
    [GET_ENCODERS_CHECKSUM] = trans_target2initiator_initializer(encoders.checksum),
    [GET_ENCODERS_DATA]     = trans_target2initiator_initializer(encoders.state),
#endif  // ENCODER_ENABLE

#ifndef DISABLE_SYNC_TIMER
    [PUT_SYNC_TIMER] = trans_initiator2target_initializer(sync_timer),
#endif  // DISABLE_SYNC_TIMER

#ifndef DISABLE_SYNC_LAYER_STATE
    [PUT_LAYER_STATE] = trans_initiator2target_initializer(layer_state),
#endif  // DISABLE_SYNC_LAYER_STATE

#ifndef DISABLE_SYNC_LED_STATE
    [PUT_LED_STATE] = trans_initiator2target_initializer(led_state),
#endif  // DISABLE_SYNC_LED_STATE

#ifdef SPLIT_MODS_ENABLE
    [PUT_MODS] = trans_initiator2target_initializer(mods),
#endif  // SPLIT_MODS_ENABLE

#ifdef BACKLIGHT_ENABLE
    [PUT_BACKLIGHT] = trans_initiator2target_initializer(backlight_level),
#endif  // BACKLIGHT_ENABLE

#if defined(RGBLIGHT_ENABLE) && defined(RGBLIGHT_SPLIT)
    [PUT_RGBLIGHT] = trans_initiator2target_initializer(rgblight_sync),
#endif  // defined(RGBLIGHT_ENABLE) && defined(RGBLIGHT_SPLIT)

#if defined(RGB_MATRIX_ENABLE) && defined(RGB_MATRIX_SPLIT)
    [PUT_RGB_MATRIX] = trans_initiator2target_initializer(rgb_matrix_sync),
#endif  // defined(RGBLIGHT_ENABLE) && defined(RGBLIGHT_SPLIT)

#ifdef WPM_ENABLE
    [PUT_WPM] = trans_initiator2target_initializer(current_wpm),
#endif  // WPM_ENABLE
};

#define transport_write(id, data, length) transport_execute_transaction(id, data, length, NULL, 0)
#define transport_read(id, data, length) transport_execute_transaction(id, NULL, 0, data, length)

bool transactions_master(matrix_row_t master_matrix[], matrix_row_t slave_matrix[]) {
    bool okay = true;

    // Always read the slave matrix.
    static uint32_t last_smatrix_update = 0;
    uint8_t         curr_checksum;
    okay &= transport_read(GET_SLAVE_MATRIX_CHECKSUM, &curr_checksum, sizeof(curr_checksum));
    if (okay && (curr_checksum != crc8(split_shmem->smatrix.matrix, sizeof(split_shmem->smatrix.matrix)) || timer_elapsed32(last_smatrix_update) >= FORCED_SYNC_THROTTLE_MS)) {
        okay &= transport_read(GET_SLAVE_MATRIX_DATA, slave_matrix, sizeof(split_shmem->smatrix.matrix));
        okay &= curr_checksum == crc8(split_shmem->smatrix.matrix, sizeof(split_shmem->smatrix.matrix));
        if (okay) {
            last_smatrix_update = timer_read32();
        }
    } else {
        memcpy(slave_matrix, split_shmem->smatrix.matrix, sizeof(split_shmem->smatrix.matrix));
    }

#ifdef SPLIT_TRANSPORT_MIRROR
    static uint32_t last_mmatrix_update = 0;
    if (timer_elapsed32(last_mmatrix_update) >= FORCED_SYNC_THROTTLE_MS || memcmp(master_matrix, split_shmem->mmatrix.matrix, sizeof(split_shmem->mmatrix.matrix)) != 0) {
        last_mmatrix_update = timer_read32();
        okay &= transport_write(PUT_MASTER_MATRIX, master_matrix, sizeof(split_shmem->mmatrix.matrix));
    }
#endif  // SPLIT_TRANSPORT_MIRROR

#ifdef ENCODER_ENABLE
    static uint32_t last_encoder_update = 0;
    okay &= transport_read(GET_ENCODERS_CHECKSUM, &curr_checksum, sizeof(curr_checksum));
    if (okay && (curr_checksum != crc8(split_shmem->encoders.state, sizeof(split_shmem->encoders.state)) || timer_elapsed32(last_encoder_update) >= FORCED_SYNC_THROTTLE_MS)) {
        okay &= transport_read(GET_ENCODERS_DATA, split_shmem->encoders.state, sizeof(split_shmem->encoders.state));
        okay &= curr_checksum == crc8(split_shmem->encoders.state, sizeof(split_shmem->encoders.state));
        if (okay) {
            last_encoder_update = timer_read32();
        }
    }
    encoder_update_raw(split_shmem->encoders.state);
#endif  // ENCODER_ENABLE

#ifndef DISABLE_SYNC_TIMER
    static uint32_t last_sync_timer_update = 0;
    if (timer_elapsed32(last_sync_timer_update) >= FORCED_SYNC_THROTTLE_MS) {
        last_sync_timer_update = timer_read32();
        uint32_t sync_timer    = sync_timer_read32() + SYNC_TIMER_OFFSET;
        okay &= transport_write(PUT_SYNC_TIMER, &sync_timer, sizeof(sync_timer));
    }
#endif  // DISABLE_SYNC_TIMER

#ifndef DISABLE_SYNC_LAYER_STATE
    static uint32_t last_layer_state_update = 0;
    if (layer_state != split_shmem->layer_state || timer_elapsed32(last_layer_state_update) >= FORCED_SYNC_THROTTLE_MS) {
        last_layer_state_update = timer_read32();
        okay &= transport_write(PUT_LAYER_STATE, &layer_state, sizeof(layer_state));
    }
#endif  // DISABLE_SYNC_LAYER_STATE

#ifndef DISABLE_SYNC_LED_STATE
    static uint32_t last_led_state_update = 0;
    uint8_t         led_state             = host_keyboard_leds();
    if (led_state != split_shmem->led_state || timer_elapsed32(last_led_state_update) >= FORCED_SYNC_THROTTLE_MS) {
        last_led_state_update = timer_read32();
        okay &= transport_write(PUT_LED_STATE, &led_state, sizeof(led_state));
    }
#endif  // DISABLE_SYNC_LED_STATE

#ifdef SPLIT_MODS_ENABLE
    static uint32_t   last_mod_update = 0;
    bool              mods_need_sync  = timer_elapsed32(last_mod_update) >= FORCED_SYNC_THROTTLE_MS;
    split_mods_sync_t new_mods;
    new_mods.real_mods = get_mods();
    if (new_mods.real_mods != split_shmem->mods.real_mods) {
        mods_need_sync = true;
    }

    new_mods.weak_mods = get_weak_mods();
    if (new_mods.weak_mods != split_shmem->mods.weak_mods) {
        mods_need_sync = true;
    }

#    ifndef NO_ACTION_ONESHOT
    new_mods.oneshot_mods = get_oneshot_mods();
    if (new_mods.oneshot_mods != split_shmem->mods.oneshot_mods) {
        mods_need_sync = true;
    }
#    endif  // NO_ACTION_ONESHOT

    if (mods_need_sync) {
        last_mod_update = timer_read32();
        okay &= transport_write(PUT_MODS, &new_mods, sizeof(new_mods));
    }
#endif  // SPLIT_MODS_ENABLE

#ifdef BACKLIGHT_ENABLE
    static uint32_t last_backlight_update = 0;
    uint8_t         level                 = is_backlight_enabled() ? get_backlight_level() : 0;
    if (timer_elapsed32(last_backlight_update) >= FORCED_SYNC_THROTTLE_MS || split_shmem->backlight_level != level) {
        last_backlight_update = timer_read32();
        okay &= transport_write(PUT_BACKLIGHT, &level, sizeof(level));
    }
#endif  // BACKLIGHT_ENABLE

#if defined(RGBLIGHT_ENABLE) && defined(RGBLIGHT_SPLIT)
    static uint32_t last_rgblight_update = 0;
    if (timer_elapsed32(last_rgblight_update) >= FORCED_SYNC_THROTTLE_MS || rgblight_get_change_flags()) {
        last_rgblight_update = timer_read32();
        rgblight_syncinfo_t rgblight_sync;
        rgblight_get_syncinfo(&rgblight_sync);
        if (transport_write(PUT_RGBLIGHT, &rgblight_sync, sizeof(rgblight_sync))) {
            rgblight_clear_change_flags();
        } else {
            okay = false;
        }
    }
#endif  // defined(RGBLIGHT_ENABLE) && defined(RGBLIGHT_SPLIT)

#if defined(RGB_MATRIX_ENABLE) && defined(RGB_MATRIX_SPLIT)
    static uint32_t   last_rgb_matrix_update = 0;
    rgb_matrix_sync_t rgb_matrix_sync;
    memcpy(&rgb_matrix_sync.rgb_matrix, &rgb_matrix_config, sizeof(rgb_config_t));
    rgb_matrix_sync.rgb_suspend_state = g_suspend_state;
    bool needs_update                 = memcmp(&rgb_matrix_sync, &split_shmem->rgb_matrix_sync, sizeof(rgb_matrix_sync_t)) != 0;
    if (needs_update || timer_elapsed32(last_rgb_matrix_update) > FORCED_SYNC_THROTTLE_MS) {
        last_rgb_matrix_update = timer_read32();
        okay &= transport_write(PUT_RGB_MATRIX, &rgb_matrix_sync, sizeof(rgb_matrix_sync));
    }
#endif  // defined(RGBLIGHT_ENABLE) && defined(RGBLIGHT_SPLIT)

#ifdef WPM_ENABLE
    static uint32_t last_wpm_update = 0;
    if (timer_elapsed32(last_wpm_update) > FORCED_SYNC_THROTTLE_MS) {
        last_wpm_update     = timer_read32();
        uint8_t current_wpm = get_current_wpm();
        if (current_wpm != split_shmem->current_wpm) {
            okay &= transport_write(PUT_WPM, &current_wpm, sizeof(current_wpm));
        }
    }
#endif  // WPM_ENABLE

    return okay;
}

void transactions_slave(matrix_row_t master_matrix[], matrix_row_t slave_matrix[]) {
    // Always prepare the slave matrix for read.
    memcpy(split_shmem->smatrix.matrix, slave_matrix, sizeof(split_shmem->smatrix.matrix));
    // Now update the checksum given that the slave matrix has been written to
    split_shmem->smatrix.checksum = crc8(split_shmem->smatrix.matrix, sizeof(split_shmem->smatrix.matrix));

#ifdef SPLIT_TRANSPORT_MIRROR
    memcpy(master_matrix, split_shmem->mmatrix.matrix, sizeof(split_shmem->mmatrix.matrix));
#endif

#ifdef ENCODER_ENABLE
    uint8_t encoder_state[NUMBER_OF_ENCODERS];
    encoder_state_raw(encoder_state);
    // Always prepare the encoder state for read.
    memcpy(split_shmem->encoders.state, encoder_state, sizeof(encoder_state));
    // Now update the checksum given that the encoders has been written to
    split_shmem->encoders.checksum = crc8(split_shmem->encoders.state, sizeof(split_shmem->encoders.state));
#endif

#ifndef DISABLE_SYNC_TIMER
    static uint32_t last_sync_timer = 0;
    if (last_sync_timer != split_shmem->sync_timer) {
        last_sync_timer = split_shmem->sync_timer;
        sync_timer_update(split_shmem->sync_timer);
    }
#endif

#ifndef DISABLE_SYNC_LAYER_STATE
    layer_state = split_shmem->layer_state;
#endif  // DISABLE_SYNC_LAYER_STATE

#ifndef DISABLE_SYNC_LED_STATE
    void set_split_host_keyboard_leds(uint8_t led_state);
    set_split_host_keyboard_leds(split_shmem->led_state);
#endif  // DISABLE_SYNC_LED_STATE

#ifdef SPLIT_MODS_ENABLE
    set_mods(split_shmem->mods.real_mods);
    set_weak_mods(split_shmem->mods.weak_mods);
#    ifndef NO_ACTION_ONESHOT
    set_oneshot_mods(split_shmem->mods.oneshot_mods);
#    endif
#endif

#ifdef BACKLIGHT_ENABLE
    backlight_set(split_shmem->backlight_level);
#endif

#if defined(RGBLIGHT_ENABLE) && defined(RGBLIGHT_SPLIT)
    // Update the RGB with the new data
    if (split_shmem->rgblight_sync.status.change_flags != 0) {
        rgblight_update_sync(&split_shmem->rgblight_sync, false);
        split_shmem->rgblight_sync.status.change_flags = 0;
    }
#endif

#if defined(RGB_MATRIX_ENABLE) && defined(RGB_MATRIX_SPLIT)
    memcpy(&rgb_matrix_config, &split_shmem->rgb_matrix_sync.rgb_matrix, sizeof(rgb_config_t));
    g_suspend_state = split_shmem->rgb_matrix_sync.rgb_suspend_state;
#endif

#ifdef WPM_ENABLE
    set_current_wpm(split_shmem->current_wpm);
#endif
}
