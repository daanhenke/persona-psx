# Blackjack total update after a draw

`func_800821B8` updates the two possible hand totals after drawing another card (340 bytes). Ace draws retain distinct low/high totals where possible; other draws use the saved alternative total when one result would exceed 21.

The matching form uses an integer card-value temporary and explicit byte casts in the comparisons. Reusing the byte argument introduced an extra register copy, and reversing the branch layout introduced extra jumps. Both changed the original instruction sequence.

The original-compiler isolated comparison has zero differing instruction words. The progress report gives a 100% match, and all eleven linked JP1 SHA-256 checks pass, independently rehashed after the build.
