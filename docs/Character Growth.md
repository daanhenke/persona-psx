# Character growth

`CharGrow` (392 bytes) and `CharRegrow` (720 bytes) are matching C in the JP1 ADV overlay. They update HP, SP and five base stats from the original growth tables. Regrowth restores the template values first, applies the growth loop, caps HP/SP at 999 and recalculates the derived stats.

The compiler requires a separate growth-table base assignment inside the loop, after the HP/SP updates. GCC 2.6 then moves that base outside the loop while retaining the original magic-stat loads. Initializing the base before the loop produces a different instruction sequence.

`CharRegrow` keeps its existing character pointer through the loop. Calling the helper that reconstructs the pointer produces an extra register copy; the explicit loop preserves the original instructions.

Validation: the pinned original-compiler build reproduces all eleven JP1 binary SHA-256 values. No compiler flags, padding, inline assembly or volatile accesses were added. This verifies the PS1 build; native Linux execution remains a separate porting task.
