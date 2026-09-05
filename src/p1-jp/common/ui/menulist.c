/* Persona 1 (JP) - menu list setup.
 *
 * Compiled into three overlays rather than called across the boundary, and
 * identical in all three because it touches no globals:
 *   DNG @ 0x80077518
 *   ADV @ 0x80067D50   S2D @ 0x80067540
 */
#include <types.h>
#include <persona/common/menulist.h>

extern int g_pad_held[];

/* Bit 0 is forced on so the first hold waits the long delay before it moves.
   Callers pass the rest: MenuListInit(g_menu + 0x20, 0, 0, count, 0x1a) sets
   up a wrapping vertical list with a click. */
void MenuListInit(MenuList *m, int cur, int lo, int hi, u_short flags)
{
    m->cur = cur;
    m->lo = lo;
    m->hi = hi;
    m->delay = 0;
    m->flags = flags | MENU_FIRST_REPEAT;
}

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
