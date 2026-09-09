/* Persona 1 (JP) - each side's average numbers.
 *
 * BtlAverageSides works these out once as the battle opens and nothing puts
 * them back afterwards, so they describe the fight as it started rather than
 * as it stands. The rolls that weigh one side against the other read them.
 *
 * A count is how many slots the average was taken over, and is the divisor
 * the hit roll spreads its headroom across, so it is never zero while there
 * is anyone left to swing.
 */
#ifndef PERSONA_BTLP_SIDES_H
#define PERSONA_BTLP_SIDES_H

#include <decomp/types.h>

extern int g_btl_party_level;
extern int g_btl_party_agility;
extern int g_btl_party_luck;
extern int g_btl_party_counted;

extern int g_btl_enemy_level;
extern int g_btl_enemy_agility;
extern int g_btl_enemy_luck;
extern int g_btl_enemy_counted;

extern void BtlAverageSides(void);

#endif
