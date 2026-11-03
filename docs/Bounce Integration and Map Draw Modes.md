# Bounce integration and map draw modes

Two JP1 functions now use exact C, totaling 592 bytes:

- Casino `func_800917F0` (284B) applies the original double-precision acceleration when enabled, integrates position, converts `762.0-position` to an integer, then starts the first-impact sequence and invokes the existing bounce helper when the result reaches 762. The signed mode byte increments with byte wrapping.
- S2D `func_80096904` (308B) selects draw modes for map effects 0/1, duplicates their SDK packets for both buffers and ignores other effect values.

The acceleration literal is `-0.9800000000000002`, recovered from the original
64-bit constant `0xBFEF5C28F5C28F5E`. Shortening it to `-0.98` changes the
constant bits. This preserves observed arithmetic rather than claiming the
original spelling of the expression. Integration uses the original operand
order: `0.1 * velocity + position` and then `762.0 - position`.

The existing bounce helper and its new caller now share one `CasinoBounce`
type in `persona/casino/bounce.h`. The known inhibit byte at 0xE2 was added;
the existing callee's instructions remain unchanged. No fake padding or
assembly was added to force a match. The draw-mode helper uses SDK DR_MODE
assignments, preserving the original three-word copies and common tail.

Both objects compare at 100%, and all 11 executable/overlay SHA-256 checks
pass after final report regeneration and rebuild. No emulator gameplay test
or native Linux backend is included. A separate 188-byte rank-palette helper
remains assembly and is saved locally for advanced-model analysis.
