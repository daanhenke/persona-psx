/* Persona 1 (JP) - an effect object on every fighter the move reaches.
 * BTLP only.
 *   0x800C021C BtlOpenFxOnTargets
 *
 * Walks both sides against the acting fighter's target mask and opens one
 * effect record over each fighter it finds, chaining them together so the
 * whole set can be handled as one. Each record is given a later timer than
 * the one before it, which is what staggers the effect across the field.
 *
 * The fighter under each record is put on the colour it is handed before the
 * record is made, so the effect and the tint arrive together.
 *
 * The head of the chain is the one answered. A move whose mask reaches nobody
 * never sets it, and the caller is handed whatever the register held - the
 * mask is filled in from a live target, so it does not happen.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* How fast the fighter under a record walks toward the effect's colour. */
#define FX_FADE 2

/* How much later each record starts than the one before it. */
#define FX_STEP 2

#ifdef NON_MATCHING
BtlObj *BtlOpenFxOnTargets(int r, int g, int b, int timer)
{
    BtlObj  *o;
    BtlObj  *head;
    BtlObj  *last;
    int      slot;
    int      bit;
    u_short  targets;

    bit     = 1;
    slot    = 0;
    last    = 0;
    targets = g_btl_actors[g_btl_actor_turn].targets;

    for (; slot < BTL_ACTORS; slot++, bit <<= 1)
    {
        if (g_btl_actors[slot].c.key != 0
            && (signed char)g_btl_actors[slot].c.status != BTL_STATUS_DOWN
            && !(g_btl_actors[slot].flags & BTL_ACTOR_OUT)
            && (targets & bit) != 0)
        {
            g_btl_actors[slot].obj->rgb_to[0] = r;
            g_btl_actors[slot].obj->rgb_to[1] = g;
            g_btl_actors[slot].obj->rgb_to[2] = b;
            g_btl_actors[slot].obj->fade      = FX_FADE;

            o = BtlOpenFxObj(slot, timer);
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
            timer += FX_STEP;
            last = BtlObjLast(o);
        }
    }
    return head;
}
#else
INCLUDE_ASM("btlp/nonmatchings/fxtargets", BtlOpenFxOnTargets);
#endif
