# Reward display and selection

Three routines now use matching C and a shared reward-work-area declaration:

- `func_800915FC` (500 bytes) blinks the selected dice reward. Palette phases
  occur every eight frames; digit visibility changes every thirty-two frames.
  At the bright phase it shows one through four digits, depending on the
  unsigned reward amount. Values at least 10,000 show neither digit branch.
- `func_8008DF68` (344 bytes) slides the result panels into place, then reveals
  the selected reward at frame 16 and hides unused leading digits. It advances
  to step 49 and resets the timer.
- `func_8008E0C0` (1,232 bytes) handles the choice to continue or leave the dice
  reward round, including panel motion, audio handles, mutually exclusive
  input directions and restoring the selected reward display.

The shared record has thirteen reward values, the intervening 84 bytes of
workspace, and three dice bytes. Dice sums 2–12 are the valid table indices.
The explicit unsigned shift followed by signed narrowing retains the original
index-conversion sequence before optimization. This prevents the compiler from
reusing a scaled index where the original reloads/recomputes it, without
assembly padding or edits to compiler output. Whole-record addressing also
avoids constructing pointers before a separately declared dice array.

The linked comparison caught an older candidate passing sequence slot zero
instead of slot one at frame 16; the accepted version uses slot one. Checks
must resolve relocation addends as well as compare instruction shapes.

`tests/native/casino_rewardflow.c` executes these C bodies after removing only
PS1 linker aliases. Its 18,644 cases check signed timer boundaries, valid dice
sums, unsigned amount thresholds, input combinations, round modes, audio slots,
call order, neighboring bytes, and callbacks changing timers or selection.
The selection fixture records its blink-helper call independently so that
selection sequencing and blink behavior can be checked separately. It passes
normally and with ASan/UBSan; leak checking is disabled in the sandbox.
Acceptance additionally requires all eleven full JP1 binary SHA-256 checks.
These fixtures do not establish emulator gameplay or native Linux game startup.
