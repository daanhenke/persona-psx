/* Persona 1 (JP) - crediting a contact to the offer it belongs to.  BTLP only.
 *   0x80074A9C BtlOfferScoreEnemy
 *
 * An offer covers a set of enemies, one bit per enemy slot in `used`, so the
 * first offer whose bit is set for this enemy is the one that scores. The
 * amount goes two places: the round's running total for that offer, which
 * BtlOfferApplyScores folds into the moods and then clears, and the offer's
 * own unk20, which is kept for the whole fight.
 *
 * Nothing stops the search running off the end. An enemy no offer claims
 * leaves the index at BTL_OFFERS and both writes land one record past the
 * array; it does not happen, because every enemy that can be talked to is in
 * an offer by the time this is reached.
 */
#include <decomp/types.h>
#include <persona/btlp/offer.h>

void BtlOfferScoreEnemy(int enemy, int amount)
{
    int i;

    for (i = 0; i < BTL_OFFERS; i++) {
        if ((g_btl_offer[i].used >> enemy & 1) != 0) {
            break;
        }
    }

    g_btl_talk_scratch[i] += amount;
    g_btl_offer[i].damage += amount;
}
