# QMK's default is sym_defer_g: one global debounce timer, restarted by ANY key's
# transition, and `cooked` is only ever a snapshot of `raw` taken after DEBOUNCE ms
# of total matrix quiet. Intermediate transitions are never replayed, so a press
# that is released before a quiet window lands is dropped outright - which shows up
# as "the first press after another key vanishes, rapid repeats survive".
#
# asym_eager_defer_pk keeps a counter per key: key-down registers eagerly on first
# detection, key-up is deferred. No key's activity can swallow another's press.
DEBOUNCE_TYPE = asym_eager_defer_pk
