/* Persona 1 (JP) - moving the command cursor over the party.  BTLP only.
 *   0x800C4814 BtlCursorNext   0x800C494C BtlCursorPrev
 *
 * Both walk the party's five slots until they find one the cursor may sit on,
 * and neither can fail to return: the slot the caller came from passes its own
 * tests, so the search comes back round to it at worst.
 *
 * The four tests are the same in each direction, and in the same order: the
 * slot must hold somebody - an empty record has no Char key - the actor must
 * not be down, must not carry the flag that takes it out of the fight, and must
 * not be in the state that leaves it targetable but unable to take an order.
 *
 * The battle reads the ailment byte as signed where Char declares it unsigned,
 * so the casts are load-bearing.
 */
#include <types.h>
#include <persona/btlp/actor.h>

int BtlCursorNext(int slot)
{
    for (;;) {
        slot++;
        slot = (slot < BTL_PARTY) ? slot : 0;
        if (g_btl_actors[slot].c.key != 0 &&
            (signed char)g_btl_actors[slot].c.status != BTL_STATUS_DOWN &&
            (g_btl_actors[slot].flags & BTL_ACTOR_OUT) == 0 &&
            (signed char)g_btl_actors[slot].c.status != BTL_STATUS_NOINPUT) {
            break;
        }
    }
    return slot;
    /* Unreachable, and load-bearing: without it gcc lays the loop out with a
       shared increment at the bottom instead of one per back-edge. Leave it. */
    slot++;
}

int BtlCursorPrev(int slot)
{
    for (;;) {
        slot--;
        slot = (slot >= 0) ? slot : BTL_PARTY - 1;
        if (g_btl_actors[slot].c.key != 0 &&
            (signed char)g_btl_actors[slot].c.status != BTL_STATUS_DOWN &&
            (g_btl_actors[slot].flags & BTL_ACTOR_OUT) == 0 &&
            (signed char)g_btl_actors[slot].c.status != BTL_STATUS_NOINPUT) {
            break;
        }
    }
    return slot;
    slot--;   /* unreachable, and load-bearing the same way */
}
