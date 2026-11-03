# Grid setup and prompt timing

`func_8008BD08` (700 bytes) builds grid layouts and models, clears the nine model-group fields and prepares two six-model groups. Explicit views of data inside the merged `D_800A2BC8` region preserve the original loads. Separate loop offsets and 32-bit address calculation preserve register allocation.

`func_8008C2DC` (500 bytes) advances the prompt timer, blinks sprites at the original frame cases, and marks completion at the requested duration. Its compiler-generated jump table occupies 996 bytes at `0x800655D8`.

Both functions match isolated linked code and report 100% in objdiff. All eleven JP1 SHA-256 checks pass after the full build.
