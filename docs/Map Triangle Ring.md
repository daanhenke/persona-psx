# Map triangle ring

`func_80096A38` constructs 64 points in PS1 scratch memory, deforms opposing
halves with frame-dependent sine offsets, closes the ring with a 65th point,
and emits 64 shaded triangles. Each group of sixteen triangles uses one of
four stored center coordinates. A pulsing rectangular backdrop and draw-mode
packet follow. The draw buffer is read again after callbacks, preserving the
original copy-versus-queue behavior when it changes.

The scratch region is a typed triangle followed by 65 two-component vectors.
The output region contains two draw modes, two backdrops and two banks of
64 triangles. Unsigned right shifts followed by halfword truncation preserve
the original point conversion. Point traversal and advancing the pointer
after the copy reproduce the compiler's schedule without assembly patches.

The complete linked binary matches exactly. The object report remains at
99.914894% because four scratch-memory HI16/LO16 operands use literal
addresses in C and symbolic addresses in the reference assembly. Published
progress retains that conservative score; this function is not counted as a
100% object match.

7,680 native fixtures cover both buffers, positive and negative frame values,
three bounded trigonometry stubs, center-coordinate boundaries, ring closure,
colors, every packet field, untouched output storage, scratch canaries,
frame changes during trigonometry calls, and buffer/ordering-table changes
inside callbacks. They pass normally and under ASan/UBSan with recovery
disabled. The adapter replaces the fixed scratch address and packet copy;
these tests do not validate the SDK sine approximation, GPU rasterization,
or native game startup.

```sh
cc -std=c99 -O2 -fno-builtin -funsigned-char -fno-strict-aliasing -Iinclude -Iinclude/psyq tests/native/map_ring.c -o /tmp/map-ring
/tmp/map-ring
```
