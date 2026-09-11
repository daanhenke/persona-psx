/* Persona 1 (JP) - a borrowed effect and a grid.  BTLP only.
 *   0x800B8E8C BtlFxStart28  0x800B8EAC BtlFxStart29
 *
 * Two start handlers out of g_btl_spell_fx: the first borrows move 0x10's
 * handler, which move 0x11 borrows as well, and the second opens a grid over
 * the whole field.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* Which of the staged script tables the grid is built from. */
#define FX_GRID_TABLE 0

BtlObj *BtlFxStart28(void)
{
    return BtlFxStart10();
}

BtlObj *BtlFxStart29(void)
{
    return BtlOpenFxGrid(FX_GRID_TABLE);
}
