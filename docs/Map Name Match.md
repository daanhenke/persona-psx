# Shared automap name match

`MapDrawName` draws the ten-cell automap title banner in ADV, DNG, and S2D. Its shared C reads each source byte pair by index while advancing the two destination pointers. The original GCC 2.6.0 reduces the indexed source access to the original pointer loop and assigns the saved registers correctly. The previous explicit source-pointer increment swapped the source and destination saved registers.

The cell byte swap, high-bit flag, terminator, background dimensions, and visibility flag are unchanged. Each overlay compiles its own instance so its global addresses remain correct. No inline assembly, padding, compiler changes, or binary edits are needed.

All three 252-byte instances are 100% exact in the regenerated objdiff report. All eleven JP1 executable/overlay SHA-256 checks pass after the report and final matching rebuild; the hashes were also independently recomputed. No native runtime or emulator gameplay testing is claimed.
