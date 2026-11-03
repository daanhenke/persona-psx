# Matching casino animation steps

`CasinoAnimStep0`, `CasinoAnimStep1` and `CasinoAnimStep2` each start two
16-frame animations with an X-rotation delta of 0x800 and all other deltas
zero. Direct calls with the existing `CasinoStartAnim` signature reproduce
the original 128-byte functions. `func_80082034` is the adjacent small lookup:
7 maps to 9, 6 maps to 10, and all other integer inputs map to 0xFF.
These four functions total 424 bytes; all 11 JP1 binary hashes pass.
