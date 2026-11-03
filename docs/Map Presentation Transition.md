# Map presentation transition

`func_80093DF8` replaces 1,948 bytes of S2D assembly with matching C. It
captures the field, presents the map window with two gradients, waits for
the window's response, then restores field rendering and any pending message.

The routine preserves the two VRAM backups, four textured capture strips,
geometry offset save/restore, projection inputs, display dimensions and font
reinitialization. Its animation reaches 48 in increments of two, waits for
the response value one, then returns to zero. Each endpoint is rendered once
more when the state advances. Overshoots retain the original clamping.

The capture rectangle and its trailing width/height label overlap in the
original memory layout. The four strips and second gradient follow that
tail. Those linker labels are retained; no instruction bytes or inline
assembly are used to implement the function.

## Validation

The isolated function has zero differing linked words against JP1. Acceptance
also requires the full eleven-binary SHA-256 gate and a 100% object comparison.

The native fixture exercises the actual C for 108 combinations of frame
buffer, response delay, endpoint perturbation, pending message and captured
screen position. It observes 2,304 animation frames and checks 25,884
conditions, including GPU call order, VRAM rectangles and buffer addresses,
gradient colours, texture pages, projection inputs, generated quad pointers,
geometry restoration, font setup and message restoration.

```sh
cc -std=gnu99 -O2 -funsigned-char -fno-builtin -fno-strict-aliasing \
  -Iinclude -Iinclude/psyq tests/native/map_present.c -o /tmp/persona-map-present
/tmp/persona-map-present
```

Optimized and ASan/UBSan builds pass. For sanitizers use `-O1 -g
-fsanitize=address,undefined -fno-sanitize-recover=all`; under the traced
sandbox, `ASAN_OPTIONS=detect_leaks=0` disables only LeakSanitizer.

The fixture substitutes observed SDK calls and synthetic projection results.
On LP64 it writes projected coordinates as two shorts, matching the SDK's
32-bit output without overrunning the host struct. Overlapping linker labels
have separate fixture storage. These checks do not emulate VRAM, validate
the real GTE projection implementation, or demonstrate game playability.
