# Reel, grid, and scene matches

Six routines replace assembly fallbacks with matching C (1,768 code bytes):

| Routine | Bytes | Behavior |
| --- | ---: | --- |
| func_80085920 | 348 | Queues reel palettes across an ascending or descending range. |
| func_8008945C | 400 | Starts grid music and advances after the sequence ends. |
| func_80089E7C | 404 | Handles grid acknowledgement input and the next scene transition. |
| func_800808D8 | 416 | Animates the card reset and updates card counters. |
| func_80093B74 | 8 | Preserves an empty callback. |
| func_80096454 | 192 | Scans five 96-byte records to select the scene mode. |

The scene scan models only the two accessed bytes, at offsets 10 and 21. Other
record fields remain unidentified. Its type-16 case takes priority and exits
the scan; type 13 selects mode 1 and can shorten the wait by two ticks.

All six routines reach 100% in objdiff. The pinned original-compiler build
passes all eleven JP1 SHA-256 checks; the generated files were also independently
rehashed. This verifies the original PS1 executable output, not a native port.
