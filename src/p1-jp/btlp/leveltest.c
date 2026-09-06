/* Persona 1 (JP) - how the demon's level compares.  BTLP only.
 *   0x800742C0 BtlOfferLevelTest
 *
 * Every offer carries a level, and g_btl_talk_level is what it is measured
 * against. Six named comparisons of the difference are what the negotiation
 * picks its answers from - whether the demon will listen at all, whether it
 * will take a small bribe, whether it is far enough above the party to walk
 * away.
 *
 * The difference is left in g_btl_level_gap whichever test was asked for, so a
 * caller that wants the number rather than the answer reads it afterwards.
 */
#include <types.h>
#include <persona/btlp/offer.h>

/* Which comparison to make. */
#define LEVEL_AT_OR_BELOW  0
#define LEVEL_UNDER_4      1
#define LEVEL_UNDER_6      2
#define LEVEL_UNDER_M4     3
#define LEVEL_6_OR_MORE    4
#define LEVEL_UNDER_M9     5

extern int   g_btl_level_gap;
extern u_short g_btl_talk_level;

int BtlOfferLevelTest(int test, u_short slot)
{
    int ok;
    /* The original reserves sixteen bytes of stack it never touches. Without
       them the frame comes out a different size. */
    int spare[4];

    g_btl_level_gap = g_btl_offer[slot].level - g_btl_talk_level;
    switch (test) {
    case LEVEL_AT_OR_BELOW:
        if (g_btl_level_gap > 0) {
            goto no;
        }
        return 1;
    case LEVEL_UNDER_4:
        ok = g_btl_level_gap < 4;
        break;
    case LEVEL_UNDER_6:
        ok = g_btl_level_gap < 6;
        break;
    case LEVEL_UNDER_M4:
        ok = g_btl_level_gap < -4;
        break;
    case LEVEL_6_OR_MORE:
        if (g_btl_level_gap < 6) {
            goto no;
        }
        return 1;
    case LEVEL_UNDER_M9:
        ok = g_btl_level_gap < -9;
        break;
    default:
        goto no;
    }
    if (ok) {
        return 1;
    }
no:
    return 0;
}
