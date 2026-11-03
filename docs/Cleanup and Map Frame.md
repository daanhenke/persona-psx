# Cleanup and map frame routines

`func_8007A9F8` (992 bytes) clears the blackjack hand presentation, animates cards away, resets result labels and transforms, then returns to the selection state. Transform addresses are read through the original array-backed storage, preserving reloads after writes.

`func_80093AE0` (792 bytes) submits a map frame: clipping and window drawing, indicators, drawing synchronization, ordering tables, framebuffer transfer, and frame counter advancement. The lower rectangle coordinates preserve a 16-bit intermediate before subtracting the doubled offset.

Both functions score 100% in objdiff and preserve all eleven JP1 binary SHA-256 hashes. These are PS1 routines, not native Linux replacements.

`func_8008F2B8` (1,324 bytes) resets the dice state and two transform records. Its overlapping byte view preserves the original duplicate write at address `0x800AAE9B`; no store is removed. Isolated linked bytes match, objdiff reports 100%, and all eleven JP1 binary SHA-256 checks pass.
