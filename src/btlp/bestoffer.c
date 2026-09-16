/* Persona 1 (JP) - one offer picked out of several. BTLP only.
 *   0x8006ED70 BtlBestOffer
 *
 * The two choosers hand this a mask of the offers that are allowed, and it
 * answers which one of them the party gets. One offer in the mask is that
 * offer; past that the mask is narrowed three times over, and each round stops
 * early when it left a single winner behind.
 *
 * The rounds are, in order: the four gauges added up, the offer's level, and
 * how far down g_btl_talk_kind_order the demon's own kind sits - the first
 * byte of its row in g_btl_demon_talk_profiles, which offeranswer.c matches a
 * carried Persona against. The first two narrow by striking the losers out of
 * the mask and counting the ties; a round that ends with no tie is the answer.
 * The last keeps no mask, since it has the last word either way.
 *
 * The order table is the twelve kinds in the order they are weighed, which in
 * this build is the order they are numbered in.
 */
#include <decomp/types.h>
#include <persona/btlp/offer.h>

/* How many kinds a demon's first byte can be. */
#define BTL_TALK_KINDS 12

u_char g_btl_talk_kind_order[BTL_TALK_KINDS] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

int BtlBestOffer(u_int offers)
{
    int best;
    int ties;
    int top;
    int score;
    int i;
    int j;
    int kind;

    ties = 0;
    i    = 0;
    while (i < BTL_OFFERS) {
        if ((offers & (1 << i)) != 0) {
            ties++;
            best = i;
        }
        i++;
    }
    if (ties == 1) {
        return best;
    }

    ties = 0;
    best = 0;
    top  = 0;
    i    = 0;
    while (i < BTL_OFFERS) {
        u_long bit = 1 << i;

        if ((offers & bit) != 0) {
            score = g_btl_offer[i].mood[0] + g_btl_offer[i].mood[1]
                    + g_btl_offer[i].mood[2] + g_btl_offer[i].mood[3];
            if (top < score) {
                top = score;
                offers &= ~(1 << best);
                best = i;
                offers |= bit;
            } else if (top == score) {
                ties++;
            } else {
                offers &= ~bit;
            }
        }
        i++;
    }
    if (ties == 0) {
        return best;
    }

    ties = 0;
    best = 0;
    top  = 0;
    i    = 0;
    while (i < BTL_OFFERS) {
        u_long bit = 1 << i;

        if ((offers & bit) != 0) {
            score = g_btl_offer[i].level;
            if (top < score) {
                top = score;
                offers &= ~(1 << best);
                best = i;
                offers |= bit;
            } else if (top == score) {
                ties++;
            } else {
                offers &= ~bit;
            }
        }
        i++;
    }
    if (ties == 0) {
        return best;
    }

    ties = 0;
    best = 0;
    i    = 0;
    while (i < BTL_OFFERS) {
        if ((offers & (1 << i)) != 0) {
            kind = g_btl_demon_talk_profiles[g_btl_offer[i].persona][0];
            for (j = 0; j < BTL_TALK_KINDS; j++) {
                if (g_btl_talk_kind_order[j] == kind) {
                    break;
                }
            }
            if (ties < j) {
                best = i;
                ties = j;
            }
        }
        i++;
    }
    return best;
}
