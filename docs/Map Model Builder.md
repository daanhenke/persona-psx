# Map model builder

`func_80097508` builds a two-part TMD model with seven primitives, two groups of eight vertices, and six normals. The C preserves the PS1 relative offsets and original arithmetic order. The primitive index loop uses each row of the source table; the Y coordinate shifts before negation and multiplication.

The isolated function is 628 bytes with zero normalized instruction differences and zero linked-byte differences. Objdiff reports 100%, and all eleven JP1 binary SHA-256 checks pass. This is a PS1 routine, not a native Linux renderer.

`func_8009777C` (488 bytes) computes an animated two-color palette from the frame counter and copies colors into seven model primitives. Its palette pointer, independent color cursor, and primitive mode initialization preserve the original register and instruction order. Isolated linked bytes match; objdiff reports 100% and all eleven JP1 binary SHA-256 checks pass.

`func_80097964` (644 bytes) builds two gradient-colored model quads and their vertex groups. Separate index and color cursors preserve the original stores and initialization order. Isolated linked bytes match; objdiff reports 100% and all eleven JP1 binary SHA-256 checks pass.
