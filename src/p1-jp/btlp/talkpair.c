/* Persona 1 (JP) - which reaction a pair of talk acts earns.  BTLP only.
 *   0x8006E4A4 BtlTalkPairIndex
 *
 * Negotiation collects one bit per talk act the party used this round, and the
 * demon's reaction is chosen by the *combination* rather than by either act on
 * its own. This turns the mask into the index of the unordered pair, counting
 * the two lowest bits set:
 *
 *   0  act 0 alone      4  act 1 alone      7  act 2 alone      9  act 3 alone
 *   1  acts 0 and 1     5  acts 1 and 2     8  acts 2 and 3
 *   2  acts 0 and 2     6  acts 1 and 3
 *   3  acts 0 and 3
 *
 * which is the ten rows of the reaction table the caller then reads - each
 * holding a mood shift and the pair of message ids the demon answers with.
 * A mask with no bits set at all runs off the end of the bases and is caught
 * by the clamp, so it reads as the first row rather than out of the table.
 */
#include <types.h>

/* Talk acts, and therefore bits of the mask, per round. */
#define BTL_TALK_ACTS 4

/* Rows in the reaction table: the pairs of four acts, with repetition. */
#define BTL_TALK_PAIRS 10

int BtlTalkPairIndex(u_int acts)
{
    /* Where each act's run of pairs starts: 4 pairs begin at act 0, then 3, 2
       and 1 as the partner can only be a higher act. */
    int base[BTL_TALK_ACTS] = { 0, 4, 7, 9 };
    int first;
    int act;
    int start;
    int above;
    int partner;
    int pair;

    first = 0;
    while (first < BTL_TALK_ACTS) {
        partner = 0;
        if ((acts & (1 << first)) != 0) {
            break;
        }
        first++;
    }

    pair = base[first];
    start = first + 1;
    for (act = start; act < BTL_TALK_ACTS; act++) {
        above = act - start;
        if ((acts & (1 << act)) != 0) {
            partner = above + 1;
            break;
        }
    }

    pair += partner;
    if (pair >= BTL_TALK_PAIRS) {
        pair = 0;
    }
    return pair;
}
