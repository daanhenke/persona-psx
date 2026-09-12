/* Persona 1 (JP) - the move that drops out of the sky, and a bare record on
 * the middle of the field.  BTLP only.
 *   0x800BDA9C BtlFxStartDB  0x800BDBA8 BtlFxStepDB  0x800BDC90 BtlFxStart75
 *
 * BtlFxStartDB opens one record over the fighter aimed at, a hundred and
 * twenty-eight units up and falling, and picks its artwork out of any of the
 * staged tables at random - twenty-nine of them, which is more than any other
 * handler reaches for.
 *
 * BtlFxStepDB drops it, adding a whole unit to the fall every frame so it
 * accelerates, and when it reaches the floor it shakes the field for half a
 * second and arms the hit.
 *
 * BtlFxStart75 is a different move: one plain record on the middle of the
 * field, sixty-four units toward the camera, and nothing more.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* How many of the staged script tables the falling record may be drawn from. */
#define FX_DB_TABLES 29

/* Where it starts and how fast it falls - 16.16, so a hundred and twenty-eight
   units up and a whole unit added to the fall each frame. */
#define FX_DB_HIGH  0x800000
#define FX_DB_ACCEL 0x10000

/* How long the field shakes once it lands, in frames. */
#define FX_DB_SHAKE 0x1E

/* What the step handler leaves behind. */
#define FX_DB_DONE 0x80

/* How far toward the camera move 0x75's record stands. */
#define FX_75_NEAR 0x400000

BtlObj *BtlFxStartDB(void)
{
    BtlObj *o;
    long    pos[3];

    pos[0] = g_btl_actors[g_btl_fx_target].obj->x;
    pos[1] = g_btl_actors[g_btl_fx_target].obj->y;
    pos[2] = 0;
    g_btl_fx_def.scripts =
        ((const u_long ***)g_btl_unused_gfx)[rand() % FX_DB_TABLES];
    o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0, pos,
                    FX_OBJ_CD, FX_OBJ_CE);
    o->attr = BTL_OBJ_NO_SHADOW;
    o->z = -FX_DB_HIGH;
    o->step_z = FX_DB_ACCEL;
    return o;
}

void BtlFxStepDB(BtlObj *o)
{
    switch (o->phase) {
    case 0:
        o->z += o->step_z;
        o->step_z += FX_DB_ACCEL;
        if (o->z < 0) {
            break;
        }
        g_btl_shake_on = 1;
        o->z = 0;
        o->timer = FX_DB_SHAKE;
        o->phase++;
        break;
    case 1:
        if (o->timer != 0) {
            break;
        }
        g_btl_shake_on = 0;
        o->phase = FX_DB_DONE;
        o->attr |= BTL_OBJ_HIDDEN;
        o->children = (u_char)g_btl_spell_fx[o->kind].group;
        BtlArmHitChain();
        break;
    default:
        BtlFxFinish01(o);
        break;
    }
}

BtlObj *BtlFxStart75(void)
{
    BtlObj *o;
    long    pos[3];

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    pos[2] = -FX_75_NEAR;
    pos[0] = 0;
    pos[1] = 0;
    o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0, pos,
                    FX_OBJ_CD, FX_OBJ_CE);
    o->mark_num = 0;
    o->attr |= FX_OBJ_ATTR;
    return o;
}
