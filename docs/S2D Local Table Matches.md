# S2D local table matches

Replace two assembly helpers with 188 bytes of matching C: `func_80094594` translates the current facing through a four-entry short table; `func_8008BB14` selects a movement step and direction bit pattern from two three-entry tables.

Local array initializers preserve the original stack copies. Their 24 bytes of constant tables and alignment are assigned to the original read-only-data ranges; the following four-byte padding remains in the original data segment.

Validation: both functions score 100% in objdiff. All eleven JP1 binary SHA-256 checks pass and were independently rehashed after report generation and the normal build. Local log: `easy-localtables-validation.log`. No emulator or native-port gameplay testing was performed.
