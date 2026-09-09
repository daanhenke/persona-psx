/* Persona 1 (JP) - a round's damage turned into mood.  BTLP only.
 *   0x80074B28 BtlOfferApplyScores
 *
 * Run once as a negotiation resumes, over all three offers whether or not
 * they are the one being talked to. Whatever the party took off an offer's
 * demons since the last time round moves one of its gauges, and the running
 * total is cleared either way - an offer whose flags say the damage does not
 * count still loses it rather than carrying it forward.
 *
 * Which gauge it moves is the whole of the decision: hitting something that
 * can fight back makes it angry, and hitting something far weaker than the
 * party frightens it instead. The offer's own flag says so outright, and a
 * demon nine or more levels under the party is treated the same way.
 *
 * The damage is scaled against the demons' combined health, so a round that
 * halves a weak offer moves its gauge as far as one that barely scratches a
 * strong one.
 */
#include <decomp/types.h>
#include <persona/btlp/offer.h>

/* Which of BtlOfferLevelTest's comparisons frightens rather than angers. */
#define LEVEL_UNDER_M9 5

void BtlOfferApplyScores(void)
{
    BtlOffer *offer;
    short    *gauge;
    int      *score;
    int       value;
    int       i;

    offer = g_btl_offer;
    i = 0;
    score = g_btl_talk_scratch;
    do {
        if (offer->used != 0) {
            if ((offer->flags & OFFER_SCORED) != 0) {
                gauge = &offer->mood[MOOD_ANGRY];
                if ((offer->flags & OFFER_FEARFUL) != 0) {
                    gauge = &offer->mood[MOOD_AFRAID];
                }
                if (BtlOfferLevelTest(LEVEL_UNDER_M9, i) == 1) {
                    gauge = &offer->mood[MOOD_AFRAID];
                }
                value = *gauge + *score * BTL_MOOD_FULL / offer->hp;
                if (value > BTL_MOOD_STRONG) {
                    value = BTL_MOOD_STRONG;
                }
                *gauge = value;
            }
            *score = 0;
        }
        offer++;
        i++;
        score++;
    } while (i < BTL_OFFERS);
    BtlOfferMarkStrong();
}
