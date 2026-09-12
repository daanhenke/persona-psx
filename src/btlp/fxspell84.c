/* Persona 1 (JP) - the move whose effect starts above the fighter and falls.
 * BTLP only.
 *   0x800BDF60 BtlFxStart84
 *
 * A start handler out of g_btl_spell_fx, shared by moves 0x84 and 0x85. The
 * record is the ordinary one on the fighter aimed at, and then three things are
 * written on it by hand: a count for the step handler to work down, four units
 * of height so the artwork begins above the fighter and drops onto it, and the
 * hidden bit cleared so it is drawn from this frame rather than the next.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* None of these staggers its records. */
#define FX_TIMER 0

/* What the step handler counts down, and how far above the fighter the record
   starts - 16.16, so four whole units. */
#define FX_84_COUNT 0x40
#define FX_84_RISE  0x400000

BtlObj *BtlFxStart84(void)
{
    BtlObj *obj;

    obj = BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
    obj->steps = FX_84_COUNT;
    obj->y    -= FX_84_RISE;
    obj->attr &= ~BTL_OBJ_HIDDEN;
    return obj;
}
