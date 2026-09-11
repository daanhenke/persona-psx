/* Persona 1 (JP) - a move whose effect is drawn and moving at once.
 * BTLP only.
 *   0x800BE38C BtlFxStart86
 *
 * A start handler out of g_btl_spell_fx. The record is the ordinary one on the
 * fighter aimed at, with the two bits an effect normally opens holding - hidden
 * and static - cleared straight away, so the artwork is drawn and its script
 * runs from the frame it is taken rather than from the next. Move 0x87 has the
 * same handler written out again.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* None of these staggers its records. */
#define FX_TIMER 0

BtlObj *BtlFxStart86(void)
{
    BtlObj *obj;

    obj = BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
    obj->attr &= ~(BTL_OBJ_HIDDEN | BTL_OBJ_STATIC);
    return obj;
}
