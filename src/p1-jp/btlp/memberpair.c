/* Persona 1 (JP) - the per-member answer a negotiation opens with.  BTLP only.
 *   0x80073D1C BtlMemberPairUnlisted   0x80073D7C BtlSetMemberAnswers
 *
 * A party member's record carries a two-byte code at +1 and the answer to it
 * at +4. The answer is a lookup against a fixed table of eighteen codes: zero
 * when the code is one of them, one when it is not. A table entry whose second
 * byte is 0xFF matches whatever second byte the code has, so four of the
 * eighteen pin both bytes and the rest pin only the first.
 *
 * The first bytes of the table are all distinct, which is why the search can
 * step past a half-matched entry without backing the code pointer up.
 *
 * BtlSetMemberAnswers runs the lookup over all five members at the top of a
 * negotiation; BtlMarkMembersMatched turns the results into a bitmask three
 * calls later.
 */
#include <types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/member.h>

#define BTL_MEMBER_PAIRS 18

/* A table entry's second byte, when any second byte will do. */
#define BTL_PAIR_ANY 0xFF

extern const u_char g_btl_member_pairs[];

int BtlMemberPairUnlisted(const u_char *pair)
{
    const u_char *code;
    int           i;

    code = g_btl_member_pairs;
    i = 0;
    while (i < BTL_MEMBER_PAIRS) {
        if (*pair == *code++) {
            pair++;
            if (*pair == *code || *code == BTL_PAIR_ANY) {
                break;
            }
        }
        i++;
        code++;
    }
    return i == BTL_MEMBER_PAIRS;
}

void BtlSetMemberAnswers(void)
{
    BtlMember *m;
    int        i;

    m = g_btl_member;
    i = 0;
    do {
        m->answer = 0;
        if (m->key != 0) {
            m->answer = BtlMemberPairUnlisted(m->pair);
        }
        i++;
        m++;
    } while (i < BTL_PARTY);
}
