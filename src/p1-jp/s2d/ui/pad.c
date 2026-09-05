/* Persona 1 (JP) - controller poll.
 *
 * S2D's copy of src/p1-jp/common/ui/pad.c; its work area sits 0x20000 higher,
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
 * All three are reached by hardcoded address rather than through their linker
 * symbols.
 */
#include <types.h>

extern u_long PadRead(int n);

/* Reads controller 1 once a frame: last frame's held word is rolled into the
   previous slot, the new one takes its place, and `(old & now) ^ now` -
   i.e. now & ~old - leaves the newly-pressed edge in the middle word. */
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
