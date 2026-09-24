/* Persona 1 (JP) - giving a fighter an ailment.  BTLP only.
 *   0x800938B4 BtlInflictStatus
 *
 * Three ways to be turned down, in order: a demon that cannot catch this one
 * at all, an ailment already in place that this one may not land over, and an
 * ailment already at its deepest. Anything else lands, and the answer says
 * which way it went - callers that show a message read it.
 *
 * The same ailment landing again drives it a level deeper instead of starting
 * over, and a different one puts the level back to zero. Both cases fall into
 * the same tail, which sets how long it is meant to last and puts the marker
 * up - except for death, which has a whole sequence of its own elsewhere and
 * only wants the record changed.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/status.h>

/* 98.57%, from 90.05%. The level is a signed char, which is what the frame
   bytes are and what gives the image's copy of it in the branch slot. The
   turn count is stored in each arm, which is what ranks the record above the
   status in global alloc, and the row test reads the row first. The row is
   the table's base stepped on in a second statement, which is what loads the
   base before the multiply. Left: the row lands in a0 where the image has v0,
   so the constant 1 for the shift is set early instead of after the row's
   last use. A byte-offset spelling gets the registers right but loads the
   base late again. */
#ifdef NON_MATCHING
int BtlInflictStatus(BtlActor *a, int status)
{
    const u_long *row;
    BtlObj       *mark;
    signed char   level;

    if (a->c.key >= BTL_KEY_DEMON
        && (g_btl_demon_statuses[a->c.key] & (1 << status)) == 0) {
        return 0;
    }

    row = g_btl_status_over;
    row += status * BTL_AIL_LEVELS;
    level = a->c.ail_level;
    if (row[level] & (1 << (signed char)a->c.status)) {
        if ((signed char)a->c.status == status) {
            if (level == BTL_AIL_DEEPEST) {
                return 0;
            }
            a->c.ail_level = level + 1;
        } else {
            a->c.status = status;
            a->c.ail_level = 0;
        }

        if (status == 0x12) {
            a->ail_turns = BTL_AIL_TURNS_CLOAK;
        } else if (status == 0xE || status == 0xF
                   || status == 0x15 || status == 0x16) {
            a->ail_turns = BTL_AIL_TURNS_LONG;
        } else {
            a->ail_turns = BTL_AIL_TURNS_SHORT;
        }

        mark = a->obj->mark;
        if (status != BTL_STATUS_DOWN) {
            BtlObjSetScript(mark,
                            *(const u_long **)(g_btl_actor_gfx + status * 4
                                               + BTL_GFX_SCRIPTS));
            BtlObjSetScript(mark->attached,
                            g_btl_ail_level_marks[(signed char)a->c.ail_level]);
            mark->unkCE = status + BTL_MARK_BIAS;
            if ((signed char)a->c.status >= BTL_AIL_MARK_ONLY) {
                mark->attached->attr |= BTL_OBJ_HIDDEN;
            }
            mark->motion = BTL_MARK_MOTION;
            mark->timer = BTL_MARK_TIMER;
        }
        return 1;
    }
    return 0;
}
#else
INCLUDE_ASM("btlp/nonmatchings/inflict", BtlInflictStatus);
#endif
