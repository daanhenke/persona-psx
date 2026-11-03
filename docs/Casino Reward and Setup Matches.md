# Casino reward, setup and small drawing matches

Eight JP1 functions now use matching C, totaling 2,764 code bytes:

| Function | Bytes | Behavior |
| --- | ---: | --- |
| `func_8008CE0C` | 348 | Dispatch reward codes 0x200–0x207, apply item ranges, or pass ordinary item codes through. |
| `func_8008514C` | 680 | Build layouts, a frame and three objects, then queue their palette identifiers. |
| `func_80092430` | 428 | Initialize four double parameters with the original constants and random expressions. |
| `func_80090828` | 552 | Reset round state, initialize both parameter blocks, clear three lamp bits and cursors, and queue opening animations. |
| `func_80091B00` | 120 | Update the double accumulator at offset 0x68 by `0.1 * valueB0`, then return its signed short conversion. |
| `func_8007FBD8` | 188 | Slide either object pair in opposite directions; ignore other selector values. |
| `func_800971D4` (S2D) | 180 | Select a map's front-layer draw handler and always advance its frame counter. |
| `func_8008C8D0` | 268 | Fill a 3x3 grid with nine distinct random indices using the original rejection loop. |

The random initialization preserves `-120.49`, `(rand()%1200-500)*0.01`,
`(rand()%100-50)*0.1+40.0` and `(rand()%1750+50)*0.1`. These are ordinary
C expressions compiled through the original software floating-point helpers.
The reward counter wraps to a byte before its 99-limit test, as in the image.
The shuffle keeps byte-sized row and column conversions; its zero-initialized
nine-byte local table occupies the original read-only-data region.

All 11 JP1 executable/overlay SHA-256 checks pass with these source units
included. No emulator or native Linux gameplay test was performed. The
original PS1 graphics, sound, random generator and memory interfaces remain.
