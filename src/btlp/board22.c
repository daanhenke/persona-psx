/* Persona 1 (JP) - the board pair numbered 0x22.  BTLP only.
 *   0x800A955C BtlOpenBoard22  0x800A9594 BtlCloseBoard22
 *
 * The plainest pair in the family: a table of four records and a position out
 * of the data segment rather than off the stack, so the open is one call and
 * one store. Named after the index its window record carries.
 */
#include <decomp/types.h>
#include <persona/btlp/board.h>
#include <persona/btlp/object.h>

extern BtlObj *g_btl_board22;

extern const BtlBoardDef g_btl_board22_defs[];
extern const long        g_btl_board22_pos[];

void BtlOpenBoard22(void)
{
    g_btl_board22 = BtlBoardOpen(g_btl_board22_defs, g_btl_board22_pos);
}

void BtlCloseBoard22(void)
{
    BtlBoardShut(g_btl_board22);
}
