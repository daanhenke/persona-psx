# Reel palette selection

`func_80084D50` now reproduces its original 252 bytes in C, including both 32-byte jump tables. Call sites pass a signed-byte reel index and an active flag. The two palette-coordinate temporaries are locals, not extra parameters inferred from registers.

For indices 0, 2, 5 and 7, the active palette coordinates come from `D_800999D8`; indices 1, 3, 4 and 6 use `D_800999F0`. Inactive coordinates are (592, 498) and (592, 500), respectively. The palette is queued for index + 376. The original valid-input contract is retained: index 0..7 and active 0 or 1; no new default case changes the binary.

The first isolated candidate matched. Production then passed the pinned original-compiler build, 100% objdiff for this routine, all 11 JP1 SHA-256 checks, and an independent rehash of those outputs. No emulator or native runtime test was performed.
