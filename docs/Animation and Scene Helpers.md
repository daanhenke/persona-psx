# Animation and scene helpers

Nine JP1 functions now use matching C, totaling 1,320 bytes:

| Function | Behavior | Bytes |
| --- | --- | ---: |
| `func_8007F160` | Dispatch animation modes 0x70 through 0x73; ignore other modes | 112 |
| `func_8007F368` | Select one of two animation steps from side and event | 144 |
| `func_8007C2B8` | Restore the bet on entry and dispatch blackjack results | 160 |
| `func_8007D4B8` | Start the dealer animation and refresh its card total | 156 |
| `func_8008A9DC` | Select a layout texture and start its horizontal tween | 136 |
| `func_80091154` | Set two decimal digit textures and refresh the layout | 144 |
| `func_80091544` | Start the one- or two-digit animation | 160 |
| `func_8008AE00` (S2D) | Rotate the relative map displacement with the original PSYQ routines | 168 |
| `func_80096198` (S2D) | Walk scene records until a condition is unmet or the sentinel is reached | 140 |

All 11 JP1 executable/overlay SHA-256 checks pass with these C units enabled.
No emulator gameplay test was performed. The source preserves original
byte/short truncation, signed coordinate arithmetic, supported switch cases,
record stride, and call order. The matrix routine still calls the PS1 SDK;
this change does not implement a native graphics backend.
