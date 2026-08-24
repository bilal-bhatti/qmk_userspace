# Diagnostic build. Console is the whole point of this keymap, so unlike
# bbhatti it is enabled unconditionally rather than passed with -e.
CONSOLE_ENABLE = yes

# Match bbhatti so the probe runs against the same debounce behaviour.
DEBOUNCE_TYPE = asym_eager_defer_pk

# Dark while working on the board. This only stops E2 being driven and B7
# being lit; it disconnects nothing, and the LED power rails stay live
# whenever USB is plugged in. Also buys back flash, which matters at 93%.
RGBLIGHT_ENABLE = no
BACKLIGHT_ENABLE = no
