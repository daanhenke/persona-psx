# Pad repeat, coin return and map transition matches

Three JP1 helpers now use exact C, totaling 928 bytes.

| Function | Bytes | Behavior |
| --- | ---: | --- |
| S2D `func_8008E1C0` | 200 | Read pad input, retain previous/held/newly pressed bits, and emit repeat input on hold counts 1 and 7, saturating the counter at 7. |
| Casino `func_8007A77C` | 312 | Queue a palette and object animation at timer zero, play the original sequence at 28, and return one bet coin per update from timer 61 onward; animate out and advance the blackjack step when the bet reaches zero. |
| S2D `func_8008AEA8` | 416 | Apply a Q12 map-transition offset, update completion/save-cycle state, rebuild scene and root coordinates, and run the final map-effect update. |

The pad and coin helpers matched their first typed drafts. Pad history and
repeat counters use their original scalar symbols, including the counter's
unsigned saturation comparison and explicit no-repeat store.

The transition helper uses existing VECTOR/SVECTOR/GsCOORDINATE2 types and
the already matched coordinate builder. The callbacks at 0x80096224 and
0x80096514 take no arguments; m2c inferred three incidental live registers
at the first call. Assembly inspection confirms that callback overwrites
its argument registers before reading them. No invented arguments were added.

Independent aliases for the transition step and map transform components
retain original loads. Direct aggregate member access let GCC cache base
addresses, producing 404 bytes instead of 416; the aliases restore the
original instructions. The save byte at 0x801F2B30 and its four-bit wrap
are preserved. The local VECTOR's unused pad is left untouched as in the
target; no fake data or instruction padding is added.

All three objdiff comparisons are 100%, and all 11 JP1 executable/overlay
SHA-256 checks pass after final report generation and rebuild. No emulator
gameplay testing or native Linux backend is included.
