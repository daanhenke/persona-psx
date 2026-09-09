/* Persona 1 (JP) - the little marker that says what is wrong with someone.
 *   0x80093660 BtlShowAilmentMarks    BTLP only.
 *
 * Every actor's object carries a second pair of records on `mark`, and the
 * frame tick keeps them on its x, y and z, so they float wherever the fighter
 * is. This is what arms them: the marker itself takes a script out of the
 * ailment table at +0x74 of the actor graphics, indexed by the ailment, and
 * the piece attached to it takes one out of g_btl_ail_level_marks, indexed by the
 * fighter's kind.
 *
 * Ailments from 13 up show only the marker; below that the attached piece is
 * shown too, which is the one thing that tells the two ranges of codes apart
 * here.
 *
 * Everything is hidden again for an actor with no ailment, one that is out of
 * the fight, one whose own object is already hidden, and for everyone at once
 * when the caller asks for nothing to be shown - which the negotiation does
 * before it puts a demon's face up.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/status.h>

#ifdef NON_MATCHING
void BtlShowAilmentMarks(int show)
{
    BtlObj *mark;
    int     status;
    int     i;

    for (i = 0; i < BTL_ACTORS; i++) {
        mark = g_btl_actors[i].obj->mark;
        if (mark != 0
            && g_btl_actors[i].c.key != 0
            && (status = *(signed char *)&g_btl_actors[i].c.status) != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
            if (show == 0
                || (g_btl_actors[i].obj->attr & BTL_OBJ_HIDDEN) != 0
                || status == 0) {
                g_btl_actors[i].obj->mark->attr |= BTL_OBJ_HIDDEN;
                g_btl_actors[i].obj->mark->attached->attr |= BTL_OBJ_HIDDEN;
            } else {
                BtlObjSetScript(mark,
                                *(const u_long **)(g_btl_actor_gfx + status * 4
                                                   + BTL_GFX_SCRIPTS));
                BtlObjSetScript(mark->attached,
                                g_btl_ail_level_marks[*(signed char *)&g_btl_actors[i].c.ail_level]);
                mark->unkCE = g_btl_actors[i].c.status + BTL_MARK_BIAS;
                if (*(signed char *)&g_btl_actors[i].c.status < BTL_AIL_MARK_ONLY) {
                    g_btl_actors[i].obj->mark->attached->attr &= ~BTL_OBJ_HIDDEN;
                } else {
                    g_btl_actors[i].obj->mark->attached->attr |= BTL_OBJ_HIDDEN;
                }
                g_btl_actors[i].obj->mark->attr &= ~BTL_OBJ_HIDDEN;
                g_btl_actors[i].obj->mark->motion = 0;
                g_btl_actors[i].obj->mark->phase = 0;
            }
        }
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/ailmentmarks", BtlShowAilmentMarks);
#endif

