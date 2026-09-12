/* Persona 1 (JP) - the move whose one record stands over the field at a depth
 * its own id picks.  BTLP only.
 *   0x800BADA4 BtlFxStart3A
 *
 * A start handler out of g_btl_spell_fx, shared by moves 0x39 and 0x3A. The
 * position is not built on the stack: the two moves stand their record at the
 * same place except for how far toward the camera it is, so the handler writes
 * that one word into the shared g_btl_fx_lift and hands the whole vector over.
 * Eighty-eight units for 0x3A and sixty for its partner.
 *
 * The record is drawn and stepped from the frame it is taken - neither hidden
 * nor static - and starts black, walking to half grey three steps a frame.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* Which move takes the deeper of the two places, and what the two are. */
#define FX_3A_MOVE 0x3A
#define FX_3A_DEEP 0x580000
#define FX_3A_NEAR 0x3C0000

/* What the record starts as, and how fast it takes on colour. */
#define FX_3A_ATTR BTL_OBJ_NO_SHADOW
#define FX_3A_FADE 3

BtlObj *BtlFxStart3A(void)
{
    BtlObj *o;
    short   n;

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    /* The depth is chosen inside the store, not through a local and not in
       two stores of its own: either of those puts the address of the vector
       somewhere the image does not have it. */
    g_btl_fx_lift[2] =
        g_btl_fx_move == FX_3A_MOVE ? -FX_3A_DEEP : -FX_3A_NEAR;
    o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0,
                    g_btl_fx_lift, FX_OBJ_CD, FX_OBJ_CE);
    o->attr = FX_3A_ATTR;
    o->mark_num = FX_MARK_HEAD;
    o->fade = FX_3A_FADE;
    /* The walked-to colour goes through the one local, and the colour it
       starts on after it - the order the image writes them in. */
    n = FX_GREY;
    o->rgb_to[0] = n;
    o->rgb_to[1] = n;
    o->rgb_to[2] = n;
    o->rgb[0] = 0;
    o->rgb[1] = 0;
    o->rgb[2] = 0;
    return o;
}
