/* Persona 1 (JP) - five moves whose effect stands on everything they reach.
 * BTLP only.
 *   0x800BB210 BtlFxStart3B  0x800BB23C BtlFxStart3C  0x800BB268 BtlFxStart3D
 *   0x800BB294 BtlFxStart3F  0x800BB2C0 BtlFxStart40
 *
 * Five start handlers out of g_btl_spell_fx, and one body: a record over every
 * fighter the move reaches, with each of them walked to grey rather than to
 * white. The move between the third and the fourth - 0x3E - shares its handler
 * with 0x3A and so is not here; the five that are left run consecutively in
 * the image because that is the order their records sit in the table.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

BtlObj *BtlFxStart3B(void)
{
    return BtlOpenFxOnTargets(FX_GREY, FX_GREY, FX_GREY, FX_TIMER);
}

BtlObj *BtlFxStart3C(void)
{
    return BtlOpenFxOnTargets(FX_GREY, FX_GREY, FX_GREY, FX_TIMER);
}

BtlObj *BtlFxStart3D(void)
{
    return BtlOpenFxOnTargets(FX_GREY, FX_GREY, FX_GREY, FX_TIMER);
}

BtlObj *BtlFxStart3F(void)
{
    return BtlOpenFxOnTargets(FX_GREY, FX_GREY, FX_GREY, FX_TIMER);
}

BtlObj *BtlFxStart40(void)
{
    return BtlOpenFxOnTargets(FX_GREY, FX_GREY, FX_GREY, FX_TIMER);
}
