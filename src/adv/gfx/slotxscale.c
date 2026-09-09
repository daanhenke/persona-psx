/* Persona 1 (JP) - the once-a-room x-scale pass over the first eight slots.
 *   ADV 0x80082650
 *
 * A unit of its own, far from the rest of the source it shares; see slot.c.
 */
#include <decomp/types.h>
#include <persona/common/slot.h>

/* 0x44-byte records at 0x800DC10C, indexed by a u8 slot. Reached by hardcoded
   address rather than through a linker symbol. */
#define g_slots ((Slot *)0x800DC10C)

/* Gives the first eight slots an x scale of one unit under full when their
   SLOT_ATTR_XSCALE bit is set - enough to change how the GPU rounds the
   sprite - and exactly 1.0 when it is not. The scene setup runs it once a
   room. g_slot_cur is pointed at each slot on the way past. */
extern Slot *g_slot_cur;

void SlotsApplyXScale(void)
{
    u_char i;

    for (i = 0; i < 8; i++) {
        g_slot_cur = &g_slots[i];
        if (g_slots[i].attr & SLOT_ATTR_XSCALE) {
            g_slots[i].scale_x = 0xFFF;
        } else {
            g_slots[i].scale_x = 0x1000;
        }
    }
}
