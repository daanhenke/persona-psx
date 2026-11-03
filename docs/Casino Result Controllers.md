# Casino result controllers

`func_8008915C` (768 bytes) resolves the grid round. It copies nine symbols to
the display, compares the grid, applies the first player's delay, advances the
rolling digits and selects the win or loss state when animation completes.
The player and digit timer use their existing linker field aliases, avoiding
an extra cached base pointer in the original compiler. Shared by-value types
in `persona/casino/grid.h` agree with both display callees. Other recovered
units still use byte-array views of these original tables; a complete native
port must reconcile their global declarations as it integrates those units.

`func_8007EB20` (856 bytes) selects a blackjack result label or formats a two-digit
total. It changes the label width and cell count, optionally plays a sequence
or number voice, shifts adjacent cards and animates the label into position.
Its original fourteen-entry jump table is generated from the C switch. The
old count is a byte and the distance is a short; preserving those widths and
the count update order reproduces the original instructions.

Native fixtures execute the production routines: 36,288 grid-controller cases
and 196,608 blackjack-label cases. These cover state/timer boundaries, all byte
values for label kinds and totals, argument data, audio/animation call order,
callback changes, signed frame durations and untouched neighboring fields.
Both pass normally and under ASan/UBSan with recovery disabled. The grid test
uses the real field aliases on the host. Complete acceptance additionally
requires all eleven JP1 SHA-256 checks, including the generated jump table.
No emulator gameplay or native Linux startup is asserted by these tests.

`func_8008A628` (844 bytes) accepts grid play, restores the coin item on exit,
refreshes two status lamps and the displayed coin count, and handles the
auxiliary pad toggle. Its two halfword writes preserve the original packed
item operations and reload the coin byte between them. Explicit cursor-field
stores preserve the original write order. A further 64,512 native fixtures
cover every lamp combination, coin boundaries, active/inactive requests,
callback changes, cursor guards and packed item bits. Normal and ASan/UBSan
runs pass with recovery disabled.

`func_80089C04` (632 bytes) reveals the target grid, advances its cells and
rolling digits, then enters state 119 once the completion flag is set. Its
final block keeps a scoped ordinary pointer to that flag; a const pointer or
direct global access causes different original-compiler output. The target
grid argument is shared with `func_8008CBF8` through `persona/casino/grid.h`.
Its 15,120 native and sanitizer fixtures cover frame 32, row/column call order,
by-value grid data, callback changes, signed timer wrap and completion states.
