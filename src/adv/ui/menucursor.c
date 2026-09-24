/* Persona 1 (JP) - menu cursor movement.  ADV @ 0x80067D70.
 *
 * The routine DNG and S2D carry in their own menucursor.c, with the step click
 * written out in place: where they call SoundPlaySeq, ADV's copy opens and
 * plays the sequence itself - the same four library calls against the same
 * hardcoded tables sound.c reaches.
 */
#include <decomp/types.h>
#include <libsnd.h>
#include <persona/common/menulist.h>
#include <persona/common/pad.h>

/* All reached by hardcoded address, as in sound.c. */
#define g_seq_handle ((short *)0x801F537C)   /* one open handle per slot */
#define g_vab_id     ((short *)0x801F535C)   /* VAB ids, by bank         */
#define g_seq_offset ((u_long *)0x80118020)  /* offsets into the blob    */
#define SEQ_DATA     0x80118000

#define CLICK_SLOT 0x18

/* SoundPlaySeq, which this unit has in place rather than calling. */
static inline void PlayClick(u_short slot, u_short seq, short vab)
{
    short  *handle;
    u_long *offset;
    short   h;

    offset = &g_seq_offset[seq];
    handle = &g_seq_handle[slot];
    SsSetNck(*handle);
    h = SsSeqOpen((u_long *)(*offset + SEQ_DATA), g_vab_id[vab]);
    *handle = h;
    SsSeqSetVol(h, 0x7F, 0x7F);
    SsSeqPlay(*handle, 1, 1);
}

/* Steps one menu list from the d-pad, once a frame; see DNG's menucursor.c.
   Returns 1 while a direction is held, whether or not the cursor actually
   moved, and 0 when none is. */
int MenuStepCursor(MenuList *m)
{
    u_int inc, dec;
    int   cur, limit;

    inc = 0;
    dec = 0;
    if (m->flags & MENU_DOWN_IS_NEXT) {
        inc = 0x4000;
        dec = 0x1000;
    } else if (m->flags & MENU_UP_IS_NEXT) {
        inc = 0x1000;
        dec = 0x4000;
    }
    if (m->flags & MENU_RIGHT_IS_NEXT) {
        inc |= 0x2000;
        dec |= 0x8000;
    } else if (m->flags & MENU_LEFT_IS_NEXT) {
        inc |= 0x8000;
        dec |= 0x2000;
    }

    if ((inc & g_pad_held[0]) != 0 || (dec & g_pad_held[0]) != 0) {
        if (m->delay == 0) {
            if (m->flags & MENU_FIRST_REPEAT) {
                m->delay = 0x20;
                m->flags ^= MENU_FIRST_REPEAT;
            } else {
                m->delay = 2;
            }

            if (inc & g_pad_held[0]) {
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
            } else if (dec & g_pad_held[0]) {
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
            if (m->flags & MENU_CLICK_A) {
                PlayClick(CLICK_SLOT, 1, 1);
            } else if (m->flags & MENU_CLICK_B) {
                PlayClick(CLICK_SLOT, 3, 1);
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
