# Field input selector match

`func_8008E158` (104 bytes, S2D) returns held, newly pressed, or repeated input for selectors 0, 1, and 2. The shared return path also preserves the original binary's out-of-range behavior: negative selectors return 1 and selectors above 2 return 2. Every C return path has an initialized result.

Explicit branch labels and a shared return reproduce the original dispatch and delay slots without inline assembly, padding, or compiler changes. The existing input-repeat routine remains in its separate translation unit.

The regenerated objdiff report is 100% exact. All eleven JP1 linked executable/overlay SHA-256 checks pass after the report and final matching rebuild, and the hashes were independently recomputed. No emulator gameplay or native runtime testing was performed.
