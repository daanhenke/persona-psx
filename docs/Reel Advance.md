# Reel advance (JP1)

`func_800868A0` at CASINO address `0x800868A0` advances one reel after its
travel counter reaches a full symbol height. The matching C implementation
in `src/casino/game/reeladvance.c` occupies 548 bytes.

The first five reel entries use 32-pixel steps; the other entries use
16-pixel steps. At position 27 the two texture banks wrap to entries zero
and 27, then the position becomes zero after the final increment. Other
positions select the next entry for the first bank and the current entry
for the second bank. Four one-frame tween/scroll calls move the two banks
in opposite directions, then reset the travel counter.

The initial signed position is saved for the first texture lookup. The
second lookup rereads the byte, preserving the original access order after
the intervening texture-index store. The signed narrow index and promoted
index retain the original compiler's separate address calculations.

Validation: complete instruction match and all eleven JP1 binary SHA-256
checks passed. No inline instruction bodies or compiler changes are used.
Native Linux gameplay remains a separate porting milestone.
