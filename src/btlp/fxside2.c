/* Persona 1 (JP) - the move whose layers stand on every fighter opposite.
 * BTLP only.
 *   0x800B6E48 BtlFxStartSideLayers
 *
 * BtlFxStartOnSide again, three layers a fighter instead of one: the side the
 * acting fighter is not on is walked and every fighter of it still in the
 * fight is given a chain of its own, all of them chained together head first.
 *
 * Nothing is staggered here - each fighter's layers start at once, and the
 * colour they put it on is BtlOpenFxLayers's own rather than one written out
 * here.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* Which of a set each record stands for. */
#define FX_MARK_HEAD 0x10
#define FX_MARK_REST 0x11

BtlObj *BtlFxStartSideLayers(void)
{
    BtlObj *o;
    BtlObj *head;
    BtlObj *last;
    int     first;
    int     end;
    int     slot;

    if (g_btl_actor_turn < BTL_PARTY)
    {
        first = BTL_PARTY;
        end   = BTL_ACTORS;
    }
    else
    {
        first = 0;
        end   = BTL_PARTY;
    }

    slot = first;
    last = 0;
    while (slot < end)
    {
        if (g_btl_actors[slot].c.key != 0
            && (signed char)g_btl_actors[slot].c.status != BTL_STATUS_DOWN
            && !(g_btl_actors[slot].flags & BTL_ACTOR_OUT))
        {
            o = BtlOpenFxLayers(slot, 0);
            if (last != 0)
            {
                last->attached = o;
                o->mark_num    = FX_MARK_REST;
            }
            else
            {
                o->mark_num = FX_MARK_HEAD;
                head        = o;
            }
            last = BtlObjLast(o);
        }
        slot++;
    }
    return head;
}
