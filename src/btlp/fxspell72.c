/* Persona 1 (JP) - the move whose one record stands in the middle of the
 * field.  BTLP only.
 *   0x800BD330 BtlFxStart72
 *
 * A start handler out of g_btl_spell_fx. One record on g_btl_fx_centre - the
 * middle of the field, sixty-four units toward the camera - drawn and stepped
 * from the frame it is taken and carrying only the bit that says it has no
 * shadow. Everything else about it comes from its script.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

BtlObj *BtlFxStart72(void)
{
    BtlObj *o;

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0,
                    g_btl_fx_centre, FX_OBJ_CD, FX_OBJ_CE);
    o->mark_num = FX_MARK_HEAD;
    o->attr |= BTL_OBJ_NO_SHADOW;
    return o;
}
