/* Persona 1 (JP) - slipping a turn into the round's order.  BTLP only.
 *   0x80097900 BtlInsertTurn
 *
 * Everything after the turn being played is shifted up one place and the new
 * fighter dropped into the gap, so an interruption acts next rather than
 * whenever its own initiative would have brought it round.
 */
#include <decomp/types.h>

/* round.h is deliberately not included: the turn is signed here, and the
   image says so - it reads it with lb at both tests, where the u_char form
   every other unit agrees on would give lbu. */
extern signed char g_btl_turn;
extern u_char      g_btl_turns;
extern u_char      g_btl_turn_order[];


void BtlInsertTurn(int actor)
{
    int i;

    for (i = g_btl_turns - 1; i > g_btl_turn; i--) {
        g_btl_turn_order[i + 1] = g_btl_turn_order[i];
    }
    g_btl_turn_order[g_btl_turn + 1] = actor;
    g_btl_turns++;
}
