/* Persona 1 (JP) - two small pieces of the negotiation.  BTLP only.
 *   0x80067630 BtlMoodBand  0x80067D74 BtlTalkEndEffect
 *   0x80074A74 BtlTalkMenuEndEffect
 *
 * The band boundaries sit between the two levels the mood gauges are read at -
 * 0x46 for the lower and 0x5F for the higher - so what this sorts is a value
 * already close to the top of its range.
 *
 * The effect calls are the ones the negotiation's endings all make in the
 * breath before they close the message box - one for the negotiation's own
 * effect and one for the command menu's, which the menu builder fills in.
 */
#include <decomp/types.h>

#define MOOD_BAND_LOW  0x58
#define MOOD_BAND_HIGH 0x5D

/* The kind the negotiation's effect is left on. */
#define TALK_EFFECT_END 6

extern int g_btl_talk_effect;
extern int g_btl_talk_menu_effect;

extern void BtlEffectSetKind(int slot, u_char kind);
/* Defined in the unit before this one; the prototype is what
   decides how the arguments are converted. */
extern int BtlMoodBand(int value);


void BtlTalkEndEffect(void)
{
    BtlEffectSetKind(g_btl_talk_effect, TALK_EFFECT_END);
}
