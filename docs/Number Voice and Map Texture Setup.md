# Number voice and map texture setup

`func_8007FC94` (792 bytes) selects a casino sequence for values 4 through 21. The caller chooses the sequence channel; each case preserves its original sequence identifier and VAB bank. The 18-entry generated jump table occupies the original 72 bytes at `0x80064EDC`. Values outside the range do nothing.

`func_80098BFC` (816 bytes) uploads a map palette, initializes five textured quadrilaterals, assigns their UV and palette data, and initializes the associated transform. Its two local UV rectangles retain the original 16-byte initializer and two-byte alignment. Primitive layouts retain the PS1 GPU packet sizes and fields.

Both isolated objects match their original linked function bytes. Both functions score 100% in objdiff, and all eleven original JP1 binary SHA-256 checks pass. These routines retain the PS1 graphics and sound interfaces; they are not native Linux replacements.
