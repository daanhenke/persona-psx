# Casino grid input

`func_80088A28` initializes the grid prompt and lamp timers on frame zero, then
accepts cancel or confirm input after frame 16. An eligible cancel takes priority
when both types of input arrive together. An existing pending confirmation is
processed independently of the input-delay gate.

A remaining retry stops and restarts sequence 4, marks the bet complete, and
consumes one retry. Otherwise the lamp sequence takes over from the grid prompt.
The two final update flags are tested separately; the lamp callback can enable
the prompt callback in the same frame. Sound handles and retry counts are read
after preceding callbacks, preserving the original ordering.

The two timer fields use named offsets in the existing overlay work area, as in
their other consumers. Treating them as independent scalar fields reproduces the
original compiler's store ordering. No instruction padding or assembly body is
used. The complete 612-byte function must match with actual symbol addresses,
and all eleven JP1 binary hashes remain the acceptance gate.

The native fixture covers 6,912 combinations of timer boundaries, all low input
bits, retry limits, busy/done/quit states, and pending confirmations. Four more
cases exercise callbacks changing the timer, sequence handle, retry count, and
prompt flag, plus flags whose nonzero value is not one. It checks untouched work
area fields as well. These are isolated logic tests, not an emulator gameplay
or native Linux game-boot test.

```sh
cc -std=c99 -O2 -fno-builtin -funsigned-char -fno-strict-aliasing \
  -Iinclude -Iinclude/psyq tests/native/casino_gridinput.c \
  src/casino/game/gridinput.c -o /tmp/persona-gridinput-test
/tmp/persona-gridinput-test
```

## Blackjack round reset

`func_8007DA00` resets the grid cursor to (0,0), sets its size to12 by12 with
wrapping enabled, clears the bet and insurance bookkeeping, and requests a shoe
reshuffle. Displayed money and jackpot values copy the live values. Only the
lowest six lamp bits and the documented work-area flag are cleared; unrelated
bits and neighboring bytes remain intact.

The flag at `D_800AAEA8+4` is also represented as an independent scalar. Together
with the existing lamp bitfields, this preserves the original252-byte scheduling.
Native checks exercise all256 low-byte lamp patterns, high-bit preservation,
unsigned money and negative jackpot values, and untouched cursor/work bytes.
