# Casino state helpers

Three additional JP1 routines now compile to the original bytes:

- `func_8007DAFC`: 204-byte blackjack hand reset, initializing card/hand state
  and clearing per-player and dealer counters.
- `func_8007E570`: 164-byte bet-control lamp update.
- `func_80085874`: 172-byte variant that also excludes shown-bet value0xFF.

Total: 3 functions,540 bytes. All11 JP1 binary SHA-256 checks pass. No emulator
playtest was performed. Separate read/modify/write statements and pointer
locals retain the original lamp masks and address reuse. Adjacent unmatched
reset/animation routines remain in assembly.
