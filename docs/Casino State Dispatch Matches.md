# Additional casino state dispatch matches

Two JP1 state dispatchers now use matching C, totaling 628 code bytes:

- `func_800886F0` (296 bytes) selects one of fourteen handlers from
  `g_casino_step`, then always calls `func_8008A628`, including for
  unhandled states. Its original jump table contains 104 entries.
- `func_8008D250` (332 bytes) selects one of fifteen handlers, then reloads
  the current state and calls `func_8008EE00` unless it is 0x11. A handler
  can change the state, so the second test must not reuse the earlier
  switch value. Its original jump table contains 102 entries.

The original case ordering and jump-table regions are preserved. Both
functions matched their first typed switch drafts. All 11 JP1 binary
SHA-256 checks pass. No emulator or native Linux gameplay test was
performed; the existing handlers remain responsible for the actual work.
