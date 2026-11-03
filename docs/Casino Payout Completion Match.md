# Casino payout completion

`func_8008E6F0` clears eleven display entries over the first eleven timer
values, initializes win digits, advances payment on alternating frames, and
caps money at 99,999,999. Once payment finishes and the timer reaches eleven,
it advances to state 36 and resets the timer to -1.

The first timer range check is unsigned; the final threshold is signed.
The reconstruction preserves both, along with independent reads after calls
that can change shared state. Acceptance requires 100% objdiff for all
280 bytes and all eleven JP1 linked binary SHA-256 checks.

The batch also recovers `func_8008D120` (304 bytes), which compares each
three-by-three grid against a reference grid and queues CLUT 0x7EA4 for
matching cells. An explicit row base, advanced before the row index,
preserves the original loop scheduling and placement of the sprite offset.
