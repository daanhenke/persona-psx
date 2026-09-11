/* Persona 1 (JP) - eight moves at the front of the effect table.  BTLP only.
 *   0x800B8A58 BtlFxStart1F  0x800B8A80 BtlFxStart20  0x800B8AA0 BtlFxStart21
 *   0x800B8AC0 BtlFxStart22  0x800B8AE0 BtlFxStart23  0x800B8B00 BtlFxStart24
 *   0x800B8B20 BtlFxStart25  0x800B8B48 BtlFxStart26
 *
 * Eight start handlers out of g_btl_spell_fx. Four shapes between them: a grid
 * over the whole field, a single record on the fighter aimed at, and two that
 * hand their frame straight to a handler earlier in the table - which is how a
 * move borrows another's effect outright rather than being given its record in
 * the table, and the borrowed handler is reached by a call, so the id the
 * effect is drawn under stays the borrower's.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* None of the eight staggers its records. */
#define FX_TIMER 0

/* Which of the staged script tables the grid is built from. */
#define FX_GRID_TABLE 0

BtlObj *BtlFxStart1F(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}

BtlObj *BtlFxStart20(void)
{
    return BtlFxStart19();
}

BtlObj *BtlFxStart21(void)
{
    return BtlOpenFxGrid(FX_GRID_TABLE);
}

BtlObj *BtlFxStart22(void)
{
    return BtlOpenFxGrid(FX_GRID_TABLE);
}

BtlObj *BtlFxStart23(void)
{
    return BtlFxStart1C();
}

BtlObj *BtlFxStart24(void)
{
    return BtlOpenFxGrid(FX_GRID_TABLE);
}

BtlObj *BtlFxStart25(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}

BtlObj *BtlFxStart26(void)
{
    return BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
}
