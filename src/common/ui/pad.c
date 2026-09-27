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
 * held and previous are reached by hardcoded address; the edge is stored
 * through its symbol.
 */
#include <decomp/types.h>

extern u_long PadRead(int n);
extern int    g_pad_pressed[];

/* Reads controller 1 once a frame: last frame's held word is rolled into the
   previous slot, the new one takes its place, and `(old & now) ^ now` -
   i.e. now & ~old - leaves the newly-pressed edge in the middle word. */
void PadPoll(void)
{
    int *prev;
    int *held;
    int  now;
    int  old;

    prev = (int *)(0x800DC008 + WORK_BIAS);
    held = (int *)(0x800DC000 + WORK_BIAS);
    *prev = *held;
    now = PadRead(1);
    old = *prev;
    g_pad_pressed[0] = (old & now) ^ now;
    *held = now;
}
