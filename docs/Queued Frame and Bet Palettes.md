# Queued frame drawing and bet palettes

Two JP1 helpers now use exact matching C, totaling 668 bytes.

- S2D `func_8008AC10` (496B) resets the graphics queue, adds menu and scene draw-mode primitives, reads the draw environment, clears with the fade colour, draws the object/map/front/back ordering tables and all three layer tables, waits for drawing and advances the frame counter.
- Casino `func_8009019C` (172B) queues one of two palette identifiers for each of eleven bet entries and sets the shared presence flag according to whether any entry is nonzero.

The S2D helper reuses the existing graphics globals and SDK types in
`persona/s2d/s2d.h`, following the already matched frame code's interfaces.
The precise draw order is preserved. It does not add the buffer swapping,
upload flushing or other work performed by the separate S2dEndFrame routine.

The bet helper keeps a local entry pointer separate from its input parameter.
Advancing the parameter itself swapped two saved registers and changed twelve
instruction words, although the body still measured 172 bytes. A local pointer
recovers the original initialization and allocation. Explicit nonzero/zero
flag stores preserve the original final branch structure.

Both objects compare at 100%, and all 11 JP1 executable/overlay SHA-256 checks
pass after final report generation and rebuild in the pinned environment.
These functions retain PS1 interfaces. No emulator gameplay testing or native
Linux backend is included.
