# Casino timing and reel helpers

Five JP1 casino functions now use matching C, totaling 900 bytes.

The three timing selectors (`D_80086AC4`, `func_80086B88`, and
`func_80086C4C`, 636 bytes) select byte timing fields from an indexed signed
16-bit state. Each switch preserves the original cases, field updates,
and behavior for states outside those cases.

The reel helpers (`func_800870B4` and `func_80087124`, 264 bytes) decrement
signed byte positions, wrap zero to 27, and resolve symbols through the
per-reel lookup tables. One handles the first five reels; the other handles
four entries selected by its signed byte group argument.

All 11 JP1 executable/overlay SHA-256 checks pass with both source units
enabled. No emulator gameplay test was performed.
