/* Persona 1 (JP) - moving a cursor over the fighters.  BTLP only.
 *   0x800C49E8 BtlPickableNext  0x800C4A48 BtlPickablePrev
 *   0x800C4AAC BtlDownMemberPrev  0x800C4B18 BtlDownMemberNext
 *
 * Two pairs of the same walk. The first goes over the whole combatant list,
 * nine slots, and stops on one that is occupied and marked pickable - that is
 * the target cursor. The second goes over the party's five and stops on a
 * member who is not dead, which is the command cursor's own version; the pair
 * in cursor.c does the same job with a longer list of tests.
 *
 * Neither can fail to return: the slot the caller came from passes its own
 * tests, so the search comes back round to it at worst.
 *
 * The wrap forward is written without a branch - the index is masked against
 * the comparison, so a slot past the end becomes zero - while the wrap
 * backward is a plain test. That asymmetry is the original's.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>

/* Slots in each list. */
#define TARGET_SLOTS 9
#define MEMBER_SLOTS 5


/* The trailing statement after each return is unreachable and load-bearing,
   the same way cursor.c's is: without it gcc shares one increment at the
   bottom of the loop instead of putting one on each back-edge. */
int BtlPickableNext(int slot)
{
    BtlActor *a;

    a = g_btl_combatants;
    for (;;) {
        slot++;
        slot = (slot < TARGET_SLOTS) ? slot : 0;
        if (a[slot].c.key != 0 && a[slot].pickable != 0) {
            break;
        }
    }
    return slot;
    slot++;
}

int BtlPickablePrev(int slot)
{
    BtlActor *a;

    a = g_btl_combatants;
    for (;;) {
        slot--;
        slot = (slot >= 0) ? slot : TARGET_SLOTS - 1;
        if (a[slot].c.key != 0 && a[slot].pickable != 0) {
            break;
        }
    }
    return slot;
    slot--;
}

int BtlDownMemberPrev(int slot)
{
    for (;;) {
        slot--;
        slot = (slot >= 0) ? slot : MEMBER_SLOTS - 1;
        if (g_btl_actors[slot].c.key != 0 &&
            (signed char)g_btl_actors[slot].c.status == BTL_STATUS_DOWN) {
            break;
        }
    }
    return slot;
    slot--;
}

int BtlDownMemberNext(int slot)
{
    for (;;) {
        slot++;
        slot = (slot < MEMBER_SLOTS) ? slot : 0;
        if (g_btl_actors[slot].c.key != 0 &&
            (signed char)g_btl_actors[slot].c.status == BTL_STATUS_DOWN) {
            break;
        }
    }
    return slot;
    slot++;
}
