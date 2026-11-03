# Matching palette table helpers

Two previously deferred casino helpers now recover their original instructions:

- `func_8008A974` (104 bytes) clears a marked grid entry and queues its two palettes. A flat signed-byte table pointer retains the original order of the base-address load and row-times-three calculation. The prior two-dimensional array expression differed in six words.
- `func_80090EEC` (108 bytes) redraws an amount only when it differs from the saved value, then reloads and saves that amount after the callback. Explicit byte offsets from the two table bases retain the original address scheduling. The prior typed array-index expression differed in three words.

Both helpers passed all eleven linked JP1 binary SHA-256 checks with the original GCC 2.6.0 compiler in the pinned offline environment. Relocation-normalized object comparisons are used only to identify candidates; linked checks remain the acceptance gate.
