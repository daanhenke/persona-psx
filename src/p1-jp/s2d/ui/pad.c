/* Persona 1 (JP) - controller poll.
 *
 * S2D's copy of src/p1-jp/common/pad.c; its work area sits 0x20000 higher,
 * which is the only difference.
 *   S2D @ 0x8007FDF4
 *
 * Three words in a row, and this is what settles which is which:
 *   0x800FC000  held    - the raw PadRead result, this frame
 *   0x800FC004  pressed - held & ~previous, i.e. the newly-pressed edge
 *   0x800FC008  previous frame's held
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

    prev = (int *)0x800FC008;
    held = (int *)0x800FC000;
    *prev = *held;
    now = PadRead(1);
    old = *prev;
    *(int *)0x800FC004 = (old & now) ^ now;
    *held = now;
}
