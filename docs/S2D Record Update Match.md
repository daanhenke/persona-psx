# S2D record update

`func_80099A00` updates three 40-byte records at `D_800B5E28`. Mode one
adds an increment and clamps the value to 4096; mode two subtracts it and
clamps to zero. Both copy the resulting value to the next field and clear
the mode on reaching the endpoint. The zero endpoint also clears the enable
halfword. Other modes leave the record untouched.

Halfword-based field pointers retain the original access ordering and repeated
loads; the integer fields are at aligned byte offsets 20, 24 and 36 from the
record base. Advancing the record pointer before the loop index reproduces
the original instruction scheduling. A conventional struct reconstruction
eliminated accesses and did not match.

Acceptance requires 100% objdiff for all 216 bytes and all eleven JP1 linked
binary SHA-256 checks using the pinned original compiler.
