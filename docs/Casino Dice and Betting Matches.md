# Casino dice and betting matches

Five more JP1 casino routines replace 3,080 bytes of assembly with exact matching C:

| Function | Bytes | Behavior |
| --- | ---: | --- |
| func_800911E4 | 536 | Convert quarter-turn dice orientation into the visible face. |
| func_8008DD44 | 548 | Present a reward and accept acknowledgment after the delay. |
| func_8007E30C | 612 | Handle card betting, refund, confirmation, and quit input in original priority order. |
| func_8008D6EC | 660 | Record dice results, maintain the ten-entry history, and select the next state. |
| func_8008D980 | 724 | Calculate capped winnings and animate the win presentation. |

All five score 100% code matching in objdiff. All 11 original JP1 binary SHA-256 checks pass, with an independent rehash on 22 October 2026. Exact code progress is 1,200,644 / 1,413,944 bytes (84.914536%), with 2,885 / 3,109 functions matched. The original compiler and pinned offline environment were used.

The dice win state's 44-byte zero template retains the global name D_80065CA8 because another payout state also references it. The dice-face routine preserves all four jump tables, including the original treatment of negative rotations. Its tables add 124 bytes to objdiff's data accounting; the code denominator remains unchanged.

No emulator gameplay test or native Linux runtime milestone is claimed.
