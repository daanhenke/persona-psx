# Map fade transition

`func_80092C6C` is now matching C in S2D, replacing 1,208 bytes of assembly.
All eleven JP1 binary SHA-256 checks pass, the function's linked bytes are
identical, and the object comparison reports 100%.

Mode zero draws the field, starts loading LTS, captures the screen into four
textured strips and fades those strips in over black. Other modes draw the
current screen, start loading ADVCMD and fade the captured strips out.
Two ordering tables and two copies of the strips support either frame buffer.

The original six-level steps are preserved: fade-in renders 22 frames ending
at brightness 132; fade-out renders 21 ending at brightness 2. These are not
rounded to 128 or zero. Active-buffer reads and callback-visible fade values
also keep their original ordering. Layout-derived byte indexing preserves
GCC 2.6's colour-store instruction order without hardcoded host offsets.

## Validation

The native fixture runs the actual C for 192 combinations of mode, captured
screen position, frame-buffer pattern and callback mutations. It checks
209,616 conditions, including call order, captured rectangle, quad geometry,
UVs, texture pages, both copies, fade endpoints and frame counts. Optimized
and ASan/UBSan builds pass. This is a synthetic graphics fixture, not gameplay.

```sh
cc -std=gnu99 -O2 -Wno-error=incompatible-pointer-types \
  -funsigned-char -fno-builtin -fno-strict-aliasing \
  -Iinclude -Iinclude/psyq tests/native/map_fade.c -o /tmp/persona-map-fade
/tmp/persona-map-fade
```

For sanitizers, replace `-O2` with `-O1 -g -fsanitize=address,undefined
-fno-sanitize-recover=all`. The legacy SDK's byte-pointer declarations can
produce host pointer-type warnings. LeakSanitizer cannot run inside the
traced sandbox, so use `ASAN_OPTIONS=detect_leaks=0` there; address and
undefined-behaviour checks remain enabled. No game assets are required.
