# Small helper reconstruction

These JP1 routines now compile from C with the original GCC 2.6.0 toolchain.
The translation units are split at their original file offsets.

| Overlay | Function | Bytes | Behavior |
| --- | --- | ---: | --- |
| Casino | func_8007D9F4 | 12 | Store card byte 0x25 |
| Casino | func_800915E4 | 24 | Test an indexed word for nonzero |
| S2D | func_80096DE4 | 40 | Copy words forward, preserving overlap order |
| S2D | func_80099930 | 28 | Store spacing multiplied by 192 |
| S2D | func_800999D0 | 48 | Set a map entry's state and associated word |
| S2D | func_8009C33C | 48 | Restore three fixed-point scales and state |
| Casino | func_8007D47C | 60 | Advance dealer state when its timer expires |
| Casino | func_8008D6A4 | 64 | Advance after a pending operation finishes |
| Casino | func_80090E9C | 80 | Reset an 11 by 11 wrapping cursor |
| Casino | func_8007C080 | 100 | Handle the selected natural-hand state |
| Casino | func_8007C0E4 | 116 | Select the next natural-hand state |
| Casino | func_8008CF68 | 88 | Apply an operation to an inclusive 16-bit range |
| Casino | func_8009065C | 116 | Expire an idle state after 600 ticks |

Total: 13 functions and 824 bytes. All 11 JP1 executable/overlay SHA-256
checks pass. No emulator gameplay test was performed.

The timer's byte aliases name existing storage at symbol offsets; they add
no data. Keep the direct aliases so the compiler uses the original loads
and stores instead of retaining a separate base pointer. The range helper
retains the original 16-bit wrap behavior, including the endpoint edge case.
Original function names remain until their surrounding callers are understood.
