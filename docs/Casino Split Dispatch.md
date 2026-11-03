# Blackjack split-hand dispatch

`func_8007ADD8` dispatches fourteen values of `g_bj_play_step` to their original
handlers. Other byte values do nothing. Exactly one handler runs, even if that
handler changes the state. State 0x74 passes the state byte address to the dealer
substate dispatcher; that routine ignores the argument and reads D_800A8C7C.
Its shared declaration now preserves this calling convention across both callers.

The original 488-byte routine uses a sparse decision tree. A flat switch produces
a jump table with GCC 2.6, so the C spells out the decision tree and joins at named
handler labels. The range shortcut for 0x2D is reached only after excluding 0x2E.
There is no inline assembly, instruction padding, or post-compilation rewriting.

Validation includes all 256 input byte states, unknown-state no-ops, one-handler
execution after callback mutation, and the dealer pointer argument. Full JP1
binary checks remain the acceptance gate. No emulator gameplay claim is made.

## Second-hand fades

`func_8007F668` stores the second hand's display mode and fades its cards and
labels. Mode 1 selects brightness128, mode2 selects64, and other modes select0.
Additional cards are faded only when the second hand has at least three cards.
The count and natural-hand flags are read after earlier fade calls, as in the image.

`CasinoFade` now has shared full-width `int` range arguments. It narrows those
values only when packing `CasinoFadeArg`; colours and frame count retain their
original widths. Both the existing wrapper and its callers are checked against
the original binaries. Prematurely declaring the range as `short` inserted two
unwanted sign extensions in this routine. No incompatible function declarations
are used to obtain the match.
