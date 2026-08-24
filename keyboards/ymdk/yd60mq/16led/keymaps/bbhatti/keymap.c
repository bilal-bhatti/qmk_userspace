#include QMK_KEYBOARD_H

/* Mirror of keyboards/thevankeyboards/bananasplit/keymaps/bbhatti/keymap.json
 *
 * Same physical build: ANSI, split backspace, split right shift (1.75u + 1u fn),
 * split spacebar, 5x 1u bottom right (RGUI + arrows).
 *
 * LAYOUT_all is a superset, so three switches that this build does not populate
 * are KC_NO: ISO hash (2,12), ISO backslash (3,1), and the spare right-shift
 * position (3,12).
 *
 * Extras this board has and the BananaSplit does not (QK_BOOT, RGB, backlight)
 * live only in positions the BananaSplit leaves as KC_NO.
 */

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

	[0] = LAYOUT_all(
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_GRV,  KC_BSPC,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS,
        KC_LCTL, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_NO,   KC_ENT,
        KC_LSFT, KC_NO,   KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_NO,   KC_RSFT, MO(2),
        KC_CAPS, KC_LALT, KC_LGUI,          KC_SPC,  MO(1),   KC_SPC,           KC_RGUI, KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT
    ),

	[1] = LAYOUT_all(
        QK_BOOT, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_TILD, KC_DEL,
        KC_NO,   KC_F13,  KC_F14,  KC_F15,  KC_F16,  KC_F17,  KC_F18,  KC_F19,  KC_F20,  KC_F21,  KC_F22,  KC_F23,  KC_F24,  KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   MS_BTN2, MS_BTN1, MS_LEFT, MS_DOWN, MS_UP,   MS_RGHT, KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,            KC_NO,   _______, KC_NO,            KC_NO,   KC_HOME, KC_PGDN, KC_PGUP, KC_END
    ),

	[2] = LAYOUT_all(
        KC_MPLY, KC_MPRV, KC_MNXT, KC_VOLD, KC_VOLU, KC_MUTE, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   UG_TOGG, UG_NEXT, UG_HUEU, UG_HUED, UG_SATU, UG_SATD, UG_VALU, UG_VALD, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   BL_TOGG, BL_DOWN, BL_UP,   BL_STEP, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   _______,
        KC_NO,   KC_NO,   KC_NO,            KC_NO,   KC_NO,   KC_NO,            KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO
    ),

};

/* Temporary diagnostic for the sporadic key drops (d, e, p, - so far).
 *
 * Only compiled into a CONSOLE_ENABLE build, so the normal firmware is
 * byte-identical without it:
 *     qmk compile -kb ymdk/yd60mq/16led -km bbhatti -e CONSOLE_ENABLE=yes
 *
 * Logs every key event. `kc` is the keycode AFTER layer resolution, so a
 * momentary layer leak shows up as the wrong keycode on the right matrix cell.
 * Reading a drop against this log:
 *
 *   no line at all         -> never got through the matrix (electrical / USB in)
 *   line, lyr != 0         -> layer leak; MO(1) or MO(2) chattering
 *   line, lyr 0, kc right  -> firmware sent it, host lost it (USB / hub)
 *
 * Delete this block once the cause is found.
 */
#ifdef CONSOLE_ENABLE
#    include "print.h"

/* Raw matrix dump. Prints one line per scan in which any cell changed state,
 * BEFORE the keymap is consulted, so it fires even for cells mapped to KC_NO
 * and even for cells no LAYOUT macro exposes. This is the layer to trust when
 * asking "did the switch close at all?".
 *
 * Bottom-row matrix positions on this board (rows D0 D1 D2 D3 D5,
 * cols F0 F1 E6 C7 C6 B6 D4 B1 F7 B5 B4 D7 D6 B3 B2):
 *
 *   left space   [4,3]  row D5 x col C7
 *   MO(1)        [4,7]  row D5 x col B1
 *   right space  [4,8]  row D5 x col F7
 */
void keyboard_post_init_user(void) {
    debug_enable = true;
    debug_matrix = true;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    static uint16_t ev = 0;
    ev++;
    uprintf("EV#%u r%uc%u kc=0x%04X lyr=%u %s\n", ev, record->event.key.row, record->event.key.col, keycode, get_highest_layer(layer_state), record->event.pressed ? "DOWN" : "UP");
    return true;
}
#endif
