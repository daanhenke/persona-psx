# Preview transforms and grid/reel state

Five saved hard cases now compile to the original JP1 bytes:

- `func_80093178` (1,180 bytes): initialize and draw paired preview transforms. A shared transform array and an inline helper taking rotation, translation and scale by value preserve the original copies and aliasing.
- `func_8008CBF8` (532 bytes): advance a 3×3 grid toward a target passed by value. Two-dimensional indexing, full-width loop counters and the cursor animation helper preserve register allocation.
- `func_800925DC` (820 bytes): initialize preview parameters and convert the double vectors to fixed transform data. The conversion functions receive a double-vector aggregate by value.

- `func_8008C9DC` (540 bytes): compare matching positions and displaced symbols across both axes of the 3×3 grid. Recovering the two-dimensional grids and four groups of three counters restores the original induction variables.
- `func_80087964` (420 bytes): swap active unequal reel rows. The row-array layout, paired temporary buffers and short conversion of the expanded row count preserve the original copies and operand order.

The draw helper `func_80092DEC` now declares its separate vector arguments consistently; its 296 original bytes remain unchanged.

All five new functions pass isolated linked-byte comparisons and report 100% objdiff. The complete original-compiler build passes all eleven JP1 SHA-256 checks, independently recomputed after the build. These are matching PS1 functions; no native Linux gameplay milestone is added.
