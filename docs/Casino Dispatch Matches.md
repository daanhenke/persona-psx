# Casino dispatch matches

Three JP1 casino functions now use matching C, totaling 692 code bytes:

- `func_800799F0` (232 bytes) dispatches the blackjack play state. Unhandled
  states return without invoking a handler.
- `func_8007FFAC` (232 bytes) selects one of four sequences for card values
  3–11 and the requested signed 16-bit sequence handle index.
- `func_80080094` (228 bytes) selects those sequences for result ranks 0–6.

The switches retain the original case ordering and jump-table regions.
The four-byte gap between the two sequence tables remains a separate data
segment; it is original padding, not an extra C instruction or table entry.

All 11 JP1 executable/overlay SHA-256 checks pass with these units enabled.
These functions still call the original PS1 sound interface. No emulator
or native Linux gameplay test was performed.
