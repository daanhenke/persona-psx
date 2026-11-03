# Reel choice exit match

Casino `func_800885C4` now uses 300 bytes of exact matching C.

The 0x20 input bit takes priority: it clears winnings, updates the reel result,
tweens three layouts and one object out, then sets state 0x26 and timer -1.
Otherwise, input bit 0x80 advances to state 0x77 and timer 32 only while the
unsigned winnings value is below 10,000,000. Other input leaves state/timer
unchanged. The source preserves the original SDK interfaces, signed tween
directions and shared timer-store behavior.

The candidate matched its first typed draft. Objdiff scores the object 100%,
and all 11 JP1 executable/overlay SHA-256 checks pass after final report
regeneration and rebuild. No emulator gameplay test or native Linux backend
is included. Grid rotations and a separate horizontal double integrator
remain assembly and are preserved locally for advanced-model analysis.
