/* Persona 1 (JP) - the close of the board numbered 0x23.  BTLP only.
 *   0x800A98A8 BtlCloseBoard23
 *
 * Its open is the tail of the routine in front of it, which fills the board in
 * before putting it up, so only the close stands on its own.
 */
#include <decomp/types.h>
#include <persona/btlp/board.h>
#include <persona/btlp/object.h>

extern BtlObj *g_btl_board23;

void BtlCloseBoard23(void)
{
    BtlBoardShut(g_btl_board23);
}
