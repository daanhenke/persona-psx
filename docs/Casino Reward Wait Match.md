# Casino reward wait

`func_8008DC54` starts the reward palette animation if inactive, starts the
reward audio on timer zero, and advances to state 117 or 41 when the sequence
test returns zero and the timer is nonnegative. A 10000-unit win selects
state 117; otherwise it selects 41. It then sets the timer to -1.

The palette animation's active byte at `0x80099CB8` is named independently
of its enclosing 24-byte record. This preserves the original direct load
and separate animation-pointer construction. Explicit state assignments
also preserve the original register allocation.

Acceptance requires the 240-byte function to match in objdiff and all eleven
JP1 linked binary SHA-256 checks to pass using the pinned compiler. The code
retains the PS1 audio interfaces.
