/* Persona 1 (JP) - emptying a negotiation.  BTLP only.
 *   0x80069258 BtlClearMemberLines
 *
 * Run when a battle starts, right after the member and offer records are
 * zeroed: zero is a real slot number, so every place a line can go has to be
 * set to 0xFFFF instead.
 */
#include <types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/member.h>

void BtlClearMemberLines(void)
{
    BtlMember *m;
    short     *group;
    short     *chosen;
    short     *line;
    int        i;
    int        g;
    int        k;

    m = g_btl_member;
    i = 0;
    do {
        group = (short *)m;
        g = 0;
        chosen = (short *)m;
        do {
            chosen[BTL_MEMBER_CHOSEN] = BTL_NO_LINE;
            k = BTL_MEMBER_LINES - 1;
            line = &group[BTL_MEMBER_LAST];
            do {
                *line = BTL_NO_LINE;
                k--;
                line--;
            } while (k >= 0);
            group += BTL_MEMBER_LINES;
            g += BTL_MEMBER_GROUP_BYTES;
            chosen++;
        } while (g < BTL_MEMBER_LINE_BYTES);
        m++;
        i++;
    } while (i < BTL_PARTY);
}
