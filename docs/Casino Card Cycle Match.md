# Casino card cycle

`D_80080178` increments or decrements a card byte, wraps its value within
the ordinary 52-card range, updates its display at the selected slot, then
sets the slot's transform and draws its object. The decrement path preserves
the original signed-byte test after the byte subtraction.

The existing symbol name is retained. The implementation uses the project's
`CasinoXform` and `CasinoObj` types and existing display helpers. Acceptance
requires all 248 bytes to match in objdiff and all eleven JP1 linked binary
SHA-256 checks to pass in the pinned original-compiler environment.
