# Reel win presentation

`func_800840C8` at CASINO offset `0x1FD28` is now matching C in `src/casino/game/reelwin.c` (892 bytes).

State 2 selects the result jingle, adds the bet-scaled table payout, enables the winning sprites and palettes, and resets the presentation timer. State 5 waits for the sequence to finish and advances the timer. State 7 clears the highlights, refreshes the displayed winnings and advances to state 8.

The result fields occupy adjacent signed bytes at `D_800A85F8 + 39` and `+40` (the latter also named `D_800A8620`). Expressing them through the shared array preserves the original compiler's saved pointer. The palette index grouping `(kind * 8 + 161) + rank` also preserves its load and register order. The timer uses the established assembler symbol alias for `D_800AF834 + 218`; no instruction assembly is embedded in the function.

Validation uses the pinned GCC 2.6.0 toolchain: the isolated function and objdiff report match, and all eleven complete JP1 binaries retain their original SHA-256 hashes. This verifies the PS1 rebuild; no new runtime or native Linux test is claimed.
