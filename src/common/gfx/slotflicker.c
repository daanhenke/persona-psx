/* Persona 1 (JP) - the slot flicker setter.
 *   DNG 0x800759A4   ADV 0x8006611C   S2D 0x80065C6C
 *
 * A unit of its own: a table none of the overlays has worked out sits between
 * this and the animation setter in slotanim.c, so they cannot be one object.
 * The record layout and every prototype come from slot.h.
 */
#include <decomp/types.h>
#include <persona/common/slot.h>

/* S2D's table sits 0x20000 higher; see slot.c. */
#define g_slots ((Slot *)(0x800DC10C + WORK_BIAS))

/* Turns the flicker bit on or off. While it is set the renderer ignores the
   slot's own brightness and takes it from a 32-entry table indexed by the
   phase counter at +0x31, which it advances every frame. */
void SlotSetFlicker(u_char slot, u_char on)
{
    Slot *s;

    s = g_slots + slot;
    if (on) {
        s->attr |= SLOT_ATTR_FLICKER;
    } else {
        s->attr &= ~SLOT_ATTR_FLICKER;
    }
}
