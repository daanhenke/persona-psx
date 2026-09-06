/* Persona 1 (JP) - noting a Persona the battle has awarded.  BTLP only.
 *   0x800976B0 BtlNotePersonaWon
 *
 * One bit to a Persona id, set immediately after BtlStockAdd puts the Persona
 * in the stock, so the tally survives however many the fight hands out. The
 * id is treated as signed - the division by 32 carries the bias gcc adds for a
 * negative numerator - even though nothing ever passes one.
 */
#include <types.h>

#define PERSONA_WON_BITS 32

extern u_long g_btl_persona_won[];

void BtlNotePersonaWon(int id)
{
    g_btl_persona_won[id / PERSONA_WON_BITS] |= 1 << (id % PERSONA_WON_BITS);
}
