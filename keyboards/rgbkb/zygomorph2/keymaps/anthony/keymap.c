
    /* COLEMAK ModDH
     * .--------------------------------------------------------------.  .--------------------------------------------------------------.
     * | `~/Esc | 1      | 2      | 3      | 4      | 5      |        |  |        | 6      | 7      | 8      | 9      | 0      | Game   |
     * |--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
     * | Tab    | Q      | W      | F      | P      | B      |        |  |        | J      | L      | U      | Y      | ;      | \      |
     * |--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
     * | Caps   | A      | R      | S      | T      | G      |        |  |        | M      | N      | E      | I      | O      | '      |
     * |--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
     * | Ctrl   | Z      | X      | C      | D      | V      |        |  |        | N      | M      | ,      | .      | /      | Delete |
     * |--------+--------+--------+--------+--------+--------+--------|  |--------+--------+--------+--------+--------+--------+--------|
     * | `~/Fn  | Win    | Win    | Alt    | LShift | Space  | Bksp   |  | Enter  | Enter  | Bksp   | FN     | Alt    | Win    | Ctrl   |
     * "--------+--------+--------+--------+--------+--------+--------'  '--------+--------+--------+--------+--------+--------+--------'
     *
     */
#include QMK_KEYBOARD_H

enum sol_layers {
    _COLEMAK,
    _GAME,
    _FN,
    _ADJUST
};

enum sol_keycodes {
    COLEMAK = SAFE_RANGE,
    RGBRST = SAFE_RANGE,
    RGB_MENU,
    GAME
};

#define FN       MO(_FN)
#define ADJUST   MO(_ADJUST)
#define COLEMAK  DF(_COLEMAK)
#define GAME     TG(_GAME)
// #define QWERTY   DF(_QWERTY)
// #define FN_CAPS  LT(_FN, KC_CAPS)
#define FN_GRAVE LT(_FN, KC_GRAVE)
#define FN_QUOT  LT(_FN, KC_QUOT)
#define RC_SLASH RCTL_T(KC_SLASH)
//#define RGB_ADJ  LT(_ADJUST, RGB_TOG)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [_COLEMAK] = LAYOUT(
        KC_GESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,   KC_6,   KC_7,    KC_8,    KC_9,    KC_0,    GAME,
        KC_TAB,   KC_Q,    KC_W,    KC_F,    KC_P,    KC_B,   KC_J,   KC_L,    KC_U,    KC_Y,    KC_SCLN, KC_BSLS,
        KC_CAPS,  KC_A,    KC_R,    KC_S,    KC_T,    KC_G,   KC_M,   KC_N,    KC_E,    KC_I,    KC_O,    FN_QUOT,
        KC_LCTL,  KC_Z,    KC_X,    KC_C,    KC_D,    KC_V,   KC_K,   KC_H,    KC_COMM, KC_DOT,  KC_UP,   KC_DELETE,
        ADJUST,   KC_LGUI, KC_LALT, FN_GRAVE,KC_LSFT, KC_SPC, KC_ENT, KC_BSPC, RC_SLASH,KC_LEFT, KC_DOWN, KC_RGHT
    ),

    [_GAME] = LAYOUT(
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    _______,
        _______, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, _______,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, _______,
        KC_LCTL, _______, FN_GRAVE,KC_LALT, KC_SPC,  _______, _______, _______, _______, _______, _______, _______
    ),

    [_FN] = LAYOUT(
        _______, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,
        _______, _______, _______, KC_COLN, KC_LCBR, KC_RCBR, KC_PPLS, KC_EQL,  KC_UP,   _______, KC_PSCR, KC_F12,
        _______, _______, _______, KC_SCLN, KC_LPRN, KC_RPRN, KC_PMNS, KC_LEFT, KC_DOWN, KC_RGHT, _______, _______,
        _______, _______, _______, _______, KC_LBRC, KC_RBRC, _______, KC_PAST, KC_PSLS, _______, KC_VOLU, _______,
        _______, _______, _______, _______, _______, _______, KC_UNDS, _______, _______, _______, KC_VOLD, _______
    ),

    [_ADJUST] = LAYOUT(
        RESET,   KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  _______,
        RGBRST,  RGB_SAD, RGB_VAI, RGB_SAI, _______, KC_F11,  KC_F12,  KC_P7,   KC_P8,   KC_P9,   _______, _______,
        _______, RGB_HUD, RGB_VAD, RGB_HUI, _______, _______, _______, KC_P4,   KC_P5,   KC_P6,   _______, _______,
        _______, RGB_SPD, _______, RGB_SPI, _______, _______, _______, KC_P1,   KC_P2,   KC_P3,   _______, _______,
        _______, _______, _______, RGB_MOD, RGB_TOG, _______, _______, KC_P0,   KC_PDOT, KC_NLCK, _______, _______
    ),
};

// Default configuration: 3 tap zones, slide up, slide down
const uint16_t PROGMEM touch_encoders[][NUMBER_OF_TOUCH_ENCODERS][TOUCH_ENCODER_OPTIONS]  = {
    [_COLEMAK] = TOUCH_ENCODER_LAYOUT( \
        RGB_RMOD, RGB_TOG, RGB_MOD, RGB_HUI, RGB_HUD,
        RGB_RMOD, RGB_TOG, RGB_MOD, RGB_VAI, RGB_VAD
    ),
    [_GAME] = TOUCH_ENCODER_LAYOUT( \
        _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______
    ),
    [_FN] = TOUCH_ENCODER_LAYOUT( \
        _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______
    ),
    [_ADJUST] = TOUCH_ENCODER_LAYOUT( \
        _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______
    )
};

const uint16_t PROGMEM encoders[][NUMBER_OF_ENCODERS][ENCODER_OPTIONS]  = {
    [_COLEMAK] = ENCODER_LAYOUT( \
        KC_VOLU, KC_VOLD,
        KC_VOLU, KC_VOLD,
        KC_VOLU, KC_VOLD,
        KC_VOLU, KC_VOLD
    ),
    [_GAME] = ENCODER_LAYOUT( \
        KC_VOLU, KC_VOLD,
        KC_VOLU, KC_VOLD,
        KC_VOLU, KC_VOLD,
        KC_VOLU, KC_VOLD
    ),
    [_FN] = ENCODER_LAYOUT( \
        KC_VOLU, KC_VOLD,
        KC_VOLU, KC_VOLD,
        KC_VOLU, KC_VOLD,
        KC_VOLU, KC_VOLD
    ),
    [_ADJUST] = ENCODER_LAYOUT( \
        KC_VOLU, KC_VOLD,
        KC_VOLU, KC_VOLD,
        KC_VOLU, KC_VOLD,
        KC_VOLU, KC_VOLD
    )
};

#if defined(OLED_DRIVER_ENABLE)
uint16_t ovalue = 0;
uint32_t mcounter = 0;
uint32_t mvalue = 0;
uint16_t sec_timer = 1000;

void do_counters(void) {
    mcounter++;
    uint16_t now = sync_timer_read();
    if (timer_expired(now, sec_timer))
    {
        sec_timer += 1000;
        ovalue++;
        mvalue = (mvalue + mcounter) / 2;
        mcounter = 0;
    }
}

void matrix_scan_user(void) {
    do_counters();
}

void matrix_slave_scan_user(void) {
    do_counters();
}

void oled_task_user(void) {
    // Host Keyboard Layer Status
    oled_write_P(PSTR("Layer: "), false);

    switch (get_highest_layer(layer_state)) {
        case _COLEMAK:
            oled_write_P(PSTR("Default\n"), false);
            break;
        case _FN:
            oled_write_P(PSTR("FN\n"), false);
            break;
        case _ADJUST:
            oled_write_P(PSTR("ADJ\n"), false);
            break;
        default:
            // Or use the write_ln shortcut over adding '\n' to the end of your string
            oled_write_ln_P(PSTR("Undefined"), false);
    }

    // Host Keyboard LED Status
    led_t led_state = host_keyboard_led_state();
    oled_write_P(led_state.num_lock ? PSTR("NUM ") : PSTR("    "), false);
    oled_write_P(led_state.caps_lock ? PSTR("CAP ") : PSTR("    "), false);
    oled_write_ln_P(led_state.scroll_lock ? PSTR("SCR ") : PSTR("    "), false);

    static char buffer[6] = {0};
    snprintf(buffer, sizeof(buffer), "%5d", ovalue);
    oled_write_P(PSTR("\nTimer: "), false);
    oled_write_ln_P(buffer, false);

    snprintf(buffer, sizeof(buffer), "%5d", mvalue);
    oled_write_P(PSTR("\nScans: "), false);
    oled_write_ln_P(buffer, false);
}
#endif


// Sync
#include <print.h>
#include <string.h>
#include <transactions.h>
#include "syncfunctions.h"

user_runtime_config user_state;

void rpc_user_sync_callback(uint8_t initiator2target_buffer_size, const void *initiator2target_buffer, uint8_t target2initiator_buffer_size, void *target2initiator_buffer) {
    if (initiator2target_buffer_size == sizeof(user_state)) {
        memcpy(&user_state, initiator2target_buffer, sizeof(user_runtime_config));
    }
}

void keyboard_post_init_user(void) {
    // Register keyboard state sync split transaction
    transaction_register_rpc(RPC_ID_SYNC_STATE_USER, rpc_user_sync_callback);
}

void user_state_sync(void) {
    if (is_keyboard_master()) {
        // Keep track of the last state, so that we can tell if we need to propagate to slave
        static user_runtime_config last_user_state;
        static uint32_t            last_sync;
        bool                       needs_sync = false;

        // Check if the state values are different
        if (memcmp(&user_state, &last_user_state, sizeof(user_runtime_config))) {
            needs_sync = true;
            memcpy(&last_user_state, &user_state, sizeof(user_runtime_config));
        }

        // Send to slave every 1000ms regardless of state change
        if (timer_elapsed32(last_sync) > 1000) {
            needs_sync = true;
        }

        // Perform the sync if requested
        if (needs_sync) {
            last_sync = timer_read32();
            if (!transaction_rpc_send(RPC_ID_SYNC_STATE_USER, sizeof(user_runtime_config), &user_state)) {
                print("Failed to perform rpc call\n");
            }
        }
    } else {
        if (user_state.layer == _GAME) {
            set_game_layer_colors();
        } else if (user_state.layer == _FN) {
            set_fn_layer_colors();
        }

        if (user_state.caps_pressed) {
            set_alphakeys_color();
        }
    }
}

void housekeeping_task_user(void) {
    // Data sync from master to slave
    user_state_sync();
}

// For RGBRST Keycode
#if defined(RGB_MATRIX_ENABLE)
void rgb_matrix_increase_flags(void)
{
    switch (rgb_matrix_get_flags()) {
        case LED_FLAG_ALL: {
            rgb_matrix_set_flags(LED_FLAG_KEYLIGHT | LED_FLAG_MODIFIER);
            rgb_matrix_set_color_all(0, 0, 0);
            }
            break;
        case LED_FLAG_KEYLIGHT | LED_FLAG_MODIFIER: {
            rgb_matrix_set_flags(LED_FLAG_UNDERGLOW);
            rgb_matrix_set_color_all(0, 0, 0);
            }
            break;
        case LED_FLAG_UNDERGLOW: {
            rgb_matrix_set_flags(LED_FLAG_NONE);
            rgb_matrix_disable_noeeprom();
            }
            break;
        default: {
            rgb_matrix_set_flags(LED_FLAG_ALL);
            rgb_matrix_enable_noeeprom();
            }
            break;
    }
}

void rgb_matrix_decrease_flags(void)
{
    switch (rgb_matrix_get_flags()) {
        case LED_FLAG_ALL: {
            rgb_matrix_set_flags(LED_FLAG_NONE);
            rgb_matrix_disable_noeeprom();
            }
            break;
        case LED_FLAG_KEYLIGHT | LED_FLAG_MODIFIER: {
            rgb_matrix_set_flags(LED_FLAG_ALL);
            rgb_matrix_set_color_all(0, 0, 0);
            }
            break;
        case LED_FLAG_UNDERGLOW: {
            rgb_matrix_set_flags(LED_FLAG_KEYLIGHT | LED_FLAG_MODIFIER);
            rgb_matrix_set_color_all(0, 0, 0);
            }
            break;
        default: {
            rgb_matrix_set_flags(LED_FLAG_UNDERGLOW);
            rgb_matrix_enable_noeeprom();
            }
            break;
    }
}

void rgb_matrix_indicators_user(void) {
//   if (g_suspend_state || keyboard_config.disable_layer_led) { return; }
    int current_layer = biton32(layer_state);
    if (is_keyboard_master()) user_state.layer = current_layer;
    switch (current_layer) {
        case _GAME:
            set_game_layer_colors();
            break;
        case _FN:
            set_fn_layer_colors();
            break;
        default:
            if (rgb_matrix_get_flags() == LED_FLAG_NONE)
            rgb_matrix_set_color_all(0, 0, 0);
            break;
    }
    uint8_t this_led = host_keyboard_leds();

    if ( this_led & (1<<USB_LED_CAPS_LOCK)) {
        if (is_keyboard_master()) user_state.caps_pressed = true;
        set_alphakeys_color();
    } else {
        if (is_keyboard_master()) user_state.caps_pressed = false;
    }
}
#endif

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  static uint16_t reset_timer;
  switch (keycode) {
    case RGBRST:
#if defined(RGBLIGHT_ENABLE)
        if (record->event.pressed) {
          eeconfig_update_rgblight_default();
          rgblight_enable();
        }
#elif defined(RGB_MATRIX_ENABLE)
        if (record->event.pressed) {
          eeconfig_update_rgb_matrix_default();
        }
#endif
      return false;
    case RESET:
      if (record->event.pressed) {
          reset_timer = timer_read();
      } else {
          if (timer_elapsed(reset_timer) >= 500) {
              reset_keyboard();
          }
      }
      return false;
#if defined(RGB_MATRIX_ENABLE) && !defined(KEYBOARD_rgbkb_sol_rev1)
    case RGB_TOG:
      if (record->event.pressed) {
        rgb_matrix_increase_flags();
      }
      return false;
#endif
    case RGB_MENU:
#ifdef RGB_OLED_MENU
      if (record->event.pressed) {
        if (get_mods() & MOD_MASK_SHIFT) {
          rgb_encoder_state = (rgb_encoder_state - 1);
          if (rgb_encoder_state > 5) {
            rgb_encoder_state = 5;
          }
        } else {
          rgb_encoder_state = (rgb_encoder_state + 1) % 6;
        }
      }
#endif
      return false;
  }
  return true;
}

void suspend_power_down_kb(void) {
    rgb_matrix_disable_noeeprom();
    suspend_power_down_user();
}

 void suspend_wakeup_init_kb(void) {
    rgb_matrix_enable_noeeprom();
    suspend_wakeup_init_user();
}
