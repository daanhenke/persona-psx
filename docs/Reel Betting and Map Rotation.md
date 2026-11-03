# Reel betting and map rotation

## Reel betting

`func_80082E1C` starts the reel bet palette animation, accepts coins on alternate
frames in maximum-bet mode, updates the bet display, and handles acceptance or
exit. The layout pointer is the `l` field of the existing `CasinoLayoutDef`.
Reading that field through its parent structure restores the original compiler's
argument setup; treating it as a standalone pointer changes scheduling.

The displayed-bet sentinel255 skips a zero-bet scroll. Display changes read the
current bet again after palette callbacks. The last coin does not play the
insertion sound when it simultaneously reaches eight and leaves no money.
Acceptance changes step112 and timer -1; exit refunds after sound callbacks,
clears the relevant lamps, fades the table, and selects game31.

Sixteen native fixtures cover input delay, alternating frames, the last coin,
positive and negative display shifts, the sentinel, exact-one flags, acceptance,
refunds, and callbacks changing the bet or balance. The palette mock addresses
the original PS1 on-byte offset; it does not interpret the host-sized palette
structure. No display or audio rendering is tested here.

## Map rotation

`func_8008BA9C` decrements a positive turn countdown, adds128 times the signed
turn step to the map's16-bit Y rotation, and updates input state. At the last
step it writes the saved facing and sets `g_key_lock` to1. Zero and negative
countdowns leave the state unchanged.

Direct access to `g_map_xform.rot.vy` matches the original120-byte routine.
Using a separate pointer temporary changes instruction order. The native
fixture covers120 boundary combinations, including signed rotation wrapping,
countdowns at zero and one, and preservation of other transform fields.

Both routines require all eleven original JP1 binary SHA-256 checks. Native
fixtures also run with address and undefined-behavior sanitizers; they do not
establish emulator gameplay or a native Linux game boot.

## Panel sliding

`func_80087F48` moves three layouts in alternating directions while translating
their associated object. Byte zero slides out; every other byte slides in.
Each subsequent layout is read after the preceding tween call.

All three layout pointers are read as `CasinoLayoutDef.l` fields. Horizontal
tween distance is a full-width `int`, narrowed when stored in the tween argument
record. The other tween parameters keep their existing widths; changing stack
parameter widths alters the original wrapper's load instructions. The wrapper
and existing callers retain their original bytes. The panel routine's512 native
fixtures cover all byte modes and callbacks replacing the next layout pointer.

## Reward-digit reveal

`func_800913FC` accepts a byte value. Values below ten update and animate the
single digit; other values write the remainder and quotient to the ones and
tens models, update those textures in that order, and animate tens then ones.
The three adjacent `CasinoModel` entries share one typed array. Object pointers
are read after texture callbacks, as in the original assembly.

The native fixture covers all 256 byte inputs both with and without callbacks
replacing the animated objects. It removes only the PS1 linker alias so the same
C body can use a host-sized model array. The 512 cases also check unused digits
and call order. Sanitizers and all eleven binary hashes are acceptance gates.

## Entering reel betting

`func_80082C80` resets the cursor records, palette selections, lamps and digit
buffers on frame zero. At frame 33 or later it starts the palette only when
inactive, reveals the controls, and enters step 38 with timer -1. The second
phase reads the timer again after callbacks from the first phase.

Separate cursors for the two descending buffer clears reproduce the original
412 bytes. The 54 native cases cover frame boundaries, nonzero palette states,
callbacks changing the timer or next layout, exact sprite ranges, and untouched
neighboring fields. Both ordinary and sanitizer builds pass.

## Rank palettes

`func_800844D4` selects a palette from the current rank;
`func_80085090` reads the rank from a signed-byte result index. Both use the low
byte of the active argument: zero chooses the second bank of seven records.
The rank and record offset are cached across the CLUT callbacks, and each record
keeps the original 24-byte PS1 layout. Coordinates are read through the whole
bank rather than treating an individual short field as an array.

The address temporary uses `unsigned long`, matching the original compiler's
32-bit arithmetic while retaining the full pointer width on Linux. The native
mocks do not interpret the records as host-sized `CasinoPalAnim` structures.
All 2,940 cases check coordinate selection, the CLUT range, the final animation
pointer, masked active arguments, and callbacks changing the source rank.
Normal and address/undefined-behavior sanitizer runs pass. These routines add
176 and 188 exact bytes; acceptance requires all eleven complete binary hashes.

`func_80084FC4` uses the same selection sequence for four card kinds, numbered
3–6 in the input array. Its signed-byte conversion subtracts three before
selecting the CLUT group (`rank * 8 + 160`). The inactive bank starts four
24-byte records later. Its 204 bytes also match the original. The combined
rank-palette fixture now checks 4,500 cases, including all four valid card kinds
and callbacks changing the input array after the rank has been cached.

## Staged reel loading

`func_800829F4` loads six graphics stages on frames 0–5, then the sound and
reel layouts at frame 6. It initializes five and sixteen texture pairs, with
the first member using the selected texture plus eight. The shared
`CasinoLayoutDef` arrays preserve the original address calculations and all
652 bytes. Its independent timer checks allow a callback to advance loading
within the same call. At frame 39 or later it advances to step 18.

The 844 native fixtures cover each loading boundary, callback changes to the
timer at every call site, all 256 signed-byte table indices using centered
fixture tables, texture-pair writes, neighbor guards and call order. These
synthetic tables exercise the signed-index behavior; they do not assert that
every index occurs in the game. Both ordinary and ASan/UBSan runs pass.
