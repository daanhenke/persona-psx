# Casino reel update controller

`func_8008634C` updates the displayed money and win totals, starts the active
bet's palette cycle when needed, and synchronizes three pairs of lamp bits
with object animations. It invokes manual reel handling in steps 0x71 and
0x73. In steps 0x70–0x73 and 0x3C, it visits all 21 reels, selects start/stop
timing for idle layouts, starts their transitions, advances 16-bit tick
counters, and scrolls busy layouts using signed delta/frame division.

The explicit empty state-zero switch arm preserves the original dispatch
shape. The loop begins at zero within its state guard. These ordinary C forms
recover the original compiler allocation without register assignments or
padding. `reels.h` describes the shared two-bank layout and per-reel tables,
also used by the quarter-turn routine.

Reads following callbacks remain observable: shown totals use the latest
values, lamp history copies the latest request bit after animation starts,
and layout busy state, timing and ticks are read again after timing and tween
helpers. A busy reel requires a nonzero signed frame duration, as in the
original division instruction.

The recovered function is 1,116 bytes and matches at the original linked
addresses. Acceptance also requires all eleven complete JP1 hashes. The
221,184 native fixtures cover all 64 low lamp-bit patterns, twelve game steps,
twelve bets including invalid/high values, three busy-state patterns, all
switch arms and high-bit state values, counter wraparound, signed timing,
unchanged totals, and callbacks changing totals, lamp requests, future reels,
layout pointers and timing. They pass optimized and ASan/UBSan builds with
recovery disabled. Palette aliases are adjusted to the native pointer width
inside the fixture only. Audio, actual rendering and native game startup are
not exercised by these controlled callback tests.

```sh
cc -std=c99 -O2 -fno-builtin -funsigned-char -fno-strict-aliasing \
  -Iinclude -Iinclude/psyq tests/native/casino_reelupdate.c -o /tmp/reelupdate
/tmp/reelupdate
```
