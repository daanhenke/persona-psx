# Map gradient-panel configuration

`func_8009994C` (S2D offset `0x355AC`, 132 bytes) initializes a panel record consumed by the existing panel update and draw routines. It sets state 1, stores the alignment bits shifted left once, centers the screen coordinates around (256, 120), and saves width, height and the update rate.

The arrays at `D_800B5E28`, `D_800B5E2C` and `D_800B5E30` are views into the same 40-byte record layout. Twenty-halfword indexing preserves the original compiler's shared byte offset. Writing the alignment before x preserves instruction scheduling without changing the stored values.

Acceptance requires a 100% function comparison and all eleven complete JP1 binary SHA-256 checks. This PS1 matching result does not claim a new native Linux runtime test.
