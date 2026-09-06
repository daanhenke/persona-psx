/* Persona 1 (JP) - keeping a sum a round figure.  BTLP only.
 *   0x80066BB0 BtlRoundMoney
 *
 * Both callers hand this money: one checks the result against what the party
 * has, the other builds a reward out of rand(). Anything over a hundred is cut
 * back to a whole hundred, so the numbers the battle announces read as sums
 * rather than as die rolls - unless the offer in hand carries the bit that says
 * to keep the exact figure.
 */
#include <types.h>
#include <persona/btlp/offer.h>

#define MONEY_ROUND 100

/* Set, the sum is left exactly as it came. */
#define BTL_OFFER_EXACT 2

extern short g_btl_offer_slot;

int BtlRoundMoney(int amount)
{
    if ((g_btl_offer[g_btl_offer_slot].flags & BTL_OFFER_EXACT) == 0 &&
        amount > MONEY_ROUND) {
        amount = (amount / MONEY_ROUND) * MONEY_ROUND;
    }
    return amount;
}
