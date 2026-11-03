# Casino grid and reel matches

Five JP1 casino routines are now implemented in C, replacing 2,360 bytes of assembly:

| Function | Bytes | Behavior |
| --- | ---: | --- |
| `func_8008AD00` | 480 | Step grid entries toward their target values and update completion/timing state. |
| `func_80083934` | 480 | Finish the reel animation and advance the casino state. |
| `func_80083BFC` | 428 | Initialize reel award animation, palettes, and audio volume. |
| `func_80083DA8` | 484 | Pay reel winnings, preserve the money cap, and advance after payout. |
| `func_8008174C` | 488 | Flash blackjack natural labels for the normal and split hands. |

Validation on 22 October 2026 used the pinned original GCC 2.6.0/MASPSX build environment. Each function scores 100% in objdiff; all 11 JP1 linked binary SHA-256 checks pass, including an independent rehash. Overall exact matching is 1,197,564 / 1,413,944 code bytes (84.6967%), with 2,880 / 3,109 functions matched.

This is matching PS1 code, not a native Linux runtime. No emulator gameplay test was performed. Approximate scratch candidates are excluded.
