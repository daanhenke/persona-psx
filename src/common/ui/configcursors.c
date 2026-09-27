/* Persona 1 (JP) - the config page's markers.
 *
 * Compiled into two overlays rather than called across the boundary:
 *   ADV 0x8008B914   DNG 0x8008FB3C   ConfigPlaceCursors
 *
 * The row cursor goes to its row (13, the exit, sits apart at the foot).
 * Rows 0-7 and the rest are two pages of settings: the page not shown loses
 * its page marker, and five markers stand on the value each setting holds,
 * one row apiece. The first page's fifth marker only exists there.
 */
#include <decomp/types.h>
#include <persona/common/menuctx.h>
#include <persona/common/slot.h>

#define g_slots ((Slot *)(0x800DC10C + WORK_BIAS))

extern Slot *g_slot_cur;

#define ROW_CURSOR   0
#define PAGE_MARK0   1   /* and 2 */
#define VALUE_MARK   16  /* five, one a row */

#define EXIT_ROW     13
#define EXIT_Y       0xCC
#define ROW_H        24
#define ROW_Y0       0x18

/* Each marker's x before its y, and the fifth's attr after both: the order
   the image's schedule keeps. */
void ConfigPlaceCursors(void)
{
    MenuCtx *m;

    g_slot_cur = &g_slots[ROW_CURSOR];
    if (g_menu->item_col.cur != EXIT_ROW) {
        g_slot_cur->y = (g_menu->item_col.cur & 7) * ROW_H + ROW_Y0;
    } else {
        g_slot_cur->y = EXIT_Y;
    }
    m = g_menu;
    g_slot_cur = &g_slots[PAGE_MARK0];
    if (m->item_col.cur < 8) {
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
        g_slot_cur = &g_slots[PAGE_MARK0 + 1];
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
        g_slot_cur = &g_slots[VALUE_MARK + 0];
        g_slot_cur->x = m->unk280.cur * 32 + 0xA0;
        g_slot_cur->y = 0x18;
        g_slot_cur = &g_slots[VALUE_MARK + 1];
        g_slot_cur->x = m->unk290.cur * 40 + 0xA0;
        g_slot_cur->y = 0x30;
        g_slot_cur = &g_slots[VALUE_MARK + 2];
        g_slot_cur->x = m->unk2A0.cur * 48 + 0xA0;
        g_slot_cur->y = 0x48;
        g_slot_cur = &g_slots[VALUE_MARK + 3];
        g_slot_cur->x = m->unk2B0.cur * 40 + 0xA0;
        g_slot_cur->y = 0x78;
        g_slot_cur = &g_slots[VALUE_MARK + 4];
        g_slot_cur->x = m->unk2C0.cur * 48 + 0xA0;
        g_slot_cur->y = 0x90;
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    } else {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
        g_slot_cur = &g_slots[PAGE_MARK0 + 1];
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
        g_slot_cur = &g_slots[VALUE_MARK + 0];
        g_slot_cur->x = m->unk2D0.cur * 32 + 0xB0;
        g_slot_cur->y = 0x30;
        g_slot_cur = &g_slots[VALUE_MARK + 1];
        g_slot_cur->x = m->unk2E0.cur * 40 + 0xB0;
        g_slot_cur->y = 0x48;
        g_slot_cur = &g_slots[VALUE_MARK + 2];
        g_slot_cur->x = m->unk2F0.cur * 32 + 0xB0;
        g_slot_cur->y = 0x60;
        g_slot_cur = &g_slots[VALUE_MARK + 3];
        g_slot_cur->x = m->unk300.cur * 32 + 0xB0;
        g_slot_cur->y = 0x78;
        g_slot_cur = &g_slots[VALUE_MARK + 4];
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
    }
}
