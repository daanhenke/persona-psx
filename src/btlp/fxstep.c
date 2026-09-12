/* Persona 1 (JP) - an effect record's own three phases.  BTLP only.
 *   0x800B71F8 BtlFxObjStep
 *
 * The tick every record of an effect chain runs, whatever move made it.
 *
 *   phase 0  waiting. The record is hidden and standing still until its timer
 *            runs out, and then it is shown and set going.
 *   phase 1  running. Once the script has finished, the head of the chain -
 *            the record marked 0 or 0x10 - takes the effect away, arms the
 *            hit, and puts the move's family on itself; every other record
 *            just frees itself.
 *   above    the chain is done with, and each record hands its frame on.
 *
 * The family is worked out again here from the move id rather than read off
 * the move's own record in g_btl_spell_fx, which carries the same three
 * numbers in its last word.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* The two phases the record does something in, and what it is put on once the
   effect is over. */
#define FX_PHASE_WAIT 0
#define FX_PHASE_RUN  1
#define FX_PHASE_DONE 0x80

/* Where the move ids stop belonging to each family. */
#define FX_FAMILY1_END 7
#define FX_FAMILY2_END 13

/* The two ways the hit is armed once the effect has played out: one record
   standing on its own, or the head of a chain of them. They differ in a single
   value they leave behind. */
extern void BtlArmHitOne(void);
extern void BtlArmHitChain(void);
extern void BtlFinishMoveFx(BtlObj *obj);

void BtlFxObjStep(BtlObj *o)
{
    u_char move;

    switch (o->phase) {
    case FX_PHASE_WAIT:
        if (o->timer == 0) {
            o->attr &= ~(BTL_OBJ_HIDDEN | BTL_OBJ_STATIC);
            o->phase++;
        }
        break;

    case FX_PHASE_RUN:
        if (o->mark_num == 0 || o->mark_num == FX_MARK_HEAD) {
            if (!(o->attr & BTL_OBJ_ANIMATING)) {
                o->attr |= BTL_OBJ_HIDDEN;
                o->phase = FX_PHASE_DONE;
                if (o->mark_num == 0) {
                    BtlArmHitOne();
                } else {
                    BtlArmHitChain();
                }
                move = g_btl_actors[g_btl_actor_turn].move;
                if ((u_int)move < FX_FAMILY1_END) {
                    o->children = 1;
                } else if ((u_int)move < FX_FAMILY2_END) {
                    o->children = 2;
                } else {
                    o->children = 0;
                }
            }
        } else {
            if (!(o->attr & BTL_OBJ_ANIMATING)) {
                BtlObjFree(o);
            }
        }
        break;

    default:
        BtlFinishMoveFx(o);
        break;
    }
}
