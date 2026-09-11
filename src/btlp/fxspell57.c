/* Persona 1 (JP) - four moves on everything they reach, and one that borrows a
 * later effect and holds it.  BTLP only.
 *   0x800BC8C4 BtlFxStart57  0x800BC8F0 BtlFxStart58  0x800BC91C BtlFxStart59
 *   0x800BC948 BtlFxStart5A  0x800BC974 BtlFxStart5B
 *
 * Start handlers out of g_btl_spell_fx. The first four are the same record over
 * everything the move reaches with the fighters walked to white; the fifth
 * hands its frame to move 0x72's handler and then writes the chain's timer
 * itself, which is the one thing it wants differently - the effect it borrows
 * stands for two seconds rather than for however long 0x72 leaves it.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

#define FX_TIMER 0
#define FX_WHITE 0xFF

/* How long the borrowed effect is left standing, in frames. */
#define FX_5B_TIMER 0x78

BtlObj *BtlFxStart57(void)
{
    return BtlOpenFxOnTargets(FX_WHITE, FX_WHITE, FX_WHITE, FX_TIMER);
}

BtlObj *BtlFxStart58(void)
{
    return BtlOpenFxOnTargets(FX_WHITE, FX_WHITE, FX_WHITE, FX_TIMER);
}

BtlObj *BtlFxStart59(void)
{
    return BtlOpenFxOnTargets(FX_WHITE, FX_WHITE, FX_WHITE, FX_TIMER);
}

BtlObj *BtlFxStart5A(void)
{
    return BtlOpenFxOnTargets(FX_WHITE, FX_WHITE, FX_WHITE, FX_TIMER);
}

BtlObj *BtlFxStart5B(void)
{
    BtlObj *obj;

    obj = BtlFxStart72();
    obj->timer = FX_5B_TIMER;
    return obj;
}
