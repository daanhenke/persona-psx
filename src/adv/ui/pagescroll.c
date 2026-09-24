/* Persona 1 (JP) - stepping a number with the page buttons.  ADV only.
 *   0x8006AD9C PageScrollValue
 *
 * The page-back and page-forward buttons move a value down or up by `step`,
 * with a click, clamped to lo..hi. Each button has its own repeat timer,
 * timed like a menu list's: the first step waits 0x20 frames and every one
 * after it 2. Holding one button rearms the other, and letting go of both
 * rearms both.
 */
#include <decomp/types.h>
#include <persona/common/menulist.h>
#include <persona/common/pad.h>

/* The pad bits the two page buttons are mapped to. */
extern u_short g_key_page_back;
extern u_short g_key_page_fwd;

extern MenuList g_page_back_repeat;
extern MenuList g_page_fwd_repeat;

extern void SoundPlaySeq(u_short slot, u_short seq, short vab);

/* Returns 1 while either button is held, 0 when neither is. */
int PageScrollValue(short *value, short lo, short hi, short step)
{
    MenuList *r;
    short     d;

    if (g_key_page_back & g_pad_held[0]) {
        g_page_fwd_repeat.delay = 0;
        g_page_fwd_repeat.flags |= MENU_FIRST_REPEAT;
        d = -step;
        r = &g_page_back_repeat;
    } else if (g_key_page_fwd & g_pad_held[0]) {
        g_page_back_repeat.delay = 0;
        g_page_back_repeat.flags |= MENU_FIRST_REPEAT;
        d = step;
        r = &g_page_fwd_repeat;
    } else {
        goto none;
    }

    if (r->delay == 0) {
        *value += d;
        SoundPlaySeq(0x18, 1, 1);
        if (r->flags & MENU_FIRST_REPEAT) {
            r->delay = 0x20;
            r->flags ^= MENU_FIRST_REPEAT;
        } else {
            r->delay = 2;
        }
    } else {
        r->delay--;
    }

    if (*value < lo) {
        *value = lo;
    }
    if (*value > hi) {
        *value = hi;
    }
    return 1;

none:
    g_page_back_repeat.delay = 0;
    g_page_fwd_repeat.delay = 0;
    g_page_back_repeat.flags |= MENU_FIRST_REPEAT;
    g_page_fwd_repeat.flags |= MENU_FIRST_REPEAT;
    return 0;
}
