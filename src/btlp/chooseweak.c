/* Persona 1 (JP) - which offer the party may make at the lower rank. BTLP only.
 *   0x8006E924 BtlChooseWeakOffer
 *
 * The twin of BtlChooseOffer, and the same routine line for line: the offers
 * are refreshed, gathered into a mask, put through the three refusals and
 * handed to BtlBestOffer. What differs is the level it reads them at. Where
 * BtlChooseOffer refreshes the strong gauges and looks for a rank in bits 0..3
 * of `kinds`, this one refreshes the weak gauges and looks in bits 4..7, and
 * the level test it asks is the second rather than the first.
 *
 * Nothing in the overlay calls it; BtlChooseOffer is what the negotiation
 * reaches for.
 */
#include <decomp/types.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/offer.h>

/* How far up `kinds` a rank can sit, and where the weak ranks start. */
#define OFFER_KINDS      4
#define OFFER_WEAK_FIRST 0x10

/* The level test asks about the second rank here, where BtlChooseOffer asks
   about the first. */
#define OFFER_TEST_WEAK 2

/* The for-loop keeps the first rank test inside the loop. Giving the counter
   initialization its own block preserves the original register allocation. */
int BtlChooseWeakOffer(void)
{
    u_int mask;
    u_long bit;
    u_long kinds;
    int   i;
    int   kind;

    BtlOfferMarkWeak();
    mask = 0;
    i    = 0;
    do {
        if (g_btl_offer[i].used != 0) {
            do {
                kind = 0;
            } while (0);
            bit = OFFER_WEAK_FIRST;
            kinds = g_btl_offer[i].kinds;
            for (; kind < OFFER_KINDS; kind++, bit <<= 1) {
                if ((bit & kinds) != 0) {
                    break;
                }
            }
            if (kind != OFFER_KINDS) {
                mask |= 1 << i;
            }
        }
        i++;
    } while (i < BTL_OFFERS);

    if (mask != 0) {
        i = 0;
        do {
            if ((mask & (1 << i)) != 0 && BtlStockHolds(&g_btl_offer[i]) != 0) {
                return -1;
            }
            i++;
        } while (i < BTL_OFFERS);

        i = 0;
        do {
            if ((mask & (1 << i)) != 0
                && (g_btl_offer[g_btl_offer_slot].flags & OFFER_SCORED) == 0) {
                return -1;
            }
            i++;
        } while (i < BTL_OFFERS);

        i = 0;
        do {
            if ((mask & (1 << i)) != 0
                && BtlOfferLevelTest(OFFER_TEST_WEAK, i) != 0) {
                return -1;
            }
            i++;
        } while (i < BTL_OFFERS);

        return BtlBestOffer(mask);
    }
    return -1;
}
