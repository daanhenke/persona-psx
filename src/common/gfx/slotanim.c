/* Persona 1 (JP) - the slot animation setter.
 *   DNG 0x80075A60   ADV 0x800661CC   S2D 0x80065D18
 *
 * A unit of its own; the flicker setter it shares its source with is in
 * slotflicker.c, ahead of a table neither is part of.
 */
#include <decomp/types.h>
#include <persona/common/slot.h>

/* S2D's table sits 0x20000 higher; see slot.c. */
#define g_slots ((Slot *)(0x800DC10C + WORK_BIAS))

/* Sets the seven animation fields in one go. The slot index arrives as a short
   here, not a u_char as everywhere else in this family. */
void SlotSetAnim(short slot, short unk18, short unk1A, u_char tpage_add,
                 u_char u_add, u_char v_add, u_short clut_x, u_short clut_y)
{
    g_slots[slot].unk18 = unk18;
    g_slots[slot].unk1A = unk1A;
    g_slots[slot].tpage_add = tpage_add;
    g_slots[slot].clut_x = clut_x;
    g_slots[slot].clut_y = clut_y;
    g_slots[slot].u_add = u_add;
    g_slots[slot].v_add = v_add;
}
