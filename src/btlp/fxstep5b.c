/* Persona 1 (JP) - the move that throws sparks off in every direction.
 * BTLP only.
 *   0x800BC99C BtlFxStep5B
 *
 * Every fourth frame the record opens a spark of its own out of the artwork's
 * second script table, points it at a random angle, and gives it a standing
 * displacement of that angle's sine and cosine along with a step back toward
 * the middle - so each spark is thrown out and drawn in again over the thirty-
 * two frames it is allowed. The sparks carry the copy mark and do nothing but
 * walk themselves and free themselves.
 *
 * Meanwhile the record itself darkens, and the hit is armed the frame it
 * reaches black.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* The mark a spark carries. */
#define FX_COPY_MARK 0xFF

/* One spark every fourth frame, out of the second script table. */
#define FX_5B_EVERY 3
#define FX_5B_TABLE 1

/* The two bits a spark is given on top of what BtlObjAlloc left. */
#define FX_5B_ATTR 0xC0000

/* How far a spark stands out and how fast it comes back - the wave tables are
   12.12, so seven shifts put a whole reading a unit out and two make the step
   a thirty-second of it. */
#define FX_5B_OUT  7
#define FX_5B_BACK 2

/* How long a spark lives, in frames. */
#define FX_5B_LIFE 0x20

/* How fast the record itself darkens, and what it leaves behind. */
#define FX_5B_FADE 2
#define FX_5B_DONE 0x80

void BtlFxStep5B(BtlObj *o)
{
    BtlObj *spark;

    switch (o->mark_num) {
    case FX_MARK_HEAD:
        switch (o->phase) {
        case 0:
            if ((g_btl_tick & FX_5B_EVERY) == 0) {
                g_btl_fx_def.scripts =
                    ((const u_long ***)g_btl_unused_gfx)[FX_5B_TABLE];
                spark = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0,
                                    FX_OBJ_DRAW, 0, &o->x,
                                    FX_OBJ_CD, FX_OBJ_CE);
                spark->mark_num = FX_COPY_MARK;
                spark->attr |= FX_5B_ATTR;
                spark->motion = o->motion;
                spark->kind = o->kind;
                spark->angle = rand() & BTL_WAVE_MASK;
                spark->shift_x = g_btl_wave_sin[spark->angle] << FX_5B_OUT;
                spark->shift = g_btl_wave_cos[spark->angle] << FX_5B_OUT;
                spark->step_x = -g_btl_wave_sin[spark->angle] << FX_5B_BACK;
                spark->step_y = -g_btl_wave_cos[spark->angle] << FX_5B_BACK;
                spark->steps = FX_5B_LIFE;
            }
            if (o->timer != 0) {
                break;
            }
            o->rgb_to[0] = 0;
            o->rgb_to[1] = 0;
            o->rgb_to[2] = 0;
            o->fade = FX_5B_FADE;
            o->phase++;
            break;
        case 1:
            if (o->rgb[0] != 0) {
                break;
            }
            o->phase = FX_5B_DONE;
            o->attr |= BTL_OBJ_HIDDEN;
            o->children = (u_char)g_btl_spell_fx[o->kind].group;
            BtlArmHitChain();
            break;
        default:
            BtlFxFinish53(o);
            break;
        }
        break;
    case FX_COPY_MARK:
        if (o->phase != 0) {
            break;
        }
        o->shift_x += o->step_x;
        o->shift += o->step_y;
        o->steps--;
        if (o->steps == 0) {
            BtlObjFree(o);
        }
        break;
    default:
        break;
    }
}
