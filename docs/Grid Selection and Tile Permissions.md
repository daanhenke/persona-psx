# Grid selection and tile permissions

Four routines are recovered as C with the original compiler and linker layout:

| Function | Bytes | Behavior |
|---|---:|---|
| `func_8008AAF8` | 224 | Copy a by-value block of nine symbols into three five-cell texture rows and refresh the layout. |
| `func_80088C8C` | 472 | Reveal a selected grid row, open its controls at frame 32, and enter state 112 from frame 44. |
| `func_8008CA28` | 280 | Check the current map tile against two blinking barriers, unless the caller supplies the bypass bit. |
| `func_8009C36C` | 268 | Open or close the map scale in steps of 384, clamp its two visible components and advance its state. |

The nine-byte symbol block occupies three argument registers in the PS1 ABI;
the unused high bytes are not part of the C object. Its shared declaration in
`persona/casino/grid.h` preserves this distinction for future native callers.
Each byte
is interpreted as signed before adding 37; only the first three cells of each
five-cell row change. Named row fields and a shared row index reproduce the
original pointer progression without assembly or volatile workarounds.

Grid selection reads the player once while preparing the texture-copy call and
reads it again after that callback. Its two layouts are fields of adjacent
`CasinoLayoutDef` records. The timer checks remain independent: callbacks may
cause later stages to run during the same invocation.

The tile permission check rereads the map for each barrier. It uses the
header's byte offset, a 200-cell row stride and the original inverted Y
coordinate. `PERSONA_TILE_MAP` defaults to the PS1 address; native fixtures
replace only that address with an aligned synthetic map buffer.

## Validation

The fixtures execute production C and check 16,384 symbol-block cases, 10,800
grid-selection cases and 5,120 tile-permission cases. They cover signed symbol
values, masked player indices, untouched row gaps, timer boundaries, callback
mutations, event order, map corners, barrier flags and neighboring data.
Ordinary builds and ASan/UBSan pass, with sanitizer recovery disabled.

The scale routine preserves a constant local pointer to the three-component
scale record. This makes GCC 2.6 retain the original common base address for
both visible components. Its native fixture follows 1,372 starting conditions
for forty frames each (54,880 checks), including the clamp boundaries,
asymmetric components, inactive states and preservation of the third component.
The tested scale range is -4096 through 8192; arbitrary overflowing inputs are
outside this fixture's claim.

Acceptance additionally requires all eleven complete JP1 binary SHA-256
checks. These tests do not establish emulator gameplay or native Linux startup.
