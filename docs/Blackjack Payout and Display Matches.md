# Blackjack payout, card placement, actions, and display

Four JP1 casino assembly fallbacks are replaced with matching C (3,244 bytes):

| Function | Bytes | Behavior |
| --- | ---: | --- |
| `func_8007C514` | 976 | Presents a hand result, handles confirmation, pays winnings, and advances the hand state. |
| `func_8007E878` | 680 | Takes a card from the shoe, positions and draws it, and updates the shoe and hand counts. |
| `func_80080F1C` | 776 | Refreshes monetary displays, increments the jackpot, and updates lamp animations. |
| `func_80081934` | 812 | Handles blackjack action input and double-bet transitions, then updates labels and animations. |

`g_bj_double_palette_on` names the existing active byte at `0x800999B8`; it allocates no storage. Lamp bitfields retain the original compiler's read/modify/write behavior. Card transform fields preserve the original 40-byte record layout.

All four functions score 100% in objdiff. Isolated comparisons with original linked symbol addresses also report no differences. The pinned offline original-compiler build passes all eleven JP1 SHA-256 checks after generation and reporting. No emulator gameplay testing was performed.
