# Layout, exit and coordinate matches

Five JP1 helpers now use exact C, totaling 1,540 code bytes.

| Function | Bytes | Behavior |
| --- | ---: | --- |
| `func_80080E0C` | 272 | Start two blackjack object animations on timer zero; at timer 33 or later, restore sprite ranges, clear the exit flag and move to the saved state. |
| `func_800906D0` | 344 | Tween four layouts when the state byte changes, choosing opposite directions for zero and nonzero states, then reload and save the current state. |
| `func_8008ABD8` | 296 | Fill four groups of texture indices from a by-value block of four three-byte digit rows, adding the original 47/52/57/62 offsets. |
| `func_8008BA08` | 380 | Update a selected grid model's texture, queue its palette and toggle its mode byte. Modes other than 0/1 skip the texture store but still run the shared update. |
| `func_8008D988` (S2D) | 248 | Copy the original matrix template, apply rotation, translation and scale through PS1 SDK calls, copy the result into a coordinate and clear its flag. |

The digit-row block is a 12-byte aggregate passed by value after the selector.
Its unsigned bytes are explicitly cast to signed bytes before the additions,
preserving the original zero-load followed by sign extension. Texture indices
are reloaded through the layout in each assignment, and the layout table has
an eight-byte stride per selector.

Grid models use a flat nine-element array indexed by `row * 3 + column`.
A two-dimensional model array made GCC distribute the multiplication into
separate row and column strides, producing 412 bytes instead of 380. The
flat index restores the original shared multiplication and register allocation.
Palette lookup deliberately transposes row and column. The palette queue uses
the original unprototyped caller form, as elsewhere in the overlay; its third
parameter is an integer, not a truncated halfword.

The coordinate wrapper uses the existing `MATRIX`, `VECTOR`, `SVECTOR` and
`GsCOORDINATE2` SDK types. Its local matrix initialization and final aggregate
assignment recover both original eight-word copies without manual padding or
inline assembly. Table tweening keeps the calls inside their two arms so the
compiler recovers the original shared final call.

The reconstructed C retains PS1 interfaces and data addresses. All five object
comparisons are exact, and all 11 JP1 executable/overlay SHA-256 checks pass on
the final source after report generation. No emulator gameplay test or native
Linux runtime is included. A separate reward-digit candidate remains assembly
and is preserved locally for further analysis; it is not part of this change.
