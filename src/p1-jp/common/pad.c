/* Persona 1 (JP) - controller poll.
 *
 * Compiled into DNG and ADV rather than called across the boundary:
 *   DNG @ 0x8008F8E4
 *   ADV @ 0x8008B6BC
 * S2D's work area sits 0x20000 higher, so it keeps src/p1-jp/s2d/pad.c.
 *
 * Three words in a row, and this is what settles which is which:
 *   0x800DC000  held    - the raw PadRead result, this frame
 *   0x800DC004  pressed - held & ~previous, i.e. the newly-pressed edge
 *   0x800DC008  previous frame's held
 * MenuStepCursor tests the held word because it times its own auto-repeat;
 * InputCheckAcceptA/B test the edge so a press fires once.
 *
 * All three are reached as literals here rather than through their linker
 * symbols: the original builds two of them with lui/ori into saved registers
 * across the PadRead call, and normalize_asm folds the third to a literal too
 * because the same function builds the neighbouring addresses raw.
 */
#include <types.h>

extern u_long PadRead(int n);

/* Reading the old value into a local before storing the new one is what puts
   the AND operands in the original's order. Both spellings compute the same
   thing, but with the load left inline after `*held = now` gcc numbers the
   pseudos the other way round and emits `and v1, v0, v1`. */
void PadPoll(void)
{
    int *prev;
    int *held;
    int  now;
    int  old;

    prev = (int *)0x800DC008;
    held = (int *)0x800DC000;
    *prev = *held;
    now = PadRead(1);
    old = *prev;
    *(int *)0x800DC004 = (old & now) ^ now;
    *held = now;
}
