# Grid wins, card selection, and dice presentation

Five JP1 casino assembly fallbacks are replaced with matching C (4,920 bytes):

| Function | Bytes | Behavior |
| --- | ---: | --- |
| `func_800895EC` | 1,036 | Presents a grid win, animates lamps and palettes, handles confirmation, and advances the level. |
| `func_80080A78` | 916 | Animates card selection, repeats directional input, cycles the selected card, and exits the selection state. |
| `func_8008EE00` | 1,208 | Refreshes dice-game values, state-dependent animations, and selection palettes. |
| `func_8008F7E4` | 1,272 | Builds the dice-scene layouts, textures and draws its objects, and initializes cursor transforms. |
| `D_8008FCDC` | 488 | Applies manual pad input to dice rotation and position. |

The card cycle uses the existing matched helper's operations inline. Adding 255 to an unsigned byte retains the original decrement code generation. Layout and palette aliases identify existing records without allocating storage; their separation preserves the original compiler's address handling. `g_grid_win_palette_on` names the existing byte at `0x80099CA0`.

All five functions score 100% in objdiff and pass all eleven JP1 linked SHA-256 checks in the pinned offline original-compiler environment. Isolated linked-address comparisons also report no differences. No emulator gameplay testing was performed.
