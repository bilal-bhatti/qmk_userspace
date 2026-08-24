/* Diagnostic keymap: hunt for the right spacebar's column pin.
 *
 * Background. The right-space cell [4,8] registers nothing. Measurement so
 * far has established:
 *
 *   - the switch closes (continuity pin to pin when pressed)
 *   - its row side is intact: pad -> joint -> diode -> D5
 *   - so a press ties its column pin RC down to D5 through that diode
 *   - QMK scans all 15 columns every cycle and none of them ever moves
 *
 * Therefore RC is not on any scanned column. This board's GPIO budget is
 * 15 columns + 5 rows + E2 (RGB data) + B7 (backlight), which leaves exactly
 * four pins unaccounted for: B0, F4, F5, F6. If this PCB revision routes that
 * footprint to one of them, QMK is simply blind to it and the trace is fine.
 *
 * F4/F5/F6 are JTAG TCK/TMS/TDO, but F7 is TDI and already works as a matrix
 * column, so JTAG is disabled and all four are usable GPIO.
 *
 * Rather than redefine MATRIX_COLS (which would desync the generated
 * LAYOUT_all macro), this does a small side scan of its own from
 * matrix_scan_user, which runs after matrix_task has finished and left every
 * row deselected. Drive D5 low, read the four spares, put D5 back the way
 * matrix.c's unselect_row leaves it. Nothing the real matrix scan does is
 * disturbed.
 *
 * Build and flash:
 *     qmk flash -kb ymdk/yd60mq/16led -km bbhatti_probe
 *
 * Then hold the right spacebar and watch `qmk console -n` for a PROBE line.
 */

#include QMK_KEYBOARD_H
#include "print.h"
#include "atomic_util.h"

/* Layer 0 only, matching bbhatti, so the board still types while probing. */
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

	[0] = LAYOUT_all(
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_GRV,  KC_BSPC,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS,
        KC_LCTL, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_NO,   KC_ENT,
        KC_LSFT, KC_NO,   KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_NO,   KC_RSFT, KC_NO,
        KC_CAPS, KC_LALT, KC_LGUI,          KC_SPC,  KC_NO,   KC_SPC,           KC_RGUI, KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT
    ),

};

/* Every GPIO that the press test does not already cover, plus C7 as a control.
 *
 * The ATmega32U4 has 26 GPIO. The press test rules out all 15 matrix columns
 * in one shot, since QMK scans them every cycle and none ever moved. That
 * leaves eleven:
 *
 *   B0 F4 F5 F6   spares this board's config never claims
 *   B7            backlight, free here because BACKLIGHT_ENABLE = no
 *   E2            RGB data, free here because RGBLIGHT_ENABLE = no
 *   D0 D1 D2 D3   the other matrix rows
 *
 * B7 and E2 are the only two that could still admit a firmware fix: either
 * could be reassigned as a 16th column, at the cost of that feature. The row
 * pins cannot, because RF already reaches D5, so a hit there would mean both
 * ends of the switch land on rows. They are included because they are free and
 * would at least explain the fault. Reading a row as input-high matches the
 * state matrix.c's unselect_row leaves it in, so this disturbs nothing.
 *
 * C7 is left space's column and left space is on row D5, so pressing left
 * space must pull C7 low in this same side scan. Without that control a silent
 * result is ambiguous: "none of these is the pin" and "the probe does not
 * work" look identical. Keep it last; PROBE_CONTROL_INDEX depends on it. */
static const pin_t  probe_pins[]  = {B0, F4, F5, F6, B7, E2, D0, D1, D2, D3, C7};
static const char *const probe_names[] = {"B0", "F4", "F5", "F6", "B7", "E2", "D0", "D1", "D2", "D3", "C7ctl"};
#define PROBE_CONTROL_INDEX (PROBE_COUNT - 1)
#define PROBE_COUNT (sizeof(probe_pins) / sizeof(probe_pins[0]))

/* Row the right-space cell's diode is proven to reach. */
#define PROBE_ROW_PIN D5

/* quantum/matrix.c keeps its gpio_atomic_* helpers file-static, so mirror the
 * two we need. The atomic block matters: DDRx/PORTx writes are
 * read-modify-write, and an ISR touching the same port mid-sequence would
 * corrupt it. */
static inline void probe_row_low(pin_t pin) {
    ATOMIC_BLOCK_FORCEON {
        gpio_set_pin_output(pin);
        gpio_write_pin_low(pin);
    }
}

static inline void probe_row_release(pin_t pin) {
    ATOMIC_BLOCK_FORCEON {
        gpio_set_pin_input_high(pin);
    }
}

void keyboard_post_init_user(void) {
    debug_enable = true;
    debug_matrix = true;
    uprintf("PROBE armed: B0 F4 F5 F6 read against row D5. Hold right space.\n");
}

void matrix_scan_user(void) {
    static uint16_t tick = 0;
    static uint16_t last = 0; /* one bit per probe pin, so must be 16-bit now */
    static uint32_t beat = 0;

    /* matrix_scan_user runs every scan; probing that often would swamp the
     * console endpoint for no gain. Once per ~100 scans is far faster than
     * any human keypress. */
    if (++tick < 100) {
        return;
    }
    tick = 0;

    for (uint8_t i = 0; i < PROBE_COUNT; i++) {
        gpio_set_pin_input_high(probe_pins[i]);
    }

    probe_row_low(PROBE_ROW_PIN);
    wait_us(50); /* let the pull-ups settle against the row */

    uint16_t now = 0;
    for (uint8_t i = 0; i < PROBE_COUNT; i++) {
        if (!gpio_read_pin(probe_pins[i])) {
            now |= (uint16_t)(1u << i);
        }
    }

    /* Exactly how matrix.c's unselect_row leaves a deselected row. */
    probe_row_release(PROBE_ROW_PIN);

    /* Anything printed from keyboard_post_init_user is lost, because USB is
     * not enumerated yet and the console endpoint will not accept it. So
     * report on a heartbeat as well as on change: that way the idle baseline
     * is visible no matter when `qmk console` attaches, and a steady stream of
     * these lines is itself proof this build is the one running. */
    if (now != last || timer_elapsed32(beat) > 3000) {
        beat = timer_read32();
        uprintf("PROBE");
        for (uint8_t i = 0; i < PROBE_COUNT; i++) {
            uprintf("%s%s=%u", (i == PROBE_CONTROL_INDEX) ? " | " : " ", probe_names[i], (unsigned)((now >> i) & 1));
        }
        uprintf("%s\n", (now & (uint16_t)((1u << PROBE_CONTROL_INDEX) - 1)) ? "   <<< PULLED LOW" : "");
        last = now;
    }
}
