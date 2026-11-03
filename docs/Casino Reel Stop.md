# Casino reel stopping

`func_80083398` manages the four stop inputs, timer-driven reel updates,
palette animation and the transition from step 0x71 to 0x72. Directional
input and timer 104 move active reel modes from 4 to 3. It increments the
corresponding byte counters only for eligible modes, and starts the exit
animation on the two confirm buttons when the exit latch is clear.

The completion check reads all four modes. The pending flag is captured before
the confirmation callbacks, so later changes do not retroactively change that
check. The exit animation holds the address of the shared layout pointer and
loads its current value after audio callbacks. It does not capture an outdated
layout value. The original signed-byte checks, modulo-17 audio stop, unsigned
counter wraparound and overlapping five-byte mode view remain intact.

The recovered function is 1,436 bytes and matches in an isolated comparison
using real symbol addresses. A bounded decomp-permuter search found the final
layout-reference form after the parser was supplied with complete type
information. Its candidate was inspected and cleaned before validation.
Acceptance requires all eleven complete JP1 binary hashes.

The native fixture checks 720,896 timer, state, input and callback combinations.
It checks complete state and call traces, including callbacks that change the
layout pointer, timer, input, reel modes and latches. Optimized Linux and
AddressSanitizer/UndefinedBehaviorSanitizer runs pass with recovery disabled.
Its aliased storage represents the shared mode and counter bytes; animation
and audio callbacks are controlled replacements, not native gameplay.

```sh
cc -std=c99 -O2 -fno-builtin -funsigned-char -fno-strict-aliasing \
  -Iinclude -Iinclude/psyq tests/native/casino_reelstop.c \
  src/casino/game/reelstop.c -o /tmp/reelstop
/tmp/reelstop
```
