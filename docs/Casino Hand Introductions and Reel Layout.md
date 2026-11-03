# Casino hand introductions and reel layout

This batch replaces ten assembly fallbacks with matching C:

| Function | Bytes | Behavior |
| --- | ---: | --- |
| `func_8007A39C` | 696 | Calculates blackjack awards, animates the result and waits for acknowledgement. |
| `func_8007D714` | 736 | Animates dealer draws and updates the dealer total. |
| `func_80088E64` | 752 | Handles grid selection confirmation and saves its chosen layout. |
| `func_80081224` | 844 | Handles blackjack split, double, hit and stand choices and their label animations. |
| `func_8007B288` | 596 | Introduces the initial blackjack hand, starts its animations and updates labels. |
| `func_8007B4DC` | 616 | Introduces the split hand with its own card, label and animation offsets. |
| `func_8007F960` | 632 | Resolves blackjack hand status, terminal conditions and result presentation. |
| `func_8008FF1C` | 640 | Dispatches dice-game repeated input and clears transient flags. |
| `func_8008749C` | 608 | Expands 21 source entries into twelve five-entry reel lines with signed `-1` sentinels. |
| `func_8008E808` | 668 | Closes the dice payout presentation, restores palettes and advances the state. |

The payout state copies the existing shared 44-byte zero mask before marking the selected result. The reel layout uses signed bytes so the original compiler materializes the sentinel as `-1`. Timed actions retain the original independent conditions, including their order after calls that can change state.

All ten functions pass 100% objdiff code matching and all 11 original JP1 executable/overlay SHA-256 checks in the pinned build environment. This batch adds 6,788 matching bytes. These changes retain PS1 interfaces; no native Linux runtime or emulator gameplay testing is included.

The blackjack-result and grid-selection palette active bytes are named at their existing addresses, following the reward-palette convention. This preserves separate state-byte loads and animation-record addresses without allocating new storage.

The dealer card-count subtraction retains the original operand order. The isolated comparison masks relocation addends, so the linked SHA-256 checks are required to catch address-order differences even when objdiff reports 100%.
