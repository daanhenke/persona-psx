/* Persona 1 (JP) - the move whose record is faced squarely at the camera.
 * BTLP only.
 *   0x800B9DF0 BtlFxStart33
 *
 * A start handler out of g_btl_spell_fx, and BtlFxStart6E written out again
 * with one difference: where 0x6E always stands its record a hundred units
 * above the middle of the field, this one picks the side. Move 0x56 puts it
 * over the side that is acting and everything else over the side being aimed
 * at, which is the only thing the two arms of the outer test differ in.
 *
 * As in 0x6E the record is given no scale at all - its script grows it - and
 * the three angles it is drawn through are the camera's own two plus the
 * distance the opening pulled the camera back by, which keeps it flat against
 * the view however the field is turned.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* How far above the middle of the field the record stands - 16.16, so a
   hundred whole units, the same as move 0x6E's. */
#define FX_33_UP 0x640000

/* The one move of the set that stands its record over its own side instead of
   the other one. */
#define FX_33_SELF 0x56

/* What the record starts as - the same as move 0x6E's: neither hidden nor
   static, and carrying the bit the whitening also sets. */
#define FX_33_ATTR (BTL_OBJ_ATTR_4000 | 0x5)

BtlObj *BtlFxStart33(void)
{
    BtlObj *o;
    long    pos[3];
    short   n;

    pos[0] = 0;
    pos[2] = 0;
    /* Both sides are picked inside the store, and the two are spelled with
       opposite tests although they mean the same pair of places: written as
       an if of its own each arm costs three words, and written with the same
       test both ways one of the two branches comes out inverted. */
    if (g_btl_fx_move == FX_33_SELF) {
        pos[1] = g_btl_actor_turn >= BTL_PARTY ? -FX_33_UP : FX_33_UP;
    } else {
        pos[1] = g_btl_actor_turn < BTL_PARTY ? -FX_33_UP : FX_33_UP;
    }
    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0, pos,
                    FX_OBJ_CD, FX_OBJ_CE);
    o->attr = FX_33_ATTR;
    o->mark_num = FX_MARK_HEAD;
    o->scale_x = 0;
    o->scale_y = 0;
    /* The three angles go through the one local, the way 0x6E's tail does. */
    n = g_btl_cam_rot.vx;
    o->rot.vx = n;
    n = g_btl_cam_rot.vy;
    o->rot.vy = n;
    n = g_btl_intro_dist;
    o->rot.vz = n;
    return o;
}
