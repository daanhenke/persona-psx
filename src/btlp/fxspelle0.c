/* Persona 1 (JP) - ten of the last moves in the effect table.  BTLP only.
 *   0x800BF628 BtlFxStartE0  0x800BF650 BtlFxStartE6  0x800BF670 BtlFxStartE9
 *   0x800BF690 BtlFxStartEA  0x800BF6B8 BtlFxStartEB  0x800BF6D8 BtlFxStartEE
 *   0x800BF6F8 BtlFxStartEF  0x800BF720 BtlFxStartF0  0x800BF740 BtlFxStartF2
 *   0x800BF760 BtlFxStartE4
 *
 * Ten start handlers out of g_btl_spell_fx. Six are a grid over the whole
 * field, three a single record on the fighter aimed at, and the first is the
 * same record put on the acting fighter instead - the only handler in the table
 * that reads g_btl_actor_turn rather than g_btl_fx_target, because the move is
 * cast on the caster.
 *
 * Their ids do not run in order here. The image lays them out as the assembler
 * met them, and 0xE4's handler was written last even though its record sits
 * earlier in the table; the file keeps the image's order, not the table's.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

#define FX_TIMER      0
#define FX_GRID_TABLE 0

BtlObj *BtlFxStartE0(void)
{
    return BtlOpenFxObj2(g_btl_actor_turn, FX_TIMER);
}

BtlObj *BtlFxStartE6(void)
{
    return BtlOpenFxGrid(FX_GRID_TABLE);
}

BtlObj *BtlFxStartE9(void)
{
    return BtlOpenFxGrid(FX_GRID_TABLE);
}

BtlObj *BtlFxStartEA(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}

BtlObj *BtlFxStartEB(void)
{
    return BtlOpenFxGrid(FX_GRID_TABLE);
}

BtlObj *BtlFxStartEE(void)
{
    return BtlOpenFxGrid(FX_GRID_TABLE);
}

BtlObj *BtlFxStartEF(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}

BtlObj *BtlFxStartF0(void)
{
    return BtlOpenFxGrid(FX_GRID_TABLE);
}

BtlObj *BtlFxStartF2(void)
{
    return BtlOpenFxGrid(FX_GRID_TABLE);
}

BtlObj *BtlFxStartE4(void)
{
    return BtlOpenFxGrid(FX_GRID_TABLE);
}
