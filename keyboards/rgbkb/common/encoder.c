/*
 * Copyright 2018 Jack Humbert <jack.humb@gmail.com>
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

#include "encoder.h"
#ifdef SPLIT_KEYBOARD
#    include "split_util.h"
#endif

// for memcpy
#include <string.h>

#if !defined(ENCODER_RESOLUTIONS) && !defined(ENCODER_RESOLUTION)
#    define ENCODER_RESOLUTION 4
#endif

#if !defined(ENCODERS_PAD_A) || !defined(ENCODERS_PAD_B)
#    error "No encoder pads defined by ENCODERS_PAD_A and ENCODERS_PAD_B"
#endif

static pin_t encoders_pad_a[] = ENCODERS_PAD_A;
static pin_t encoders_pad_b[] = ENCODERS_PAD_B;
#ifdef ENCODER_RESOLUTIONS
static uint8_t encoder_resolutions[] = ENCODER_RESOLUTIONS;
#endif

#ifdef ENCODERS_PAD_C
    #define NUMBER_OF_ENCODERS_AB (sizeof(encoders_pad_a) / sizeof(pin_t))
    #define NUMBER_OF_ENCODERS_C (sizeof(encoders_pad_c) / sizeof(pin_t))
    #define NUMBER_OF_ENCODERS (NUMBER_OF_ENCODERS_C * NUMBER_OF_ENCODERS_AB)
    static pin_t encoders_pad_c[] = ENCODERS_PAD_C;
#else
    #define NUMBER_OF_ENCODERS (sizeof(encoders_pad_a) / sizeof(pin_t))
    #define NUMBER_OF_ENCODERS_AB (sizeof(encoders_pad_a) / sizeof(pin_t))
#endif

#ifndef ENCODER_DIRECTION_FLIP
#    define ENCODER_CLOCKWISE true
#    define ENCODER_COUNTER_CLOCKWISE false
#else
#    define ENCODER_CLOCKWISE false
#    define ENCODER_COUNTER_CLOCKWISE true
#endif
static int8_t encoder_LUT[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

static uint8_t encoder_state[NUMBER_OF_ENCODERS]  = {0};
static int8_t  encoder_pulses[NUMBER_OF_ENCODERS] = {0};

#ifdef SPLIT_KEYBOARD
// right half encoders come over as second set of encoders
static uint8_t encoder_value[NUMBER_OF_ENCODERS * 2] = {0};
// row offsets for each hand
static uint8_t thisHand, thatHand;
#else
static uint8_t encoder_value[NUMBER_OF_ENCODERS] = {0};
#endif

__attribute__((weak)) void encoder_update_user(int8_t index, bool clockwise) {}

__attribute__((weak)) void encoder_update_kb(int8_t index, bool clockwise) { encoder_update_user(index, clockwise); }

void encoder_init(void) {
#if defined(SPLIT_KEYBOARD)
    if (!isLeftHand) {
#    if defined(ENCODERS_PAD_A_RIGHT) && defined(ENCODERS_PAD_B_RIGHT)
        const pin_t encoders_pad_a_right[] = ENCODERS_PAD_A_RIGHT;
        const pin_t encoders_pad_b_right[] = ENCODERS_PAD_B_RIGHT;
#       if defined(ENCODER_RESOLUTIONS_RIGHT)
        const uint8_t encoder_resolutions_right[] = ENCODER_RESOLUTIONS_RIGHT;
#       endif
        for (uint8_t i = 0; i < NUMBER_OF_ENCODERS_AB; i++) {
            encoders_pad_a[i] = encoders_pad_a_right[i];
            encoders_pad_b[i] = encoders_pad_b_right[i];
#       if defined(ENCODER_RESOLUTIONS_RIGHT)
            encoder_resolutions[i] = encoder_resolutions_right[i];
#       endif
        }
#    endif
#    if defined(ENCODERS_PAD_C_RIGHT)
        const pin_t encoders_pad_c_right[] = ENCODERS_PAD_C_RIGHT;
        for (uint8_t i = 0; i < NUMBER_OF_ENCODERS_C; i++) {
            encoders_pad_c[i] = encoders_pad_c_right[i];
        }
#    endif
    }
#endif

#ifdef ENCODERS_PAD_C
    for (int i = 0; i < NUMBER_OF_ENCODERS_C; i++) {
        setPinOutput(encoders_pad_c[i]);
        writePinHigh(encoders_pad_c[i]);
    }
    for (int i = 0; i < NUMBER_OF_ENCODERS_AB; i++) {
        setPinInputHigh(encoders_pad_a[i]);
        setPinInputHigh(encoders_pad_b[i]);
    }
    for (int j = 0; j < NUMBER_OF_ENCODERS_C; j++) {
        writePinLow(encoders_pad_c[j]);
        wait_us(10);
        for (int i = 0; i < NUMBER_OF_ENCODERS_AB; i++) {
            encoder_state[j + (i * NUMBER_OF_ENCODERS_C)] = (readPin(encoders_pad_a[i]) << 0) | (readPin(encoders_pad_b[i]) << 1);
        }
        writePinHigh(encoders_pad_c[j]);
    }
    // need to disable these pins to prevent matrix activation
    for (int i = 0; i < NUMBER_OF_ENCODERS_AB; i++) {
        setPinInput(encoders_pad_a[i]);
        setPinInput(encoders_pad_b[i]);
    }
    // And reset all the row pins
    for (int i = 0; i < NUMBER_OF_ENCODERS_C; i++) {
        setPinInputHigh(encoders_pad_c[i]);
    }

#else

    for (int i = 0; i < NUMBER_OF_ENCODERS_AB; i++) {
        setPinInputHigh(encoders_pad_a[i]);
        setPinInputHigh(encoders_pad_b[i]);

        encoder_state[i] = (readPin(encoders_pad_a[i]) << 0) | (readPin(encoders_pad_b[i]) << 1);
    }

#endif

#ifdef SPLIT_KEYBOARD
    thisHand = isLeftHand ? 0 : NUMBER_OF_ENCODERS;
    thatHand = NUMBER_OF_ENCODERS - thisHand;
#endif
}

static bool encoder_update(int8_t index, uint8_t state) {
    bool    changed = false;
    uint8_t i       = index;

#ifdef ENCODER_RESOLUTIONS
    int8_t resolution = encoder_resolutions[i];
#else
    int8_t resolution = ENCODER_RESOLUTION;
#endif

#ifdef SPLIT_KEYBOARD
    index += thisHand;
#endif
    encoder_pulses[i] += encoder_LUT[state & 0xF];
    if (encoder_pulses[i] >= resolution) {
        encoder_value[index]++;
        changed = true;
        encoder_update_kb(index, ENCODER_COUNTER_CLOCKWISE);
    }
    if (encoder_pulses[i] <= -resolution) {  // direction is arbitrary here, but this clockwise
        encoder_value[index]--;
        changed = true;
        encoder_update_kb(index, ENCODER_CLOCKWISE);
    }
    encoder_pulses[i] %= resolution;
    return changed;
}

bool encoder_read(void) {
    bool changed = false;
#ifdef ENCODERS_PAD_C
    // setup row pins to act as C pins for the encoders, prep the first one
    for (uint8_t i = 0; i < NUMBER_OF_ENCODERS_C; i++) {
        setPinOutput(encoders_pad_c[i]);
        writePinHigh(encoders_pad_c[i]);
    }
    // pull these back up because we disabled them earlier
    for (int i = 0; i < NUMBER_OF_ENCODERS_AB; i++) {
        setPinInputHigh(encoders_pad_a[i]);
        setPinInputHigh(encoders_pad_b[i]);
    }
    for (int j = 0; j < NUMBER_OF_ENCODERS_C; j++) {
        writePinLow(encoders_pad_c[j]);
        wait_us(10);
        for (int i = 0; i < NUMBER_OF_ENCODERS_AB; i++) {
            encoder_state[j + (i * NUMBER_OF_ENCODERS_C)] <<= 2;
            encoder_state[j + (i * NUMBER_OF_ENCODERS_C)] |= (readPin(encoders_pad_a[i]) << 0) | (readPin(encoders_pad_b[i]) << 1);
            changed |= encoder_update(j + (i * NUMBER_OF_ENCODERS_C), encoder_state[j + (i * NUMBER_OF_ENCODERS_C)]);
        }
        writePinHigh(encoders_pad_c[j]);
    }
    // need to disable these pins again to prevent matrix activation
    for (int i = 0; i < NUMBER_OF_ENCODERS_AB; i++) {
        setPinInput(encoders_pad_a[i]);
        setPinInput(encoders_pad_b[i]);
    }
    // revert row pins back to input
    for (int i = 0; i < NUMBER_OF_ENCODERS_C; i++) {
        setPinInputHigh(encoders_pad_c[i]);
    }
#else
    for (uint8_t i = 0; i < NUMBER_OF_ENCODERS_AB; i++) {
        encoder_state[i] <<= 2;
        encoder_state[i] |= (readPin(encoders_pad_a[i]) << 0) | (readPin(encoders_pad_b[i]) << 1);
        changed |= encoder_update(i, encoder_state[i]);
    }
#endif
    return changed;
}

#ifdef SPLIT_KEYBOARD
void last_encoder_activity_trigger(void);

void encoder_state_raw(uint8_t* slave_state) { memcpy(slave_state, &encoder_value[thisHand], sizeof(uint8_t) * NUMBER_OF_ENCODERS); }

void encoder_update_raw(uint8_t* slave_state) {
    bool changed = false;
    for (uint8_t i = 0; i < NUMBER_OF_ENCODERS; i++) {
        uint8_t index = i + thatHand;
        int8_t  delta = slave_state[i] - encoder_value[index];
        while (delta > 0) {
            delta--;
            encoder_value[index]++;
            changed = true;
            encoder_update_kb(index, ENCODER_COUNTER_CLOCKWISE);
        }
        while (delta < 0) {
            delta++;
            encoder_value[index]--;
            changed = true;
            encoder_update_kb(index, ENCODER_CLOCKWISE);
        }
    }

    // Update the last encoder input time -- handled external to encoder_read() when we're running a split
    if (changed) last_encoder_activity_trigger();
}
#endif
