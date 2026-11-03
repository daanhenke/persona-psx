# Casino round return and lamp synchronization

These JP1 C implementations reproduce the original casino code. They retain
PS1 SDK calls and overlay state; they do not add a native Linux backend.

| Function | Source | Code bytes | Behavior |
| --- | --- | ---: | --- |
| func_8008D4C4 | game/roundreturn.c | 348 | Returns three layouts, clears payout digits and input cursors, then enables the next round's sprites and state. |
| func_80090C58 | game/lampsync.c | 380 | Animates three objects when their requested lamp bit differs from its remembered bit, then records the current request. |

Round return queues the three opposing horizontal tweens at timer zero and
clears all eight payout digits in reverse order. It queues the frame tween
at timer 24. At timer 32 or later it enables sprites 218–229, resets the timer
to -1, selects state 0x26, and sets lamp bit 0. The existing bitfield assignment
`CASINO_LAMPS.b0 = 1` preserves the original load/store scheduling; a raw
`g_casino_lamps |= 1` produced the same length but ten differing instruction
words in this routine.

Lamp synchronization compares bit pairs 4/5, 0/1 and 2/3 in that order.
The first object rotates by +2048 or -2048 according to bit 4; the other two
rotate by +2048 on either edge. The remembered bits are assigned from the
requested bits after each animation call, preserving the original state
reloads. Existing CasinoFlags and typed CasinoObj pointer aliases recover
the exact 380-byte implementation without changing shared headers.

Validation uses the project's original GCC 2.6.0 and pinned offline build
environment: both functions score 100% in objdiff, and all eleven linked JP1
executable/overlay SHA-256 checks pass after the final report and rebuild.
No emulator gameplay test was performed. Two other candidates from this
batch remain assembly and are saved separately as unverified hard cases.
