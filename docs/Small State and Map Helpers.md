# Small state and map helpers

Six more JP1 routines compile to the original bytes:

| Function | Overlay | Bytes | Behavior |
| --- | --- | ---: | --- |
| func_8009C308 | S2D | 52 | Clear scale values and enter a parameterized state |
| func_800905CC | Casino | 144 | Update idle-state flags and control lamps |
| func_80094F9C | S2D | 92 | Find a record by its first two bytes among96records |
| func_800945E0 | S2D | 128 | Advance two flags using a five-duration table |
| D_80094A34 | S2D | 112 | Update one nibble of a packed byte |
| func_80094AA4 | S2D | 100 | Read one nibble of a packed byte |

Total:628bytes. All11 JP1 binary SHA-256 checks pass. No emulator playtest.

The packed-byte helpers use the original `x / 2 == 0` condition for nibble
selection; changing it to an even/odd test would change the original game.
The row-offset pointer expression precedes the half-index condition so GCC
reuses the division result in the original registers. The getter spells the
zero-half case first to retain the original branch order.

Assembler aliases should omit spaces around an addend when adjacent to a
load-delay-sensitive store: `D_800AAE9C+0x1`. The existing maspsx wrapper keeps
an extra nop for the spaced spelling in this routine, while the compact
alias emits the original144-byte body. Both spellings name the same storage.
