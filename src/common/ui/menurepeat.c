/* Persona 1 (JP) - re-arming a menu list's repeat delay.
 *   ADV 0x80067EE8
 *
 * A unit of its own: the cursor step it belongs beside is not worked out, and
 * the list setup that shares its source is well ahead of it, in menulist.c.
 */
#include <decomp/types.h>
#include <persona/common/menulist.h>

extern int g_pad_held[];

/* Clears the repeat delay and re-arms the first-repeat flag when none of the
   list's mapped directions is held - the same test MenuStepCursor makes at the
   end of its run, for callers that only need the reset. */
void MenuResetRepeat(MenuList *m)
{
    u_int   inc;
    u_int   dec;
    u_short f;

    inc = 0;
    dec = 0;
    f = m->flags;
    if (f & 4) {
        inc = 0x4000;
        dec = 0x1000;
    }
    if (f & 8) {
        inc |= 0x2000;
        dec |= 0x8000;
    }
    if ((inc & g_pad_held[0]) == 0 && (dec & g_pad_held[0]) == 0) {
        m->delay = 0;
        m->flags |= MENU_FIRST_REPEAT;
    }
}
