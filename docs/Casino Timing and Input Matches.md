# Casino timing and input matches

Seven routines replace assembly fallbacks with matching C, totaling 3,004 code
bytes:

| Routine | Bytes | Behavior |
| --- | ---: | --- |
| func_80093950 | 384 | Selects the reward sequence and sound bank. |
| func_80084E4C | 376 | Updates enabled reel sprites using a 12-by-5 lookup table. |
| func_80086D40 | 428 | Selects timing steps and updates the sequence counter. |
| func_80086EEC | 456 | Selects alternate timing steps and handles its countdown. |
| func_8007C358 | 444 | Dispatches blackjack hand updates and refreshes natural-hand palettes. |
| func_8007D554 | 448 | Resolves dealer state and builds the dealer label. |
| func_800856A0 | 468 | Handles reel betting, confirmation, refund, and quit input. |

The sprite lookup remains a named shared constant, `D_80065284`, because another
assembly routine also references it. The local aggregate copy preserves the
original compiler's copy sequence. The timing countdown preserves its original
post-decrement comparison, including the negative-value clamp. Dealer natural
values 1 and 2 use a two-value condition, which the original compiler combines
into the matching range test. Existing casino flag and layout types are reused.

All seven routines score 100% in objdiff. The pinned original-compiler build
passes all eleven JP1 binary SHA-256 checks; independent rehashing confirms all
eleven outputs. These are PS1 binary matches, not a native Linux port or an
emulator gameplay test.
