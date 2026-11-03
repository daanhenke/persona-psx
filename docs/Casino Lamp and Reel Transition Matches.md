# Casino lamp and reel transition matches

Two JP1 casino helpers now use matching C, totaling 516 code bytes:

- `func_8008C4D0` (268 bytes) sequences lamp sprite groups at specific frame
  values, plays the original sound at frames 0, 0x20 and 0x40, sets the two status bytes to 0 and 1 at 0x48, and increments the frame counter. Unhandled
  frames still increment. Its original 73-entry jump table stays in place.
- `func_800867A8` (248 bytes) applies four paired tween/scroll operations to
  the two reel-layout rows. Each row contains 21 layout pointers. The frame
  byte is interpreted as signed, and the signed delta is negated for the
  complementary operations. The caller retains the original argument
  promotions used by the existing tween/scroll interface.

All 11 JP1 executable/overlay SHA-256 checks pass with these units enabled.
No emulator or native Linux gameplay test was performed. These helpers
still use the original PS1 graphics and sound interfaces.
