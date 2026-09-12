/* Persona 1 (JP) - two more of the battle's fixed boards.  BTLP only.
 *   0x800A9440 BtlCloseBoard1F  0x800A9468 BtlOpenBoard1D
 *   0x800A94A0 BtlCloseBoard1D
 *
 * The same two calls as every pair in boards.c, and named the same way: after
 * the index the window record takes, which is the picture the board is drawn
 * from. Board 0x1F's open is not here - it is the tail of the routine that
 * fills the board in, further back in the block - so only its close stands with
 * the pair that follows it.
 *
 * Board 0x1D is the only one of the family that does not stand where the rest
 * do: its position table puts it higher and well to the left.
 */
#include <decomp/types.h>
#include <persona/btlp/board.h>
#include <persona/btlp/object.h>

extern BtlObj *g_btl_board1D;
extern BtlObj *g_btl_board1F;

extern const BtlBoardDef g_btl_board1D_defs[];
extern const long        g_btl_board1D_pos[];

void BtlCloseBoard1F(void)
{
    BtlBoardShut(g_btl_board1F);
}

void BtlOpenBoard1D(void)
{
    g_btl_board1D = BtlBoardOpen(g_btl_board1D_defs, g_btl_board1D_pos);
}

void BtlCloseBoard1D(void)
{
    BtlBoardShut(g_btl_board1D);
}
