# Blackjack menu and hand totals

`func_8007F00C` positions menu pieces for two-, three-, and four-entry layouts (340 bytes). Representing the pointer as a record member preserves the original compiler's memory access ordering; a standalone pointer declaration incorrectly allowed loads to move ahead of stores.

`func_8008205C` computes the two possible totals of a two-card blackjack hand (348 bytes). It preserves repeated card-value calls and byte-truncated rank tests, including the two-ace totals of 2 and 12.

Both candidates pass isolated original-compiler instruction comparisons. Both score 100% in the progress report; all eleven linked JP1 SHA-256 checks pass and were independently rehashed. The existing card-value helper now consistently returns int, with an explicit byte cast preserving its original instructions.
