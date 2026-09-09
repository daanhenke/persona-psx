/* Persona 1 (JP) - the four mood gauges, ready to draw.  BTLP only.
 *   0x800753CC BtlSetMoodGauges
 *
 * The negotiation keeps each mood as a number the two thresholds are read at -
 * 0x46 for the lower and 0x5F for the higher - and the gauges are drawn in the
 * same 0x1000 the rest of the overlay scales by. Forty-three units a point is
 * what joins the two: 43 by 95 is 4085, a hair under full, so a gauge that has
 * gone all the way still leaves the last sliver of the bar unlit.
 *
 * All four are handed over at once because they are always redrawn together.
 */
#include <decomp/types.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/battle.h>

/* Gauge units a point of mood is worth. */
#define MOOD_SCALE 0x2B

extern short g_btl_mood_gauge[];
void BtlRefreshMoodGauges(void)
{
    short *mood;
    int    i;

    i = 0;
    mood = g_btl_offer[g_btl_offer_slot].mood;
    do {
        i++;
        if (*mood > BTL_MOOD_STRONG) {
            *mood = BTL_MOOD_STRONG;
        }
        mood++;
    } while (i < BTL_MOODS);
    BtlSetMoodGauges(g_btl_offer[g_btl_offer_slot].mood[0],
                     g_btl_offer[g_btl_offer_slot].mood[1],
                     g_btl_offer[g_btl_offer_slot].mood[2],
                     g_btl_offer[g_btl_offer_slot].mood[3]);
}
