#ifndef PERSONA_BTLP_HUD_H
#define PERSONA_BTLP_HUD_H

/* Persona 1 (JP) - whether the status HUD is still moving.
 *
 * hud.c defines BtlHudState to answer an int, but every caller in the image
 * tests only the low byte of the answer - an andi 0xFF straight after the call
 * - which is what a char answer gives them, and a cast at the call does not.
 * This is the callers' view of it; hud.c does not include it.
 */
#include <decomp/types.h>

extern char BtlHudState(void);

#endif
