/* Persona 1 (JP) - menu cursor movement.  S2D @ 0x80067560.
 *
 * DNG has the same routine at 0x80077538; S2D's work area sits 0x20000 higher,
 * which is the only difference. ADV's copy at 0x80067D70 is a different
 * function: it has the click sound inlined rather than calling SoundPlaySeq,
 * which makes it 28 instructions longer.
 */
#include <types.h>
#include <persona/common/menulist.h>

/* The pad word this tests is the held state, not an edge - it has to be, since
   the auto-repeat below is timed against it. */
extern int  g_pad_held_s2d[];
extern void SoundPlaySeq(u_short slot, u_short seq, short vab);

/* Steps one menu list from the d-pad, once a frame. `inc` and `dec` collect
   the pad bits that move the cursor forwards and backwards - 0x1000 Up,
   0x2000 Right, 0x4000 Down, 0x8000 Left - from the MENU_*_IS_NEXT flags, so a
   list can be driven vertically, horizontally or both at once. The first move
   of a hold waits 0x20 frames and every repeat after it 2, and the cursor
   either clamps at lo/hi or wraps to the far end depending on MENU_WRAP.

   Returns 1 while a direction is held, whether or not the cursor actually
   moved, and 0 when none is. */
int MenuStepCursor(MenuList *m)
{
    u_int inc, dec;
    int   cur, limit;

    inc = 0;
    dec = 0;
    if (m->flags & 4) {
        inc = 0x4000;
        dec = 0x1000;
    } else if (m->flags & 0x40) {
        inc = 0x1000;
        dec = 0x4000;
    }
    if (m->flags & 8) {
        inc |= 0x2000;
        dec |= 0x8000;
    } else if (m->flags & 0x80) {
        inc |= 0x8000;
        dec |= 0x2000;
    }

    if ((inc & g_pad_held_s2d[0]) != 0 || (dec & g_pad_held_s2d[0]) != 0) {
        if (m->delay == 0) {
            if (m->flags & MENU_FIRST_REPEAT) {
                m->delay = 0x20;
                m->flags ^= MENU_FIRST_REPEAT;
            } else {
                m->delay = 2;
            }

            if (inc & g_pad_held_s2d[0]) {
                cur = m->cur;
                limit = m->hi;
                m->cur = cur + 1;
                if (cur + 1 > limit) {
                    if (m->flags & MENU_WRAP) {
                        m->cur = m->lo;
                        goto moved;
                    }
                    m->cur = limit;
                }
            } else if (dec & g_pad_held_s2d[0]) {
                cur = m->cur;
                limit = m->lo;
                m->cur = cur - 1;
                if (limit > cur - 1) {
                    if (m->flags & MENU_WRAP) {
                        m->cur = m->hi;
                        goto moved;
                    }
                    m->cur = limit;
                }
            }
        moved:
            /* The step click, into sound slot 0x18: MENU_CLICK_A picks
               sequence 1, MENU_CLICK_B sequence 3. A list with neither flag
               moves silently. */
            if (m->flags & MENU_CLICK_A) {
                SoundPlaySeq(0x18, 1, 1);
            } else if (m->flags & MENU_CLICK_B) {
                SoundPlaySeq(0x18, 3, 1);
            }
            return 1;
        }
        m->delay--;
        return 0;
    }
    m->delay = 0;
    m->flags |= MENU_FIRST_REPEAT;
    return 0;
}
