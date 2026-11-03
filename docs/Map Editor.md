# Map editor

`src/s2d/game/mapeditor.c` reconstructs the 1,756-byte S2D debug/editor handler `func_8008C34C`. It selects and previews map models, picks up and places packed tile values, changes rotation and tile flags, wraps the editor cursor over the 200-by-200 grid, and preserves the original PC-host map export path.

The model resolver binds each branch directly to the preview object. Combining those branches into a shared resolved-value temporary changes GCC 2.6.0's branch merging. The coordinate-link address, pitch snapshot and separate yaw operations preserve the original instruction order. Three scalar/structure aliases refer only to existing fields. The placement path saves the held tile before resetting the selection to 255.

Validation uses the pinned original compiler and all eleven full JP1 SHA-256 checks. No executable inline assembly, artificial padding, compiler-option changes or binary substitutes are used. This is matching PS1 code; it does not implement the Linux runtime or replace the original host-debug API.
