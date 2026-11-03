# Reel statistics

CASINO `func_8008802C` at `0x8008802C` is the 1,432-byte reel-sampling screen controller. It clears the seven-by-six result counters, slides in the screen panels, generates 21 reel positions, reads their preceding symbols, classifies the winning lines, and accumulates counts and payout values. It refreshes the counter display and stops at the original sample milestones or when the exit input is pressed.

The five symbol groups retain their original iteration order: 5–8, 9–12, 13–16, 17–20, then 0–4. The statistics table also has a byte view for the final result's counter update. The original compiler rereads the rank after that store; preserving the byte view reproduces that aliasing behavior. Computing the row address before adding it to the column offset reproduces the final read-modify-write register allocation.

All eleven complete JP1 binary SHA-256 checks pass with this replacement. The C preserves the original sampling, indexing and stopping behavior; binary equality does not establish a native Linux runtime.
