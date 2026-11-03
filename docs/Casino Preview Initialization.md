# Casino preview initialization

`func_80092910` initializes the second preview object. Its rotation is zero,
position is (-24, -100, 634), and scale is (1, 1, 1) before the conversion
helpers populate the current and previous transforms. It clears status bytes
0, 1, 2 and 7, sets byte 8, randomizes the current rotation, then submits the
model with that current transform.

Each vector is passed by value at its call site. The second call in each pair
reloads the vector, so a helper changing shared state is observable. The model
submission likewise observes the randomizer's changes. The first preview uses
the same sequence with a positive x position and separate state.

The recovered C is 820 bytes with the original compiler. The isolated linked
comparison is exact; acceptance also requires all eleven complete JP1 binary
hashes. The native fixture checks 2,048 initial-state/callback combinations,
including untouched flags and transform padding, call ordering, destinations,
vector reloads and final by-value submission. It passes optimized Linux and
AddressSanitizer/UndefinedBehaviorSanitizer builds with recovery disabled.

The test supplies a 32-bit `VECTOR` to preserve the PS1 ABI on a 64-bit host and
controlled helper callbacks. It does not exercise PS1 rendering or establish
that the game boots natively.

```sh
cc -std=c99 -O2 -fno-builtin -funsigned-char -fno-strict-aliasing \
  -Iinclude -Iinclude/psyq tests/native/casino_previewinit2.c -o /tmp/previewinit2
/tmp/previewinit2
```
