/* Persona 1 (JP) - the move that grows, drops a sheet and marks the runner.
 * BTLP only.
 *   0x800BBFDC BtlFxStep4C
 *
 * Three records, told apart by their mark, and each with a switch of its own.
 *
 * The head grows to thirty-one times its size, opens the second record, waits
 * two seconds, darkens, and finally - if the fight is one that may be run from
 * - writes down whose turn it is and counts the acting fighter up. It turns on
 * its own axis every frame, whichever arm it took.
 *
 * The second record waits its timer out, is put on the third of its model's
 * scripts, and on the frame after that lays a sheet over the field and starts
 * darkening; it frees itself once it is black.
 *
 * Anything else is one of the sheet's own cells: it uncovers itself and frees
 * itself when its script has played out.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* How fast the head grows and how big it gets - unity is 0x100. */
#define FX_4C_GROW 0x80
#define FX_4C_FULL 0x1F40

/* How long it holds, how fast it darkens, and how far it turns each frame. */
#define FX_4C_HOLD 0x78
#define FX_4C_FADE 2
#define FX_4C_SPIN 0x20

/* The second record: its script table, what it carries, and how long it
   waits. */
#define FX_4C_TABLE  1
#define FX_4C_MOTION 2
#define FX_4C_WAIT   0x28

/* Which of the model's scripts it is put on, and which staged table the sheet
   it lays comes from. */
#define FX_4C_SCRIPT 2
#define FX_4C_SHEET  3

/* The mark the sheet's own cells carry. */
#define FX_COPY_MARK 0xFF

void BtlFxStep4C(BtlObj *o)
{
    BtlObj *n;
    const u_long **scripts;

    switch (o->mark_num) {
    case FX_MARK_HEAD:
        switch (o->phase) {
        case 0:
            o->scale_x += FX_4C_GROW;
            o->scale_y += FX_4C_GROW;
            if (o->scale_x < FX_4C_FULL) {
                break;
            }
            o->timer = FX_4C_HOLD;
            o->phase++;
            break;
        case 1:
            g_btl_fx_def.scripts =
                ((const u_long ***)g_btl_unused_gfx)[FX_4C_TABLE];
            n = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0,
                            &o->x, FX_OBJ_CD, FX_OBJ_CE);
            n->attr |= BTL_OBJ_NO_SHADOW;
            n->kind = o->kind;
            scripts = o->scripts;
            n->mark_num = FX_MARK_REST;
            n->motion = FX_4C_MOTION;
            n->timer = FX_4C_WAIT;
            n->scripts = scripts;
            o->phase++;
            break;
        case 2:
            if (o->timer != 0) {
                break;
            }
            o->rgb_to[0] = 0;
            o->rgb_to[1] = 0;
            o->rgb_to[2] = 0;
            o->fade = FX_4C_FADE;
            o->phase++;
            break;
        case 3:
            if (g_btl_no_escape == 0) {
                g_btl_pick_slowest = g_btl_actors[g_btl_actor_turn].c.key;
                g_btl_actors[g_btl_actor_turn].unkD0++;
            }
            o->motion = 0;
            o->phase = 0;
            break;
        default:
            break;
        }
        o->rot.vz += FX_4C_SPIN;
        break;
    case FX_MARK_REST:
        switch (o->phase) {
        case 0:
            if (o->timer != 0) {
                break;
            }
            BtlObjSetScript(o, (BtlSeqStep *)o->scripts[FX_4C_SCRIPT]);
            o->timer = 0;
            o->phase++;
            break;
        case 1:
            if (o->timer != 0) {
                break;
            }
            n = BtlOpenFxGrid(FX_4C_SHEET);
            BtlObjSetKind(n, o->kind);
            BtlObjSetMarkNum(n, FX_COPY_MARK);
            BtlObjSetMotion(n, FX_4C_MOTION);
            o->fade = FX_4C_FADE;
            o->rgb_to[0] = 0;
            o->rgb_to[1] = 0;
            o->rgb_to[2] = 0;
            o->phase++;
            break;
        case 2:
            if (o->rgb[0] != 0) {
                break;
            }
            BtlObjFree(o);
            break;
        default:
            break;
        }
        break;
    default:
        switch (o->phase) {
        case 0:
            if (o->timer != 0) {
                break;
            }
            o->attr &= ~(BTL_OBJ_HIDDEN | BTL_OBJ_STATIC);
            o->phase++;
            break;
        case 1:
            if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
                break;
            }
            BtlObjFree(o);
            break;
        default:
            break;
        }
        break;
    }
}
