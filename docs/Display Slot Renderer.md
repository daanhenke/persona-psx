# Display slot renderer

`SlotsRenderAll` now builds from the shared C implementation in DNG, ADV and
S2D. Each copy is 1,428 bytes and matches its original instructions exactly.
All eleven JP1 binary SHA-256 checks pass.

The remaining scheduling difference was the unscaled cel pointer. Advancing
`b` before converting it to a cell pointer preserves the original GCC 2.6
load-delay scheduling. The next entry assigns `b` afresh, so the advance does
not carry into another cel block. No inline assembly or instruction data is
used. The original 160-byte stack reservation remains.

The ADV copy at 0x80065730 was previously classified as a raw data segment.
It is now a C code segment, using the resident `GsSortSprite` at 0x8003262C.
This corrects the measured code total by adding 1,428 bytes and one function;
it is not extra game content. The S2D raw assembly segment and DNG assembly
fallback are also replaced by the shared implementation.

## Validation

The native fixture executes the actual renderer and observes GPU submissions.
It covers all 80 slots, both ordering tables, scaled and unscaled cel blocks,
transparent cells, brightness/flicker/fade handling, hidden and inactive slots,
and frame, end, stop, jump, wait and offset-bearing animation commands.
Both optimized and UBSan runs pass 9,600 cases and 535,680 checks.

```sh
cc -std=gnu99 -O2 -funsigned-char -fno-builtin -fno-strict-aliasing \
  -fwrapv -fpack-struct=4 -Iinclude -Iinclude/psyq \
  tests/native/slot_render.c -o /tmp/persona-slot-render
/tmp/persona-slot-render
```

For UBSan, replace `-O2` with `-O1 -g -fsanitize=undefined
-fno-sanitize-recover=all`. Four-byte structure packing and wrapping arithmetic
adapt the fixture to the original PS1 data conventions on a 64-bit host.
Pointer-bearing native structures still use host widths: the eleven original
binary comparisons independently establish the PS1 layouts and code.
The synthetic fixed-address ranges may be unavailable, including with ASan;
the fixture then skips with status 77. No ASan pass or native game playability
is claimed. No game assets are needed by this fixture.
