/* Persona 1 (JP) - move 0x32's effect: a stack on everyone it reaches.
 * BTLP only.
 *   0x800B9C30 BtlFxStart32
 *
 * A start handler out of g_btl_spell_fx, and what BtlFxStart4D opens too.
 * Every fighter the acting record's target mask reaches - in the fight,
 * neither down nor out - gets a stack of its own from BtlOpenFxStack, and the
 * stacks are chained one behind another through the last record of each, so
 * the set is carried as one. Each stack's mark is counted up as it is opened;
 * the last one opened is given FX_32_LAST_MARK and answered.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

#define FX_32_LAST_MARK 0x10

BtlObj *BtlFxStart32(void)
{
    BtlObj *o;
    BtlObj *prev;
    int     i;
    int     bit;

    i = 0;
    bit = 1;
    prev = NULL;
    do {
        if ((g_btl_actors[g_btl_actor_turn].targets & bit) != 0
            && g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
            o = BtlOpenFxStack(i);
            o->mark_num++;
            if (prev != NULL) {
                BtlObjLast(o)->attached = prev;
            }
            prev = o;
        }
        i++;
        bit <<= 1;
    } while (i < BTL_ACTORS);
    o->mark_num = FX_32_LAST_MARK;
    return o;
}
