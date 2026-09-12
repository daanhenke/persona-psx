/* Persona 1 (JP) - the last few values used.  BTLP only.
 *   0x80066F64 BtlPushRecent  0x80067094 BtlRecentOther
 *
 * Four entries, most recent first, with a fifth slot after them holding the
 * terminator. Pushing shifts everything down as far as the first empty slot,
 * so a list that has not filled up yet does not shift the empties around.
 *
 * The lookup skips both empty slots and the value it was asked about, which
 * makes it "the most recent one that was not this"; its caller turns the
 * answer into a bit number, so the values stored here are small.
 */
#include <decomp/types.h>
#include <persona/btlp/offer.h>

#define BTL_RECENT 4
#define BTL_RECENT_NONE (-1)

extern int g_btl_recent[];

void BtlPushRecent(int value)
{
    int *base;
    int *src;
    int *dst;
    int *p;
    int  n;
    int  none;
    int  i;
    int  end;
    int  one;

    n = 0;
    none = BTL_RECENT_NONE;
    p = g_btl_recent;
    while (n < BTL_RECENT) {
        if (*p == none) {
            break;
        }
        n++;
        p++;
    }
    i = n;
    end = BTL_RECENT_NONE;
    if (i > 0) {
        base = g_btl_recent;
        /* The step back through a variable rather than a literal 1 is what
           puts the two walking pointers in the registers the original uses. */
        one = 1;
        dst = &base[i];
        src = &(base - one)[i];
        do {
            *dst = *src;
            src--;
            i--;
            dst--;
        } while (i > 0);
        end = BTL_RECENT_NONE;
    }
    g_btl_recent[0] = value;
    g_btl_recent[BTL_RECENT] = end;
}

void BtlDropRecent(int value)
{
    int *scan;
    int *base;
    int *dst;
    int  none;
    int  gone;
    int  i;
    int  j;

    i = 0;
    gone = BTL_RECENT_NONE;
    scan = g_btl_recent;
    while (i < BTL_RECENT) {
        if (*scan == value) {
            *scan = gone;
            break;
        }
        i++;
        scan++;
    }
    i = 0;
    none = BTL_RECENT_NONE;
    base = g_btl_recent;
    for (; i < BTL_RECENT; i++) {
        if (base[i] == none) {
            for (j = i; j < BTL_RECENT; j++) {
                dst = &base[i];
                if (base[j] != none) {
                    *dst = base[j];
                    base[j] = none;
                }
            }
        }
    }
}


int BtlRecentOther(int value)
{
    int *p;
    int  i;
    int  none;
    int  found;

    i = 0;
    none = BTL_RECENT_NONE;
    p = g_btl_recent;
    while (i < BTL_RECENT) {
        if (*p != none) {
            if (*p != value) {
                break;
            }
        }
        i++;
        p++;
    }
    if (i != BTL_RECENT) {
        found = g_btl_recent[i];
    } else {
        found = BTL_RECENT_NONE;
    }
    return found;
}

/* Why the negotiation cannot go on, or zero if it can: 1 when no party member
   has anyone willing to answer them, 2 when no offer still holds a demon in a
   state to talk. Nothing calls it - the two masks it reads are tested
   separately where the answer is wanted. */
int BtlTalkBlocked(void)
{
    int reason;

    if (g_btl_member_matched == 0) {
        reason = 1;
    } else {
        reason = (g_btl_offer_live == 0) << 1;
    }
    return reason;
}
