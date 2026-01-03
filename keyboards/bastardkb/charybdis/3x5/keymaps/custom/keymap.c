/**
 * Copyright 2021 Charly Delay <charly@codesink.dev> (@0xcharly)
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
#include QMK_KEYBOARD_H

#ifdef CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_ENABLE
#    include "timer.h"
#endif // CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_ENABLE

enum charybdis_keymap_layers {
    LAYER_BASE = 0,
    LAYER_AUX,
    LAYER_SYMB,
    LAYER_SYMB_SHIFT,
    LAYER_FNC,
    LAYER_POINTER,
};

// Automatically enable sniping-mode on the pointer layer.
#define CHARYBDIS_AUTO_SNIPING_ON_LAYER LAYER_POINTER

#ifdef CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_ENABLE
static uint16_t auto_pointer_layer_timer = 0;

#    ifndef CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_TIMEOUT_MS
#        define CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_TIMEOUT_MS 1000
#    endif // CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_TIMEOUT_MS

#    ifndef CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_THRESHOLD
#        define CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_THRESHOLD 8
#    endif // CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_THRESHOLD
#endif     // CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_ENABLE

#ifndef POINTING_DEVICE_ENABLE
#    define DRGSCRL KC_NO
#    define DPI_MOD KC_NO
#    define S_D_MOD KC_NO
#    define SNIPING KC_NO
#endif // !POINTING_DEVICE_ENABLE

// Custom definitions
enum {
    TD_L_THUMB = 0,
    TD_R_THUMB
};

// Forward declare callbacks
void td_l_thumb_finished(tap_dance_state_t *state, void *user_data);
void td_l_thumb_reset(tap_dance_state_t *state, void *user_data);
void td_r_thumb_finished(tap_dance_state_t *state, void *user_data);
void td_r_thumb_reset(tap_dance_state_t *state, void *user_data);

// Tap Dance Definitions

// State tracking for our custom tap dances
typedef struct {
    bool is_held;
} td_thumb_state_t;

static td_thumb_state_t td_l_state = { .is_held = false };
static td_thumb_state_t td_r_state = { .is_held = false };

// Manual OSL State
static bool osl_fnc_active = false;
static bool osl_symb_shift_active = false;
static bool cleanup_fnc_layer = false;
static bool cleanup_symb_layer = false;

void td_l_thumb_finished(tap_dance_state_t *state, void *user_data) {
    td_l_state.is_held = false;
    if (state->count == 1) {
        if (state->pressed) { // Hold
            layer_on(LAYER_AUX);
            td_l_state.is_held = true;
        } else { // Tap
            layer_on(LAYER_FNC);
            osl_fnc_active = true;
        }
    }
}

void td_l_thumb_reset(tap_dance_state_t *state, void *user_data) {
    if (td_l_state.is_held) {
        layer_off(LAYER_AUX);
        td_l_state.is_held = false;
    }
}

void td_r_thumb_finished(tap_dance_state_t *state, void *user_data) {
    td_r_state.is_held = false;
    if (state->count == 1) {
        if (state->pressed) { // Hold
            layer_on(LAYER_SYMB);
            td_r_state.is_held = true;
        } else { // Tap
            layer_on(LAYER_SYMB_SHIFT);
            osl_symb_shift_active = true;
        }
    }
}

void td_r_thumb_reset(tap_dance_state_t *state, void *user_data) {
    if (td_r_state.is_held) {
        layer_off(LAYER_SYMB);
        td_r_state.is_held = false;
    }
}

tap_dance_action_t tap_dance_actions[] = {
    [TD_L_THUMB] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, td_l_thumb_finished, td_l_thumb_reset),
    [TD_R_THUMB] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, td_r_thumb_finished, td_r_thumb_reset)
};

// Update Thumb Definitions
#define LMT_AUX  TD(TD_L_THUMB)
#define RMT_SYMB TD(TD_R_THUMB)

// Helpers for readability
#define _L_PTR(KC) LT(LAYER_POINTER, KC)
#define SFT_SPC    LSFT_T(KC_SPC)
#define SFT_ENT    LSFT_T(KC_ENT)
#define SFT_BSP    LSFT_T(KC_BSPC)
#define SFT_LRET   LSFT_T(KC_ENT) // Shift+Enter

// ZMK uses "flavor = balanced" for home row mods.
#define CTL_X LCTL_T(KC_X)
#define ALT_C LALT_T(KC_C)
#define GUI_V LGUI_T(KC_V)
#define GUI_M RGUI_T(KC_M)
#define ALT_CM RALT_T(KC_COMM)
#define CTL_DT RCTL_T(KC_DOT)

// Layout Definitions

// Combos
enum combo_events {
  JK_ENT_COMBO,
  WE_ESC_COMBO,
  LS_DEL_COMBO,
  COMBO_LENGTH
};

uint16_t PROGMEM jk_combo[] = {KC_J, KC_K, COMBO_END};
uint16_t PROGMEM we_combo[] = {KC_W, KC_E, COMBO_END};
uint16_t PROGMEM ls_combo[] = {KC_L, KC_SCLN, COMBO_END};

combo_t key_combos[COMBO_LENGTH] = {
  [JK_ENT_COMBO] = COMBO(jk_combo, KC_ENT),
  [WE_ESC_COMBO] = COMBO(we_combo, KC_ESC),
  [LS_DEL_COMBO] = COMBO(ls_combo, LCTL(KC_BSPC)),
};

// LAYER_BASE (ZMK layer0)
// Q  W  E  R  T     Y  U  I  O  P
// A  S  D  F  G     H  J  K  L  ;
// Z  X  C  V  B     N  M  ,  .  /
// Thumbs: LMT, SFT_SPC, XXXXX | SFT_SPC, RMT
#define LAYOUT_LAYER_BASE \
    KC_Q,   KC_W,   KC_E,   KC_R,   KC_T,        KC_Y,   KC_U,   KC_I,    KC_O,    KC_P,    \
    KC_A,   KC_S,   KC_D,   KC_F,   KC_G,        KC_H,   KC_J,   KC_K,    KC_L,    KC_SCLN, \
    KC_Z,   CTL_X,  ALT_C,  GUI_V,  KC_B,        KC_N,   GUI_M,  ALT_CM,  CTL_DT,  KC_SLSH, \
            XXXXXXX, LMT_AUX, SFT_SPC,        SFT_SPC, RMT_SYMB

// LAYER_AUX (ZMK layer_1)
#define LAYOUT_LAYER_AUX \
    KC_BRID, KC_BRIU, KC_MUTE, KC_VOLD, KC_VOLU,     KC_WH_U, KC_AMPR, KC_ASTR, XXXXXXX, XXXXXXX, \
    KC_TAB,  XXXXXXX, XXXXXXX, XXXXXXX, KC_PGUP,     KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, KC_QUOT, \
    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_PGDN,     KC_WH_D, KC_EXLM, KC_AT,   KC_HASH, KC_MINS, \
             XXXXXXX, _______, SFT_ENT,          SFT_ENT, KC_ENT

// LAYER_SYMB (ZMK layer_2)
#define LAYOUT_LAYER_SYMB \
    XXXXXXX, KC_QUOT, KC_LBRC, KC_RBRC, KC_PIPE,          KC_BSLS, KC_7, KC_8, KC_9, KC_PLUS, \
    KC_TAB,  XXXXXXX, KC_LCBR, KC_RCBR, XXXXXXX,          KC_EQL,  KC_4, KC_5, KC_6, KC_0,    \
    KC_GRV,  XXXXXXX, MT(MOD_LALT, KC_LPRN), MT(MOD_LGUI, KC_RPRN), KC_PIPE, KC_UNDS, KC_1, KC_2, KC_3, KC_MINS, \
             XXXXXXX, KC_BSPC, SFT_BSP,               SFT_BSP, _______ 

// LAYER_SYMB_SHIFT (ZMK layer_3)
#define LAYOUT_LAYER_SYMB_SHIFT \
    XXXXXXX, KC_DQT,  KC_LBRC, KC_RBRC, KC_PIPE,          KC_BSLS,    KC_AMPR, KC_ASTR, KC_LPRN, KC_PLUS, \
    KC_TAB,  XXXXXXX, KC_LCBR, KC_RCBR, XXXXXXX,          KC_EQL,     KC_DLR,  KC_PERC, KC_CIRC, KC_QUOT, \
    KC_TILD, XXXXXXX, KC_LPRN, KC_RPRN, KC_PIPE,          KC_UNDS,    KC_EXLM, LSFT(KC_2), KC_HASH, KC_MINS, \
             XXXXXXX, LALT(KC_BSPC), LSFT(LALT(KC_BSPC)),          LSFT(LALT(KC_BSPC)), _______ 

// LAYER_FNC (ZMK layer_4)
#define LAYOUT_LAYER_FNC \
    XXXXXXX, KC_F7, KC_F8, KC_F9, KC_F10,          XXXXXXX, KC_AMPR, KC_ASTR, KC_LPRN, KC_PLUS, \
    KC_TAB,  KC_F4, KC_F5, KC_F6, KC_F11,          KC_EQL,  KC_DLR,  KC_PERC, KC_CIRC, KC_QUOT, \
    XXXXXXX, KC_F1, KC_F2, KC_F3, KC_F12,          KC_BSLS, KC_EXLM, KC_AT,   KC_HASH, KC_MINS, \
             XXXXXXX, _______, SFT_LRET,           SFT_LRET, LSFT(KC_ENT) 

// LAYER_POINTER (Existing)
#define LAYOUT_LAYER_POINTER_CUSTOM \
    QK_BOOT,  EE_CLR, XXXXXXX, DPI_MOD, S_D_MOD, S_D_MOD, DPI_MOD, XXXXXXX,  EE_CLR, QK_BOOT, \
    ______________HOME_ROW_GACS_L______________, ______________HOME_ROW_GACS_R______________, \
    _______, DRGSCRL, SNIPING, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, SNIPING, DRGSCRL, _______, \
                      XXXXXXX, KC_BTN2, KC_BTN1, KC_BTN3, KC_BTN1

// Need to keep the convenience macros for POINTER layer to work or redefine them
#define ______________HOME_ROW_GACS_L______________ KC_LGUI, KC_LALT, KC_LCTL, KC_LSFT, XXXXXXX
#define ______________HOME_ROW_GACS_R______________ XXXXXXX, KC_LSFT, KC_LCTL, KC_LALT, KC_LGUI

#define LAYOUT_wrapper(...) LAYOUT(__VA_ARGS__)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [LAYER_BASE] = LAYOUT_wrapper(LAYOUT_LAYER_BASE),
  [LAYER_AUX] = LAYOUT_wrapper(LAYOUT_LAYER_AUX),
  [LAYER_SYMB] = LAYOUT_wrapper(LAYOUT_LAYER_SYMB),
  [LAYER_SYMB_SHIFT] = LAYOUT_wrapper(LAYOUT_LAYER_SYMB_SHIFT),
  [LAYER_FNC] = LAYOUT_wrapper(LAYOUT_LAYER_FNC),
  [LAYER_POINTER] = LAYOUT_wrapper(LAYOUT_LAYER_POINTER_CUSTOM),
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        if (osl_fnc_active) {
            cleanup_fnc_layer = true;
            osl_fnc_active = false;
        }
        if (osl_symb_shift_active) {
            cleanup_symb_layer = true;
            osl_symb_shift_active = false;
        }
    }
    return true;
}

void post_process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (cleanup_fnc_layer) {
        layer_off(LAYER_FNC);
        cleanup_fnc_layer = false;
    }
    if (cleanup_symb_layer) {
        layer_off(LAYER_SYMB_SHIFT);
        cleanup_symb_layer = false;
    }
}

#ifdef POINTING_DEVICE_ENABLE
#    ifdef CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_ENABLE
report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    if (abs(mouse_report.x) > CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_THRESHOLD || abs(mouse_report.y) > CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_THRESHOLD) {
        if (auto_pointer_layer_timer == 0) {
            layer_on(LAYER_POINTER);
#        ifdef RGB_MATRIX_ENABLE
            rgb_matrix_mode_noeeprom(RGB_MATRIX_NONE);
            rgb_matrix_sethsv_noeeprom(HSV_GREEN);
#        endif // RGB_MATRIX_ENABLE
        }
        auto_pointer_layer_timer = timer_read();
    }
    return mouse_report;
}

void matrix_scan_user(void) {
    if (auto_pointer_layer_timer != 0 && TIMER_DIFF_16(timer_read(), auto_pointer_layer_timer) >= CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_TIMEOUT_MS) {
        auto_pointer_layer_timer = 0;
        layer_off(LAYER_POINTER);
#        ifdef RGB_MATRIX_ENABLE
        rgb_matrix_mode_noeeprom(RGB_MATRIX_DEFAULT_MODE);
#        endif // RGB_MATRIX_ENABLE
    }
}
#    endif // CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_ENABLE

#    ifdef CHARYBDIS_AUTO_SNIPING_ON_LAYER
layer_state_t layer_state_set_user(layer_state_t state) {
    charybdis_set_pointer_sniping_enabled(layer_state_cmp(state, CHARYBDIS_AUTO_SNIPING_ON_LAYER));
    return state;
}
#    endif // CHARYBDIS_AUTO_SNIPING_ON_LAYER
#endif     // POINTING_DEVICE_ENABLE

#ifdef RGB_MATRIX_ENABLE
// Forward-declare this helper function since it is defined in
// rgb_matrix.c.
void rgb_matrix_update_pwm_buffers(void);
#endif
