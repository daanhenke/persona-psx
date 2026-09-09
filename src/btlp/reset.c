/* Persona 1 (JP) - putting both sides back to standing.  BTLP only.
 *   0x800679F4 BtlEnemiesReset
 *   0x80067A14 BtlPartyReset
 *
 * The two workers live at the far end of the overlay with the rest of the
 * object code; these are the battle flow's way in to them. Every caller runs
 * the pair and then writes the next phase, so together they are what ends a
 * turn's animation: each fighter's palette goes back to the master copy, the
 * tint back to neutral, the idle script back on, and a party member who is
 * down or out of the fight is hidden rather than shown standing.
 */
#include <decomp/types.h>

extern void BtlEnemiesResetGfx(void);
extern void BtlPartyResetGfx(void);

void BtlEnemiesReset(void)
{
    BtlEnemiesResetGfx();
}

void BtlPartyReset(void)
{
    BtlPartyResetGfx();
}
