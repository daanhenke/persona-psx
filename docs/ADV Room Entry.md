# ADV room entry

`ovl_adv_entry` in `src/adv/game/advmain.c` now uses its existing C reconstruction in matching builds. Chaining the two ordering-table length assignments preserves the original compiler's base-pointer choice and removes the remaining code differences. Its command-bar filename and two jump tables are assigned to the translation unit at the original address; the final four padding bytes remain a separate segment.

The 3,000-byte function initializes a new game when no map exists, prepares room graphics and sound, loads the command bar, runs the arrival script, services the room until it exits, and selects/preloads the next overlay. The assembly fallback is removed. The unused assembly include is replaced by the header declaring `VSync`.

The native fixture executes the entire function against synthetic RAM and platform adapters. Its 512 cases and 286,086 checks cover both display buffers, entry modes -1 through 6, all eight script leave codes, new-game character/Persona/formation initialization, special map IDs, event-room kinds, music selection, the map-3 fade condition, command-bar movement, CD completion during frames, field exit, and destination preloading. Optimized and UndefinedBehaviorSanitizer builds pass.

```sh
cc -std=c99 -O2 -fpack-struct=4 -funsigned-char -fno-builtin -fno-strict-aliasing \
  -Iinclude -Iinclude/psyq tests/native/adv_entry.c -o /tmp/persona-adv-entry
/tmp/persona-adv-entry
```

The fixture keeps PS1 word fields 32-bit and checks the 96-byte character and 64-byte Persona record sizes, while retaining native host pointers. It supplies synthetic room state instead of unpacking game assets. GPU, sound, file loading and script execution are adapters; the saved-script-resume path is not exercised. This establishes isolated native execution of room-entry logic, not a playable Linux boot or rendered opening scene.

Acceptance also requires the real linked function to have zero differing words and all eleven complete JP1 binaries to pass their original SHA-256 checks.
