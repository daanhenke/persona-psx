# Scene encounter selection

`func_80096224` recovers 560 bytes of encounter selection as C. Once the fade
state permits selection, an already-pending encounter chooses one of thirty
map entries. Otherwise a counter advances and clamps at sixteen, a fresh
record is selected, and a random value is compared against one of four copied
probability thresholds. A successful new encounter clears the delay and key
lock and requests the map transition; the pending path returns zero and a
newly selected encounter returns one.

The probability table is copied before callbacks run. Record and scene-byte
reads occur after the relevant random calls. The pending path caches the map
halfword and jumps to the shared zero return; the new path defers its random
index calculation until the record read. These ordinary C forms preserve the
original compiler's two distinct tails. The active field has a scalar linker
alias. `PERSONA_ENCOUNTER_FLAG` defaults to the original fixed byte address;
native tests replace that address with local storage.

The production C passes 229,376 native fixtures normally and under ASan/UBSan,
with recovery disabled. They cover fade/active states, counter clamp boundaries,
all probability rolls from 0 through 31, table categories, selected-entry bounds,
high-bit map values, callback record changes and probability-table mutations.
Random values used for map indices are nonnegative, as the game RNG specifies;
invalid table categories and overflowing counter inputs are outside this test.
All eleven complete JP1 SHA-256 checks are additionally required for acceptance.
These checks do not establish emulator gameplay or native Linux startup.
