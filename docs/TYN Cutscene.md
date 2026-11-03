# TYN event cutscene

`AdvTynCutscene` at ADV `0x80088C70` replaces the 6,796-byte assembly fallback with C. It loads scene pictures and audio, animates the figures and sparks, presents both messages, then restores the normal ADV room.

The spark helpers narrow each computed coordinate to a signed short before returning its sign-extended value. Their divisors remain literal 50 and 30: GCC 2.6 chooses constant-division multiplication before inlining, so passing a divisor argument produces a different sequence. The unit uses the existing full-word `SlotInit` coordinate declaration and passes the fade box's negative y coordinate as a full word.

Other matching details are the chained ordering-table length assignment, initialization of the rise counter before its bounds, an explicit rise-distance temporary, and storing the character-name byte at message offset `0xE` before copying it to `0x13`. Event arguments retain their JP1 fixed address `0x801F1BA0`. The box initializer and filenames reference their existing read-only data symbols, avoiding duplicate data that would shift the overlay.

Validation uses the pinned original compiler and all eleven complete JP1 binary SHA-256 checks. No instruction encoding, added padding, volatile coercion, or compiler-flag override is used.

The original cutscene passes an uninitialized local to `TynSyncPlanes`. That incoming stack behavior is preserved for matching and still needs investigation for a native port. This match does not establish native Linux cutscene playback.
