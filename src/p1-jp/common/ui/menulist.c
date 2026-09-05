/* Persona 1 (JP) - menu list setup.
 *
 * Compiled into three overlays rather than called across the boundary, and
 * identical in all three because it touches no globals:
 *   DNG @ 0x80077518
 *   ADV @ 0x80067D50   S2D @ 0x80067540
 */
#include <types.h>
#include <persona/common/menulist.h>

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
