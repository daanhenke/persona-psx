# Casino bet input controls

`func_80090248` (232 bytes) and `func_80090448` (180 bytes) now compile as
matching C in the JP1 casino overlay. Both retain the original one-shot input
flags, signed selected-bet index, sound call order and post-call balance loads.

The add routine permits a bet below 100 before adding ten (so an input value
of 99 becomes 109); it does not clamp the result. With fewer than ten available
coins it plays the denial sequence. The refund routine captures the amount
before the sound calls, then subtracts that amount from the current bet and
adds it to the current balance. Clearing the bet unconditionally would lose
updates performed during those calls.

Validation: both units report 100% objdiff; all eleven JP1 output SHA-256 hashes
match the reference after a full generated build. Sixteen native fixtures
cover pending flags, thresholds, call order, neighboring entries, unsigned
wraparound and sound-call mutation. All pass with AddressSanitizer and
UndefinedBehaviorSanitizer (LeakSanitizer disabled for the traced sandbox).
These checks do not establish gameplay or native Linux boot support.

Run the fixtures from the repository root:

```sh
cc -std=c99 -Wall -Wextra -Werror tests/native/casino_betadd.c -o /tmp/casino-betadd
/tmp/casino-betadd
cc -std=c99 -Wall -Wextra -Werror tests/native/casino_refund.c -o /tmp/casino-refund
/tmp/casino-refund
```
