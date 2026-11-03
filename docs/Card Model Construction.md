# Card model construction

`func_800824CC` selects one of 256 card objects, sets its first sprite number,
initializes depth and visibility for twenty faces, and passes a temporary model
to `CasinoBuildObj`. The object's face-count field does not change the twenty
iterations. The unused second argument is retained.

The five contiguous pointer fields at `D_8009E8D4` form one card-data bank.
Using that record and indexed face accesses reproduces all 216 original bytes;
separate pointer globals incorrectly let the compiler share definition loads.
No volatile qualifiers, assembly padding or compiler-output edits are used.

The 23,040 native fixtures cover every object index, signed table selections
against centered synthetic tables, first-sprite boundaries and differing face
counts. They check model arguments, all face updates, surrounding faces,
vertex/padding preservation and untouched neighboring objects. Both ordinary
and ASan/UBSan runs pass. Synthetic negative selections check signed indexing;
they are not a claim that the game uses those selections.

Acceptance also requires all eleven full JP1 SHA-256 checks. These fixtures do
not establish gameplay or native game startup.

## Stopped-row settlement

`func_80085B74` (388 bytes) waits for row state five and a signed timer of at
least seventeen, then restores four textures and advances the row to state six.
It increments the timer on every call, including when no settlement occurs.
The loop end is retained only inside the loop's guard; a scoped table-base
local preserves address evaluation order. Pointer steps use `sizeof(*table)`
so the same C can be exercised with host-sized pointers.

The entire row workspace is declared together, keeping all three helper
arguments within the same allocation. `tests/native/casino_cardrowsettle.c`
checks 5,920 cases across all four rows, state/timer boundaries, all signed-byte
symbol indices in centered synthetic maps, neighbor guards and callbacks
changing timers, state, maps or future digits. Ordinary and ASan/UBSan runs
pass; the full binary acceptance check remains required.
