/* Persona 1 (JP) - the equipment screen's cursor.
 *
 * Compiled into two overlays rather than called across the boundary:
 *   ADV 0x800933E4   S2D 0x8007E1C8
 *
 * Five slots stand ready and only one is ever on screen: the weapon has its
 * own, and the other six groups share a second that moves down the list a row
 * at a time. Every screen that previews a change hides all five and shows the
 * one the selected group needs, so the cursor follows the value list rather
 * than being moved when the list is stepped.
 */
#include <types.h>
#include <persona/common/menuctx.h>
#include <persona/common/slot.h>

#define g_slots ((Slot *)(0x800DC10C + WORK_BIAS))

/* The five the screen owns: 3 walks the rows and 5 is the weapon's. */
#define CURSOR_FIRST 1
#define CURSOR_LAST  5
#define CURSOR_ROWS  3
#define CURSOR_TOP   5

/* Where the row cursor sits, and how far apart the rows are. */
#define CURSOR_Z 0x42
#define CURSOR_X 0x28
#define CURSOR_Y 0x84
#define CURSOR_PITCH 12

/* The groups the row cursor covers. */
#define CURSOR_ROW_MAX 8

extern Slot *g_slot_cur;

extern void SlotSetPos(u_char slot, int attr, short x, short y);

void EquipPlaceCursor(void)
{
    g_slot_cur = &g_slots[CURSOR_FIRST];
    g_slots[CURSOR_FIRST].attr |= SLOT_ATTR_HIDE;
    g_slot_cur = &g_slots[2];
    g_slots[2].attr |= SLOT_ATTR_HIDE;
    g_slot_cur = &g_slots[CURSOR_ROWS];
    g_slots[CURSOR_ROWS].attr |= SLOT_ATTR_HIDE;
    g_slot_cur = &g_slots[4];
    g_slots[4].attr |= SLOT_ATTR_HIDE;
    g_slot_cur = &g_slots[CURSOR_LAST];
    g_slots[CURSOR_TOP].attr |= SLOT_ATTR_HIDE;

    if (g_menu->list[1].cur == 0) {
        g_slots[CURSOR_TOP].attr &= ~SLOT_ATTR_HIDE;
    } else if (g_menu->list[1].cur >= 0 &&
               g_menu->list[1].cur < CURSOR_ROW_MAX) {
        g_slots[CURSOR_ROWS].attr &= ~SLOT_ATTR_HIDE;
        g_slot_cur = &g_slots[CURSOR_ROWS];
        SlotSetPos(CURSOR_ROWS, CURSOR_Z, CURSOR_X,
                   g_menu->list[1].cur * CURSOR_PITCH + CURSOR_Y);
    }
}
