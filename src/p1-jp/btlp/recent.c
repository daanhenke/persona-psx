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
#include <types.h>

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
        dst = &base[i];
        src = &base[i - 1];
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

/* Takes a value out and closes the gap behind it. The inner walk does not stop
   at the first entry it moves, so what ends up in a hole is the last non-empty
   entry after it rather than the first. */
void BtlDropRecent(int value)
{
    int *base;
    int *p;
    int *q;
    int *dst;
    int  none;
    int  i;
    int  j;

    i = 0;
    p = g_btl_recent;
    do {
        i++;
        if (*p == value) {
            *p = BTL_RECENT_NONE;
            break;
        }
        p++;
    } while (i < BTL_RECENT);
    i = 0;
    none = BTL_RECENT_NONE;
    base = g_btl_recent;
    p = base;
    do {
        if (*p == none && i < BTL_RECENT) {
            q = base + i;
            dst = p;
            j = i;
            do {
                j++;
                if (*q != none) {
                    *dst = *q;
                    *q = none;
                }
                q++;
            } while (j < BTL_RECENT);
        }
        i++;
        p++;
    } while (i < BTL_RECENT);
}
