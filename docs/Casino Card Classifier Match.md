# Casino card classifier

`func_80082794` returns one for card IDs 0–12 and 26–38, and zero
for all other byte values. Subtractions are explicitly converted back to
unsigned bytes before range comparison, preserving the original wraparound.

The final comparison against the 39–51 range is retained in a local, then
cleared when true. Although simplifying that expression preserves the return
value, the original compiler emits different code after simplification.
This form matches all 88 bytes and has a defined return for every input.

Acceptance requires a 100% objdiff match and all eleven JP1 linked binary
SHA-256 checks using the pinned build environment.
