/* Persona 1 (JP) - the next party member still without an order.  BTLP only.
 *   0x800C476C BtlUnreadyMemberNext
 *
 * The same walk as the cursor pairs beside it with two differences: it stops
 * at the end of the party rather than wrapping, so it can fail, and what it
 * is looking for is a member whose ready marker is not up. The caller uses it
 * to move on to whoever has still to be given an order, and reads -1 as "all
 * of them have".
 *
 * The marker byte is read unsigned where the ailment beside it is read signed;
 * both are u_char, and only the ailment is cast.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>

/* Marker values below this mean it is not up. */
#define BTL_MARKER_UP 2

int BtlUnreadyMemberNext(int slot)
{
    for (slot++; slot < BTL_PARTY; slot++) {
        if (g_btl_actors[slot].c.key != 0
            && (signed char)g_btl_actors[slot].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[slot].flags & BTL_ACTOR_OUT) == 0
            && g_btl_actors[slot].marker < BTL_MARKER_UP) {
            return slot;
        }
    }
    return -1;
}
