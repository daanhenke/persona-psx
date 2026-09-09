/* Persona 1 (JP) - the per-slot semi-transparency switch.  ADV only.
 *   0x800661D0 SlotSetSemiTrans
 *
 * A unit of its own between the two halves of the shared slot family: the
 * flicker setter is ahead of it and the animation setter behind, and neither
 * is ADV's. The fades are in slotfade.c.
 */
#include <decomp/types.h>
#include <persona/common/slot.h>

/* 0x44-byte records at 0x800DC10C, indexed by a u8 slot. Reached by hardcoded
   address rather than through a linker symbol. */
#define g_slots ((Slot *)0x800DC10C)

/* Turns 50% semi-transparency on or off for the slot. The renderer forwards
   the bit straight into GsSPRITE.attribute, where bit 30 has the same
   meaning. */
void SlotSetSemiTrans(u_char slot, u_char on)
{
    Slot *s;

    s = g_slots + slot;
    if (on) {
        s->attr |= SLOT_ATTR_SEMITRANS;
    } else {
        s->attr &= ~SLOT_ATTR_SEMITRANS;
    }
}
