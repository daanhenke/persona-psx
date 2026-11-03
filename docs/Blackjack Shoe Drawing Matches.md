# Blackjack shoe drawing

The former 352-byte `D_80081C60` assembly range contains three separate routines: a 104-byte sequence restart, a 104-byte card draw, and a 144-byte draw that updates blackjack totals. Function boundaries now reflect the three return sequences without changing code size.

The draw helpers preserve separate shoe-position reads and byte wraparound, reset at 207 cards, and set the reshuffle flag. The complete reconstructed range has zero differing instruction words in the isolated compiler check. All eleven linked JP1 SHA-256 checks pass and were independently rehashed; the report scores all three functions at 100%.
