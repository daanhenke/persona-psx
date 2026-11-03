# Cursor and collection helpers

Five JP1 casino functions now use matching C, totaling 1,048 bytes.

| Function | Behavior | Bytes |
| --- | --- | ---: |
| `func_8008B870` | Save the cursor position, update its state, and start the positive-scale animation | 204 |
| `func_8008B93C` | Apply the corresponding negative-scale animation | 204 |
| `func_800904FC` | Collect qualifying entries, refresh their displays, and restart the sequence when needed | 208 |
| `func_8007B12C` | Start three exit animations and advance the blackjack step | 220 |
| `func_80088954` | Clear both cursor inputs, reset two twelve-byte grids, and initialize reel state | 212 |

The animation pair retains the original signed short duration and division
behavior. Collection uses unsigned word values and preserves the original
load-before-clear order explicitly: save the money total, clear the entry,
then store the sum. The reset retains its four rows of three byte entries,
original stores, and timer guard.

All 11 JP1 executable/overlay SHA-256 checks pass with the C units enabled.
No emulator gameplay test was performed.
