# Casino rotation response and payout exits

These JP1 functions retain the PS1 SDK and original overlay state.

| Function | Source | Code bytes | Behavior |
| --- | --- | ---: | --- |
| func_80091D0C | game/rotationresponse.c | 292 | Classifies rotation bands, restarts a sound on a qualifying band change and damps velocity on one band. |
| func_8007A654 | game/losingbetexit.c | 296 | Queues a conditional palette/animation and sound, then clears the bet and advances the blackjack play step. |
| func_8007A8B4 | game/jackpotpayexit.c | 324 | Steps the payout on alternate frames, caps money, conditionally resets the jackpot and animates the payout display out. |

Rotation response converts the double at offset 0x50 to a signed integer,
takes signed remainder modulo 360, and then performs unsigned range tests.
Angles 0–89 set band zero; 180–269 set band one; other values preserve the
existing band. A changed band triggers sequence 9 only when signed mode at
0xE0 is positive. Band one multiplies the double at 0xB8 by exactly 0.777
(IEEE-754 bits 0x3FE8DD2F1A9FBE77). The remembered band is reloaded after the
sound/math operations. The local structure describes only fields read or
written by this routine; shared bounce types are unchanged.

The bet exit conditionally queues its entry animation at timer zero and
sound at timer 28, unless the first hand's classification is 2. At timer 73
or later it queues the exit animation, selects play step 0x30, clears the
bet and resets the timer to -1.

Payout exit calls CasinoPayStep on even frames. Money above 99,999,998 is
set to 99,999,999 and the pending win is cleared. Once the win is zero,
classification zero together with a bet of at least ten writes jackpot
300,000; both display objects animate out and play step advances to 0x30.

All three first typed drafts reproduce the original instructions and score
100% in objdiff. All eleven JP1 linked binary SHA-256 checks pass after the
full report and final normal rebuild in the pinned offline environment.
No emulator gameplay or native Linux runtime test was performed. The
unresolved grid palette candidate remains assembly and is saved separately.
