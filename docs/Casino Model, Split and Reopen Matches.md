# Casino model, split and reopening matches

These JP1 functions replace assembly with C while retaining the original
PS1 SDK and overlay interfaces.

| Function | Source | Code bytes | Behavior |
| --- | --- | ---: | --- |
| func_80092DEC | game/modelxformdraw.c | 296 | Applies a supplied rotation, translation and scale, textures the model, installs its transform and draws it. |
| func_8007AFC0 | game/splithandsetup.c | 364 | Initializes two split blackjack hands, charges the additional bet and animates their labels. |
| func_8008EB9C | game/gridreopen.c | 612 | Restores eleven grid entries, queues entry animations, resets state and rebuilds four layout groups. |

The model helper takes `CasinoXform` **by value**, after the model pointer.
GCC spills the first three argument words and reads the remaining words from
the caller's stack area. This accounts for the halfword reads at adjacent
stack offsets: rotation X and Y occupy separate halves of one argument word.
Raw pseudocode incorrectly presented both as the same scalar argument.
The existing type recovers the full 296-byte routine without changing shared
headers. Rotation padding and VECTOR padding are not written.

Split setup initializes both hand counts to one, clears their totals,
marks their classifications as plain hands and copies the original hand's
second card to the split hand's first slot. It records and charges the
additional bet. At timer eight or later it queues the two label tweens,
clears their label counts and selects play step 0x2D.

Reopening preserves the original sprite enable/disable ranges and state
store order. The word at D_800AAE98 is cleared with a halfword store; its
neighboring flags are byte stores. Existing lamp bitfields set bit 0 and
clear bit 2 before the setup callback. func_80093178 takes no incoming
arguments: its apparent argument in raw pseudocode was an incidental live
register from the preceding lamp update.

Validation: all three functions score 100% in objdiff. All eleven linked JP1
executable/overlay SHA-256 checks pass after the final report and normal
rebuild, using the pinned offline environment and original GCC 2.6.0.
No emulator gameplay test or native Linux runtime test was performed.
