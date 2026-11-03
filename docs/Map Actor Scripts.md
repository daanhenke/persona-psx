# Map actor movement scripts

`src/s2d/game/actorscript.c` replaces the 1,844-byte S2D function `func_80095700` with matching C. The original 22-entry jump table occupies 88 bytes at `0x80064FF8`. The actor layout preserves packed current/previous coordinates, fractional movement, direction, three script pointers, a repeat counter and a signed wait timer.

The interpreter runs commands until one yields, then advances the actor's fractional position. Crossing a tile records the previous coordinate pair, updates the position, checks occupancy and direction, checks the two signal flags, and optionally selects another script from the original table.

| Command | Behavior |
| --- | --- |
| 0 | Select default script and restart point |
| 1 | Advance and yield |
| 2 | Return to the restart point |
| 3 | Advance past the command |
| 4–8 | Set movement speed to command minus four |
| 9 | Increase speed; cap at four and select script 1 when the signed-byte test succeeds |
| 10 | Decrease speed; select the default script on signed-byte underflow |
| 11–12 | Decrease/increase the turn byte |
| 13–14 | Turn, wrap the direction and clear the affected fractional coordinate |
| 15 | Select a randomized delay script when the axis flag is one; otherwise yield |
| 16–17 | Adjust the other fractional coordinate and select its signed direction increment |
| 18 | Initialize or decrement a wait timer; advance when the timer expires |
| 19 | Select a script by table index |
| 20–21 | Establish and repeat a script loop |

Byte wrapping, signed comparisons and the order of script-pointer updates matter. The wait command's state update and the common pause path also affect the original compiler's branch layout. No inline assembly or instruction-byte substitutes are used.

The native fixture runs 37,915 calls/cases and checks 926,213 conditions. It covers all 22 commands, byte-boundary values, zero-count loops (256 iterations), repeated timer calls, blocked-actor recovery, both movement axes, occupancy masks, failed/mismatched/successful actor lookups, signal conditions and the packed-map query. It checks call order, script destinations, coordinates and preservation of unrelated actor fields. The query retains its existing `int` declaration; the caller explicitly narrows its result to a byte, as the original does.

Optimized and AddressSanitizer/UndefinedBehaviorSanitizer runs pass. Leak checking is disabled under the traced environment. `PERSONA_TILE_MAP` redirects map reads to synthetic, aligned test storage; the production default remains `0x80122000`. Host pointers remain native width, so this fixture tests behavior while the PS1 build verifies the original layout and bytes. Invalid/non-yielding scripts are outside the fixture's input contract.

```sh
cc -std=c99 -O2 -funsigned-char -fno-builtin -fno-strict-aliasing \
  -Iinclude -Iinclude/psyq tests/native/actor_script.c -o /tmp/persona-actor-script
/tmp/persona-actor-script
```

The linked function and all eleven JP1 binary hashes are checked separately. This is decompilation and isolated native testing; it does not yet establish playable Linux gameplay.
