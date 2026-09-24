/* Persona 1 (JP) - the map screen's scroll arrows and cursor.  ADV only.
 *   ADV 0x80095C84
 *
 * Four arrow sprites, a pair for each of the screen's two lists: each is
 * hidden and then shown again only while its list can still move that way -
 * the cursor off its low end for the first of the pair, off its high end for
 * the second. The map cursor goes where the view sits on the map, sixteen
 * pixels a tile from the scrolled origin, and is hidden once it would fall
 * outside the map's window.
 */
#include <decomp/types.h>
#include <persona/common/menuctx.h>
#include <persona/common/slot.h>

#define g_slots ((Slot *)0x800DC10C)

#define ARROW_SLOT  32   /* four: lo/hi of list 0, then of list 1 */
#define CURSOR_SLOT 40
#define CURSOR_Z    0x10

/* The window the cursor is allowed in. */
#define WIN_X0 0x28
#define WIN_X1 0x119
#define WIN_Y0 0x40
#define WIN_Y1 0xC9

extern Slot *g_slot_cur;

extern short g_map_view_x;
extern short g_map_view_y;
extern short g_map_scroll_x;
extern short g_map_scroll_y;

void MapDrawMarkers(void)
{
    MenuCtx *m;
    int      x;
    int      y;
    int      ox;
    int      oy;

    g_slot_cur = &g_slots[ARROW_SLOT + 0];
    g_slot_cur->attr |= SLOT_ATTR_HIDE;
    g_slot_cur = &g_slots[ARROW_SLOT + 1];
    g_slot_cur->attr |= SLOT_ATTR_HIDE;
    g_slot_cur = &g_slots[ARROW_SLOT + 2];
    g_slot_cur->attr |= SLOT_ATTR_HIDE;
    g_slot_cur = &g_slots[ARROW_SLOT + 3];
    g_slot_cur->attr |= SLOT_ATTR_HIDE;

    m = g_menu;
    g_slot_cur = &g_slots[ARROW_SLOT + 0];
    if (m->list[0].cur != m->list[0].lo) {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }
    g_slot_cur = &g_slots[ARROW_SLOT + 1];
    if (m->list[0].cur != m->list[0].hi) {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }
    g_slot_cur = &g_slots[ARROW_SLOT + 2];
    if (m->list[1].cur != m->list[1].lo) {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }
    g_slot_cur = &g_slots[ARROW_SLOT + 3];
    if (m->list[1].cur != m->list[1].hi) {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }

    x = g_map_view_x * 16;
    ox = g_map_scroll_x - 0xA4;
    x -= ox;
    y = g_map_view_y * 16;
    oy = g_map_scroll_y - 0x80;
    y -= oy;
    SlotSetPos(CURSOR_SLOT, CURSOR_Z, x, y);
    g_slot_cur = &g_slots[CURSOR_SLOT];
    if (y < WIN_Y0 || x < WIN_X0 || y >= WIN_Y1 || x >= WIN_X1) {
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
    } else {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }
}
