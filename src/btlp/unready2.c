/* Persona 1 (JP) - the previous party member still without an order.
 *   0x800C48AC BtlUnreadyMemberPrev    BTLP only.
 *
 * The other direction of unready.c, split off the same way cursor2.c is split
 * off cursor.c: the walk runs down to slot zero and answers -1 below it.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
/* Defined in the unit before this one; the prototype is what
   decides how the arguments are converted. */

/* Marker values below this mean it is not up. */
#define BTL_MARKER_UP 2

int BtlUnreadyMemberPrev(int slot)
{
    for (slot--; slot >= 0; slot--) {
        if (g_btl_actors[slot].c.key != 0
            && (signed char)g_btl_actors[slot].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[slot].flags & BTL_ACTOR_OUT) == 0
            && g_btl_actors[slot].marker < BTL_MARKER_UP) {
            return slot;
        }
    }
    return -1;
}
