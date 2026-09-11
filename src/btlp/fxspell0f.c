/* Persona 1 (JP) - a grid over the field, and the same handler again with
 * nothing that reaches it.  BTLP only.
 *   0x800B7DEC BtlFxStart0F  0x800B7E0C BtlFxStartUnused
 *
 * Two start handlers, of which only the first has a record in the table.
 * Nothing in the overlay reaches the second - no call, no jump, and no word
 * anywhere in BTLP.BIN holding its address - so it is here because the
 * assembler laid it down between two that are used, the way four of the fixed
 * boards' pairs are. A move it was written for was presumably cut.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* Which of the staged script tables the grid is built from. */
#define FX_GRID_TABLE 0

BtlObj *BtlFxStart0F(void)
{
    return BtlOpenFxGrid(FX_GRID_TABLE);
}

BtlObj *BtlFxStartUnused(void)
{
    return BtlOpenFxGrid(FX_GRID_TABLE);
}
