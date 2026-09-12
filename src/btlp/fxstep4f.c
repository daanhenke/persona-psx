/* Persona 1 (JP) - the move that grows, flashes the field white and pulls the
 * camera's own record through it.  BTLP only.
 *   0x800BC39C BtlFxStep4F
 *
 * Two records, told apart by their mark. The head grows a hundred and twenty-
 * eighth of full a frame until it stands thirty-one times its own size, opens
 * the second out of the artwork's second script table, waits two seconds,
 * darkens and arms the hit. It turns a thirty-second of a turn on its own axis
 * every frame it does any of that, which is the one thing every arm of its
 * switch shares.
 *
 * The second record falls a unit a frame until it is sixty-four units past the
 * middle, and then borrows the opening's own white flash: the field's flash
 * record is put on full white, uncovered, and taken away again two thirds of a
 * second later.
 */
#include <decomp/types.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* How fast the head grows and how big it gets - unity is 0x100. */
#define FX_4F_GROW 0x80
#define FX_4F_FULL 0x1F40

/* How long it holds at full size, and how fast it darkens after. */
#define FX_4F_HOLD 0x78
#define FX_4F_FADE 2

/* How far it turns each frame; a whole turn is 0x1000. */
#define FX_4F_SPIN 0x20

/* The second record: which script table it comes from, what it carries, how
   fast it falls and how far before the flash. */
#define FX_4F_TABLE  1
#define FX_4F_MOTION 2
#define FX_4F_DROP   0x10000
#define FX_4F_DEEP   (-0x400000)

/* The flash: how bright, how fast, how long, and the page it needs. */
#define FX_4F_LIT   0xFF
#define FX_4F_FLASH 8
#define FX_4F_WAIT  0x20
#define FX_4F_PAGE  0x20

/* What the head leaves behind. */
#define FX_4F_DONE 0x80

void BtlFxStep4F(BtlObj *o)
{
    BtlObj *flash;
    BtlObj *dim;
    BtlObj *n;
    const u_long **scripts;
    int     phase;

    switch (o->mark_num) {
    case FX_MARK_HEAD:
        phase = o->phase;
        switch (phase) {
        case 0:
            o->scale_x += FX_4F_GROW;
            o->scale_y += FX_4F_GROW;
            if (o->scale_x < FX_4F_FULL) {
                break;
            }
            o->timer = FX_4F_HOLD;
            o->phase++;
            break;
        case 1:
            g_btl_fx_def.scripts =
                ((const u_long ***)g_btl_unused_gfx)[FX_4F_TABLE];
            n = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0,
                            &o->x, FX_OBJ_CD, FX_OBJ_CE);
            n->attr = BTL_OBJ_NO_SHADOW;
            n->kind = o->kind;
            scripts = o->scripts;
            n->mark_num = FX_MARK_REST;
            n->motion = FX_4F_MOTION;
            n->fade = FX_4F_FLASH;
            n->rgb[0] = 0;
            n->rgb[1] = 0;
            n->rgb[2] = 0;
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
            o->fade = FX_4F_FADE;
            o->phase++;
            break;
        case 3:
            o->attr |= BTL_OBJ_HIDDEN;
            BtlArmHitChain();
            o->phase = FX_4F_DONE;
            o->children = (u_char)g_btl_spell_fx[g_btl_fx_move].group;
            break;
        default:
            BtlFxFinish01(o);
            break;
        }
        o->rot.vz += FX_4F_SPIN;
        break;
    case FX_MARK_REST:
        switch (o->phase) {
        case 0:
            o->z -= FX_4F_DROP;
            if (o->z >= FX_4F_DEEP) {
                break;
            }
            flash = g_btl_intro_obj;
            g_btl_tpage[0] = FX_4F_PAGE;
            flash->rgb_to[0] = FX_4F_LIT;
            flash->rgb_to[1] = FX_4F_LIT;
            flash->rgb_to[2] = FX_4F_LIT;
            flash->rgb[0] = 0;
            flash->rgb[1] = 0;
            flash->rgb[2] = 0;
            flash->fade = FX_4F_FLASH;
            flash->attr &= ~BTL_OBJ_HIDDEN;
            o->timer = FX_4F_WAIT;
            o->phase++;
            break;
        case 1:
            if (o->timer != 0) {
                break;
            }
            dim = g_btl_intro_obj;
            dim->rgb_to[0] = 0;
            dim->rgb_to[1] = 0;
            dim->rgb_to[2] = 0;
            o->timer = FX_4F_WAIT;
            o->attr |= BTL_OBJ_HIDDEN;
            o->phase++;
            break;
        case 2:
            if (o->timer != 0) {
                break;
            }
            g_btl_intro_obj->attr |= BTL_OBJ_HIDDEN;
            BtlObjFree(o);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}
