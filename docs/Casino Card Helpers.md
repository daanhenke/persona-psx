# Casino card and callback helpers

The JP1 reconstruction adds matching C for these routines:

| Function | Bytes | Behavior |
| --- | ---: | --- |
| func_800823A0 | 96 | Blackjack card value from rank |
| func_80082400 | 84 | Select outcome from flags and totals |
| func_80082750 | 68 | Count occurrences and compare with a minimum |
| func_800827EC | 240 | Compare two card ranks, including the special card |
| func_80083B14 | 8 | Empty callback |
| func_80089154 | 8 | Empty callback |
| func_8008D6E4 | 8 | Empty callback |
| func_8008FEC4 | 88 | Shift two coordinate pairs by eleven units |
| func_8008C81C | 180 | Map encoded text ranges to glyph rows |

Total: 9 functions and 780 bytes. All 11 JP1 executable/overlay SHA-256
checks pass under GCC 2.6.0. No emulator gameplay test was performed.

The rank temporary in the blackjack-value switch remains an unsigned byte;
its integer promotion reproduces the original range comparisons. The
coordinate update uses separate aliases for existing Y fields and a pointer
local for the first X field, preserving the original address reuse and store
schedule without adding storage. The rank comparison retains its original
two-element local array and signed remainder operations. Its final three
conditions cover all possible values of the difference.
