/* Persona 1 (JP) - what an effect object does with each frame.  BTLP only.
 *   0x800B6BB0 BtlFxObjTick
 *   0x800B6C74 BtlFxStartNone
 *
 * An effect's records are ticked here rather than by a motion of their own.
 * The ordinary record hands the frame to whichever step handler the move's
 * record in g_btl_spell_fx carries, and only while the record is on the
 * motion BtlStartMoveFx put its chain on - so a record that has been moved off
 * it is left alone until it comes back.
 *
 * A record that carries the tracking bit is doing the other job: it is turned
 * to face the camera every frame, out of the same rotation the model pass
 * reads, and re-armed with the script it was given whenever it runs out. That
 * is the whole of the branch - it never reaches the move's step handler.
 *
 * BtlFxStartNone is the start handler two thirds of the table carries: the
 * spells with no effect of their own. It answers nothing at all, so
 * BtlStartMoveFx is handed whatever the register held and skips the chain -
 * which is the same thing an absent handler does, by a different route.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* The attribute bit that says this record follows the camera instead of the
   move. Set on the one record an effect stands its artwork on. */
#define FX_OBJ_TRACKING 0x40

/* The motion BtlStartMoveFx puts every record of the chain on. */
#define FX_MOTION 2

/* The camera's own rotation; the tracked record takes all three of it, the
   last through the name the opening pulls the camera back with. */
extern SVECTOR g_btl_cam_rot;

void BtlFxObjTick(BtlObj *o)
{
    void (*step)();
    /* The camera's third value is reached through its address rather than by
       name. Held here it costs a register the whole way through, which is
       what keeps the three reads of the rotation one behind the other: given
       two registers the compiler runs the last of them ahead of the second. */
    short *dist;

    dist = &g_btl_intro_dist;
    if (!(o->attr & FX_OBJ_TRACKING))
    {
        step = g_btl_spell_fx[g_btl_fx_move].step;
        if (step != 0 && o->motion == FX_MOTION)
        {
            step();
        }
    }
    else
    {
        o->rot.vx = g_btl_cam_rot.vx;
        o->rot.vy = g_btl_cam_rot.vy;
        o->rot.vz = *dist;
        if (!(o->attr & BTL_OBJ_ANIMATING))
        {
            BtlObjSetScript(o, (BtlSeqStep *)o->unk60);
        }
    }
}


void BtlFxStartNone(void)
{
}
