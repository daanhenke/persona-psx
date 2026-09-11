/* Persona 1 (JP) - the two moves whose sheet arrives on a blue of its own.
 * BTLP only.
 *   0x800B7740 BtlFxStartSheetLit
 *   0x800B7778 BtlFxStartSheetLit2
 *
 * The same pair as BtlFxStartSheet, except that the colour the fighters are
 * put on is written out here rather than taken from the effect's own. Both
 * handlers are the same routine again; the moves differ only in their
 * pictures.
 *
 * The first of the two is the move BtlOpenFxGrid stands over the acting
 * fighter's own side - it is the only one in g_btl_spell_fx with a step
 * handler of its own as well.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* The blue every fighter the move reaches is walked to. */
#define FX_LIT_R 0
#define FX_LIT_G 0x80
#define FX_LIT_B 0xFF

#define FX_SHEET_TABLE 0

BtlObj *BtlFxStartSheetLit(void)
{
    BtlTintTargets(g_btl_actor_turn, FX_LIT_R, FX_LIT_G, FX_LIT_B);
    return BtlOpenFxGrid(FX_SHEET_TABLE);
}

BtlObj *BtlFxStartSheetLit2(void)
{
    BtlTintTargets(g_btl_actor_turn, FX_LIT_R, FX_LIT_G, FX_LIT_B);
    return BtlOpenFxGrid(FX_SHEET_TABLE);
}
