# Casino small switch matches

Replace two assembly helpers with 232 bytes of matching C. `func_8007F5F8` maps selected state bytes to compact values and preserves unhandled states. `func_80082454` returns the original card-specific values, with an unsigned modulo-13 default.

The original case ordering reproduces both code and jump tables. The nine-entry and 53-entry tables retain their original read-only-data addresses; the four-byte padding after the first remains in assembly data. The existing `cardvalue.c` helper is unchanged.

Validation: both functions score 100% in objdiff. All eleven JP1 binary SHA-256 checks pass and were independently rehashed after report generation and the normal build. Local log: `easy-casino-switches-final-validation.log`. No emulator or native-port gameplay testing was performed.
