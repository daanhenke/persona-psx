/* Persona 1 (JP) - an attribute bit set or cleared across the whole party.
 * BTLP only.
 *   0x800C5B20 BtlPartySetAttr  0x800C5BF0 BtlPartyClearAttr
 *
 * Three records a member: the member itself, the shadow under it and the
 * ailment marker over it. BtlObjSetAttr walks `attached` and would not reach
 * the other two, so these reach each of the three by name instead, for every
 * member who is alive and still in the fight.
 *
 * Each of the three is fetched off the record afresh: the write before it goes
 * through one of those pointers, so the object cannot be held across them.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>

void BtlPartySetAttr(u_long bits)
{
    int i;

    i = 0;
    do {
        if (g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
            g_btl_actors[i].obj->attr |= bits;
            g_btl_actors[i].obj->shadow->attr |= bits;
            g_btl_actors[i].obj->mark->attr |= bits;
        }
        i++;
    } while (i < BTL_PARTY);
}

void BtlPartyClearAttr(u_long bits)
{
    int i;

    i = 0;
    do {
        if (g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
            g_btl_actors[i].obj->attr &= ~bits;
            g_btl_actors[i].obj->shadow->attr &= ~bits;
            g_btl_actors[i].obj->mark->attr &= ~bits;
        }
        i++;
    } while (i < BTL_PARTY);
}
