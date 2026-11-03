# Map actor placement

`func_80094660` seeds the random generator from `VSync(-1)` and fills 96 actor
records. It snapshots three colors before either callback, chooses a color
and model for each attempt, and generates direction-dependent coordinates.
After 24 accepted actors in directions 0 or 1, subsequent direction choices
are restricted to 1 or 3. The original direction-specific last-coordinate
store order and untouched record fields are retained.

An occupied tile rejects the attempt without querying neighbors. Otherwise,
the second neighbor query occurs only when the first returns true; two true
results also reject the attempt. Rejection retries the same actor slot.
Acceptance marks the current tile with bit 0x800 and advances the slot.
The tile block offset and actor coordinates are re-read after callbacks.
The two rejection reasons are expressed as separate readable conditions.

The recovered function is 980 bytes and matches an isolated link with real
symbol addresses. A bounded decomp-permuter search identified the control-flow
sensitivity; its redundant-branch output was discarded. A normal if/else-if
form produces the exact code. The neighboring-cell wrapper's argument is also
corrected from an integer to a pointer, matching the actor address passed by
this caller and forwarded to the coordinate helper.

The fixture checks 2,048 complete 96-actor placements, all actor records,
160,128 bytes of map storage and canaries, random/callback traces, initial
occupied cells, signed color bytes, model changes, coordinate changes and
map-block changes inside callbacks. Optimized Linux and
AddressSanitizer/UndefinedBehaviorSanitizer runs pass with recovery disabled.
Acceptance additionally requires all eleven complete JP1 binary hashes.

The native fixture replaces the fixed PS1 map address, random generator and
neighbor helpers. It validates this function's behavior; it does not show
that the game starts or renders natively.

```sh
cc -std=c99 -O2 -fno-builtin -funsigned-char -fno-strict-aliasing \
  -Iinclude tests/native/map_actors.c -o /tmp/mapactors
/tmp/mapactors
```
