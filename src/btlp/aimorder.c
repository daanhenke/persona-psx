/* Persona 1 (JP) - which fighter an attack is aimed at.  BTLP only.
 *   0x8009BA20 BtlSlowestOrder  0x8009BADC BtlFrontMemberOrder
 *
 * A move that picks its own target still has to name one fighter as the one
 * it is aimed at - BtlActor.order - and these are the two ways of choosing it.
 * Both walk a side, weigh every fighter still standing on it by the grid and
 * answer a slot on that side, or -1 when nobody on it can be reached.
 *
 * The weighing is the same in both: the row decides, and the column breaks the
 * tie by whoever is nearest the attacker's own column. They differ only in
 * which end of the grid wins. The enemy walk starts at row zero and keeps
 * taking a row at least as far back, so it ends on the hindmost enemy; the
 * party walk starts past the last row and keeps taking a row at least as far
 * forward, so it ends on the frontmost member.
 *
 * A column is held doubled on the object, so the distance is in half-columns
 * and 0xFF is further apart than the grid can be.
 *
 * The enemy walk asks only whether the record is occupied and pickable,
 * because the side has already been made pickable before it runs. The party
 * walk is handed an actor rather than reading whose turn it is, and tests
 * being down itself - it is the enemy's aim, and nothing has filtered the
 * party for it.
 *
 * Both are 99%-and-change and stay behind INCLUDE_ASM. The whole of what is
 * left is the order of the two constants the walk starts from: the image sets
 * the distance up before the row and keeps the row in the lower register,
 * and no order of the statements gets both at once - whichever is written
 * first is both emitted first and given that register.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>

/* Further apart than two fighters on the grid can be. */
#define ORDER_FAR 0xFF

#ifdef NON_MATCHING
int BtlSlowestOrder(void)
{
    BtlActor *a;
    BtlObj   *o;
    int       best;
    int       row;
    int       dist;
    int       col;
    int       gap;
    int       at;
    int       i;

    dist = ORDER_FAR;
    row  = 0;
    best = -1;
    col  = g_btl_actors[g_btl_actor_turn].obj->col2;
    i    = 0;
    a    = g_btl_combatants;
    do {
        if (a[i].c.key != 0 && a[i].pickable != 0) {
            o   = a[i].obj;
            gap = o->col2 - col;
            at  = o->row;
            gap = gap < 0 ? -gap : gap;
            if (at >= row && gap <= dist) {
                row  = at;
                dist = gap;
                best = i;
            }
        }
        i++;
    } while (i < BTL_ENEMIES);
    return best;
}
#else
INCLUDE_ASM("btlp/nonmatchings/aimorder", BtlSlowestOrder);
#endif

#ifdef NON_MATCHING
int BtlFrontMemberOrder(BtlActor *by)
{
    BtlObj *o;
    int     best;
    int     row;
    int     dist;
    int     col;
    int     gap;
    int     at;
    int     i;

    row  = BTL_PARTY;
    dist = ORDER_FAR;
    best = -1;
    col  = by->obj->col2;
    i    = 0;
    do {
        if (g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0
            && g_btl_actors[i].pickable != 0) {
            o   = g_btl_actors[i].obj;
            gap = o->col2 - col;
            at  = o->row;
            gap = gap < 0 ? -gap : gap;
            if (at <= row && gap <= dist) {
                row  = at;
                dist = gap;
                best = i;
            }
        }
        i++;
    } while (i < BTL_PARTY);
    return best;
}
#else
INCLUDE_ASM("btlp/nonmatchings/aimorder", BtlFrontMemberOrder);
#endif
