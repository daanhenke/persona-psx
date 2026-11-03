# Fusion arcana stepping

`FuseArcanaStep` in `src/adv/ui/facilitymisc.c` moves within an arcana's Persona range. Direction 0 keeps the current Persona, 1/2 move down one/two positions, and 3/4 move up. Boundary checks and `FuseSpecialHas` preserve the original selection rules.

The one-step paths reuse the boundary local for the signed-short candidate and return that candidate when it is not special. The two-step downward path initializes its result after the boundary check. The upward path encloses its work in `cur != n`, leaving the boundary return after that block. These forms reproduce the original shared returns and register allocation without inline assembly.

The C function owns its five-entry switch table at ADV offset `0xED8`, including the alignment before the following table. Unsupported direction values retain the original undefined return behavior; this change does not invent a fallback result.

Validation requires the original compiler's 508-byte function match and all eleven complete JP1 binary SHA-256 checks. Binary identity does not establish native Linux portability or gameplay.
