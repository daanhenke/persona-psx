# Map, time, and casino wrappers

This batch replaces thirteen JP1 functions with matching C (2,196 bytes).

| Area | Functions | Bytes |
| --- | --- | ---: |
| S2D current-cell flag | `func_8008BB84` | 84 |
| S2D play-time update | `func_80097288` | 232 |
| S2D neighboring-cell flag | `func_80094F2C` | 112 |
| S2D restore coordinates and draw four frames | `func_8009BC4C` | 112 |
| Blackjack state decisions and display delay | `func_8007A27C`, `func_8007B208` | 416 |
| Card-pair classification | `func_80081DC0` | 448 |
| Reel-group readiness | `func_80085A7C` | 248 |
| Bet affordability and display calls | `func_8007BA14` | 76 |
| Sequence restart and state changes | `D_80081570`, `D_800816E4` | 208 |
| Start two animation scripts | `func_8007F578` | 128 |
| Cursor display updates | `func_8008D620` | 132 |

The map queries retain the original signed coordinates and flag masks. The
time updater retains byte storage, one-step carry, and the original cap
behavior; it does not normalize arbitrarily large input deltas. The card
classifier preserves the four suit-specific special pairs and rank tests.
The neighboring-cell function accepts both its original position and facing
arguments before passing an output coordinate pair to `func_80094E68`.

All 11 JP1 executable/overlay SHA-256 checks pass with the C units enabled.
No emulator gameplay test was performed. Unresolved scheduling experiments
remain local and are excluded from this contribution.
