# Map screen capture

`src/s2d/game/mapcapture.c` replaces the 832-byte S2D routine `func_80095E34` with matching C. It prepares four textured strips from the captured field, projects them using the original horizontal scale of 2564, restores the geometry offset, draws the strips and copies the resulting draw rectangle to VRAM at (0, 256). It then restores the 320 × 240 display dimensions and origin (0, 120).

The local workspace and byte-offset polygon view retain the original compiler's address calculations. Polygon pointers are assigned after vertex preparation, and each vertex uses a local pointer. These details preserve register lifetimes without inline assembly or instruction bytes.

The native fixture exercises 144 combinations of draw-buffer switching, geometry offsets, source rectangles and projection-result clobbering. It checks 18,432 conditions, including call order, vertex/texture coordinates, colors, matrix handoffs, restoring the geometry offset, selecting the post-frame draw environment and delaying display-setting writes until after vertical synchronization. Optimized and AddressSanitizer/UndefinedBehaviorSanitizer runs pass. Leak checking is disabled under the traced test environment. SDK coordinate outputs are written as two shorts because host `long` is wider than the PS1's.

Run the fixture from the repository root:

```sh
cc -std=c99 -O2 -funsigned-char -fno-builtin -fno-strict-aliasing \
  -Iinclude -Iinclude/psyq tests/native/map_capture.c -o /tmp/persona-map-capture
/tmp/persona-map-capture
```

The fixture observes the graphics interface; it does not emulate the GPU or establish playable native Linux support. Exact PS1 compatibility is checked separately against the original linked function and all eleven JP1 binary hashes.
