# Map frame dispatcher

`src/s2d/game/framefinish.c` replaces S2D `func_80089F5C` (3,252 bytes) with matching C and assigns its three jump tables (88 bytes) to the C translation unit.

The routine attaches draw modes, synchronizes VRAM transfers, updates object/packet high-water marks and optionally prints the original diagnostic menus. It then synchronizes the frame, flips map pages and display buffers, clears and draws seven ordering tables, restores the fade color to 96, advances the frame counter and calls the final timing helper.

| Mode | Behavior |
| --- | --- |
| 0 and values outside 1–4 | Finish the frame without a diagnostic menu |
| 1 | Map position, CD/key status, tile and memory statistics |
| 2 | Twelve map/event controls with a blinking selected row |
| 3 | Five sound controls and animated sequence bars |
| 4 | Nine tile diagnostic rows, local label tables and tile overlay drawing |

The display-blanking flag is read again after VRAM transfers. The selected buffer is read again after the swap. Those ordering details matter. Separate cursor-print calls preserve the original compiler's delay-slot schedule; ordinary local label copies preserve its stack workspace. There is no inline assembly or instruction-byte substitution.

The native fixture executes 7,840 cases and 2,033,914 checks with synthetic state. It covers all modes, selections inside and outside the menu, both buffers, blinking/sequence frames, packet/object maxima, event results, packed tile fields, the two label tables, flag changes during VRAM flushing and buffer changes during swapping. It checks print arguments and ordered graphics/helper calls. Optimized and UndefinedBehaviorSanitizer builds pass. The fixture uses a fixed synthetic RAM mapping for the original PS1 addresses and does not require game assets. Its formatting adapters check arguments rather than rendering text; it does not test real GPU output or playable Linux gameplay.

```sh
cc -std=c99 -O2 -funsigned-char -fno-builtin -fno-strict-aliasing \
  -Iinclude -Iinclude/psyq tests/native/map_frame.c -o /tmp/persona-map-frame
/tmp/persona-map-frame
```

The real linked code has zero differing words, the object report is 100%, and all eleven complete JP1 binaries pass their original SHA-256 checks.
