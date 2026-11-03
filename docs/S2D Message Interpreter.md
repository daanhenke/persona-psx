# S2D message interpreter

`MsgStep` replaces 3,320 bytes of assembly in the S2D field overlay with C.
It reads dialogue and control codes, draws glyphs, scrolls the message window,
handles pauses and choices, and inserts names, money and inventory counts.

The existing DNG interpreter supplies the shared logic. S2D calls `SlotSetPos`
with signed short coordinates and reads `InputCheckAcceptA` as a byte. These
call signatures account for its extra 20 bytes. S2D also reads its own held-pad
buffer, `g_pad_held_s2d`, at `0x800FC000`.

The source owns the original 84-byte rectangle/switch-table block at overlay
offset 4. Six previously anonymous data labels now identify message scripts,
palette data, blinking animation, choice marks, cursor definition and answer.
No game assets or instruction data are embedded in the C implementation.

## Validation

The isolated function has zero differing linked words against JP1. The complete
unit has a 100% object comparison, and all eleven rebuilt JP1 binaries pass
their original SHA-256 checks.

The native fixture executes the actual C for 681 synthetic cases and 685
interpreter calls, checking 48,614 conditions. It covers single/two-byte glyphs,
all cursor positions and wrap boundaries, scrolling, waits, the separate held
and pressed input buffers, choice acceptance, signed coordinate narrowing,
message termination, control codes, names, backwards inserts and decimal
inserts. UI and graphics calls are observed substitutes.

```sh
cc -std=c99 -O2 -funsigned-char -fpack-struct=4 -fno-builtin \
  -fno-strict-aliasing -ffunction-sections -fdata-sections -Wl,--gc-sections \
  -Iinclude -Iinclude/psyq tests/native/s2d_message.c -o /tmp/persona-message
/tmp/persona-message
```

The optimized and UBSan builds pass. Add `-fsanitize=undefined
-fno-sanitize-recover=all` for UBSan. The fixture privately maps the function's
fixed work/save addresses and uses four-byte structure alignment on LP64.
ASan is not claimed because those addresses overlap its shadow-memory region.
Host pointers retain their native width; binary layout and original address
resolution are checked by the PS1 build. These tests do not demonstrate native
game boot or rendered dialogue in a playable port.
