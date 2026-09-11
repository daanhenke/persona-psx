/* Persona 1 (JP) - an effect object standing on a fighter.  BTLP only.
 *   0x800B7924 BtlOpenFxObj
 *
 * Takes a record out of the effects group, stands it where the fighter in the
 * given slot is standing, and gives it a timer. The template it is built from
 * is filled in first from whichever script table the staged artwork carries,
 * so every effect object in flight shares the one template.
 *
 * The position is handed over as three words with no depth: effects are drawn
 * flat over the field, so only the fighter's x and y are taken.
 *
 * BtlOpenFxObj2 is the same routine again, a long way further down the
 * overlay. Two copies of it are in the image.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

BtlObj *BtlOpenFxObj(int slot, int timer)
{
    BtlObj *o;
    long    pos[3];

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    pos[0] = g_btl_actors[slot].obj->x;
    pos[1] = g_btl_actors[slot].obj->y;
    pos[2] = 0;
    o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0, pos,
                    FX_OBJ_CD, FX_OBJ_CE);
    o->attr = FX_OBJ_ATTR;
    o->timer = timer;
    return o;
}
