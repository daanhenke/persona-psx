/* Persona 1 (JP) - which offer the party is allowed to make.  BTLP only.
 *   0x8006E310 BtlChooseOffer
 *
 * The three offers are scored first, then gathered into a mask: an offer
 * counts if some enemy is party to it and it ranks at all - the rank is the
 * lowest set bit of `kinds`, so an offer with none of the low four set has no
 * rank and is passed over.
 *
 * What is gathered is then put through three refusals, each of which throws
 * the whole choice away rather than dropping the one offer that failed:
 * anything the party already holds the stock of, an offer the round has not
 * scored, and anything the level test turns down. Only a mask that survives
 * all three is handed on to be chosen between.
 *
 * An empty mask and a refused one answer the same -1, so the caller cannot
 * tell "nothing to offer" from "nothing allowed" - and does not need to.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/offer.h>

/* How far up `kinds` a rank can sit. */
#define OFFER_KINDS 4

extern int BtlStockHolds(const BtlOffer *offer);
extern int func_8006ED70(u_int offers);

/* The for-loop keeps the first rank test inside the loop. Giving the counter
   initialization its own block preserves the original register allocation. */
int BtlChooseOffer(void)
{
    u_int mask;
    u_long bit;
    u_long kinds;
    int   i;
    int   kind;

    BtlOfferMarkStrong();
    mask = 0;
    i    = 0;
    do {
        if (g_btl_offer[i].used != 0) {
            do {
                kind = 0;
            } while (0);
            bit = 1;
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
            if ((mask & (1 << i)) != 0 && BtlOfferLevelTest(1, i) != 0) {
                return -1;
            }
            i++;
        } while (i < BTL_OFFERS);

        return func_8006ED70(mask);
    }
    return -1;
}
