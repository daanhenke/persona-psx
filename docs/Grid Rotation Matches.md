# Grid rotation matches

`func_8008AEE0` (272 bytes) and `func_8008AFF0` (284 bytes) now use matching C for the vertical and horizontal casino grid rotations.

Both recover the model index before deriving the cell address through a separate table-base pointer. The even-cell test is written as `!(*entry % 2)`: with the original GCC 2.6.0 this produces the original mask and unsigned comparison, whereas the bitwise spelling produced an XOR and mask. The horizontal routine selects the doubled signed cell value plus parity; the vertical routine selects the signed cell value. Model texture updates, animation axes, signed direction narrowing and speed division remain intact.

Both are 100% in the regenerated objdiff report. All eleven JP1 executable/overlay SHA-256 checks pass after the report and a normal matching rebuild. No inline assembly, padding, compiler changes or reference-binary edits were used. No native runtime or emulator gameplay testing is claimed.
