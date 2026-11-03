# Casino rotation integration and snapping

`func_80091B78` advances the double angle at offset 0x50 by one tenth of the
step at 0x98, truncates to an integer, scales by eleven and wraps with signed
remainder modulo 4096. It then calls `func_80091D0C` before checking the snap
flag. Separating scaling from the remainder assignment preserves the original
register allocation and matches all 404 bytes without assembly or padding.

When snapping is enabled, the routine chooses the next signed quarter-turn
boundary, clears the step and velocity fields and changes the flag to two.
The original interval endpoints are preserved, including zero snapping to
+1024 and a negative exact quarter boundary advancing to the next negative
boundary. Shared declarations in `persona/casino/rotation.h` keep the caller,
step routine and response routine consistent.

Validation: 100% objdiff for the new routine and both affected existing units;
all eleven original JP1 binary SHA-256 checks pass and were independently
recomputed. Twenty-nine native fixtures check signed ranges, exact boundaries,
wrapping, integration, field offsets and callback changes to the snap flag.
They also pass AddressSanitizer and UndefinedBehaviorSanitizer with LeakSanitizer
disabled for the traced sandbox. No emulator gameplay or native boot was tested.

Run from the repository root:

```sh
cc -std=c99 -Wall -Wextra -Werror -Iinclude tests/native/casino_rotationstep.c src/casino/game/rotationstep.c -o /tmp/casino-rotationstep
/tmp/casino-rotationstep
```
