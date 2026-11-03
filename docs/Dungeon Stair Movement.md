# Dungeon stair movement

`FieldStairs` (952 bytes) is matching C in the JP1 DNG overlay. It moves the party across the two stair tiles, marks the current and adjacent floors on the automap, coordinates the two view hops and resumes the movement music.

The conditional adjacent-floor marks calculate the byte column first, the room index second and the bit index last. Separate integer locals preserve GCC 2.6's original load order; a single combined expression emits the same operations in a different order.

Validation: all eleven complete JP1 binary SHA-256 checks pass in the pinned original-compiler environment. No compiler changes, padding, volatile accesses or assembly replacements were introduced.
