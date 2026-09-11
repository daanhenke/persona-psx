/* Persona 1 (JP) - the move whose effect is a sheet over a whole side.
 * BTLP only.
 *   0x800B71B0 BtlFxStartSheet
 *
 * A start handler out of g_btl_spell_fx. Everything the move reaches is put on
 * the effect's own colour first, and then the sheet is opened over the side -
 * so the fighters change colour under it rather than behind it.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* Which of the staged script tables the sheet's cells are drawn from. */
#define FX_SHEET_TABLE 0

BtlObj *BtlFxStartSheet(void)
{
    BtlTintTargets(g_btl_actor_turn, g_btl_tint_fx_r, g_btl_tint_fx_g,
                   g_btl_tint_fx_b);
    return BtlOpenFxGrid(FX_SHEET_TABLE);
}
