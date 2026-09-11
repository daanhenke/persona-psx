/* Persona 1 (JP) - an effect object standing on a fighter, again.  BTLP only.
 *   0x800C0160 BtlOpenFxObj2
 *
 * The same routine as BtlOpenFxObj in fxobj.c, a long way further down the
 * overlay: two copies of it are in the image. This is the one nearly every
 * effect handler calls - thirty-nine of them - and the first is reached from
 * three.
 *
 * See fxobj.c for what it does.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>

/* The group effect records come out of, and what one is drawn as. */
#define FX_OBJ_GROUP 2
#define FX_OBJ_DRAW  5

/* The two bytes BtlObjAlloc leaves at +0xCD and +0xCE. */
#define FX_OBJ_CD 0x1D
#define FX_OBJ_CE 0xE

/* What the record starts as: hidden, static, and without a shadow. */
#define FX_OBJ_ATTR (BTL_OBJ_HIDDEN | BTL_OBJ_STATIC | BTL_OBJ_NO_SHADOW)

extern BtlObjDef g_btl_fx_def;

extern u_char *g_btl_unused_gfx;

BtlObj *BtlOpenFxObj2(int slot, int timer)
{
    BtlObj *o;
    long    pos[3];

    pos[0] = g_btl_actors[slot].obj->x;
    pos[1] = g_btl_actors[slot].obj->y;
    pos[2] = 0;
    /* The template is filled in after the position here and before it in
       fxobj.c; the two copies differ in nothing else. */
    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0, pos,
                    FX_OBJ_CD, FX_OBJ_CE);
    o->attr = FX_OBJ_ATTR;
    o->timer = timer;
    return o;
}
