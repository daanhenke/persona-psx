/* Persona 1 (JP) - what an offer says to the Persona you carry.  BTLP only.
 *   0x8006EF90 BtlOfferAnswer
 *
 * Every Persona an offer can hand over has a record of its own: six kinds of
 * carried Persona it takes an interest in, ending at 0xFF, and the answer each
 * of them earns. The kind is matched against the speaker's Persona, and the
 * top bit of the wanted kind is what tells the two answers apart - set means
 * the offer is made good, which is the answer the negotiation follows up by
 * closing the panel and reading the hand-over script off the disc.
 *
 * A speaker whose Persona has no kind at all is answered with nothing, and so
 * is one whose kind none of the six wants; both still leave the seventh answer
 * - the one past the six - behind them.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/stats.h>

/* Wanted kinds a record holds, where they start, and where the answers do. */
#define OFFER_WANTS   6
#define OFFER_WANT_AT 8
#define OFFER_ANSWER  0xE

/* What ends the list, and the bit that marks the kind the offer is made good
   for. */
#define OFFER_WANT_END  0xFF
#define OFFER_WANT_GOOD 0x80
#define OFFER_WANT_KIND 0x7F

/* The two answers a match earns. */
#define OFFER_ANSWERED 1
#define OFFER_GIVEN    2

extern const u_char *g_btl_offer_answers[];

#ifdef NON_MATCHING
int BtlOfferAnswer(u_short slot, short offer_slot, u_int *out)
{
    const u_char     *record;
    const BtlStats *p;
    int               i;
    int               none;

    record = g_btl_offer_answers[g_btl_offer[offer_slot].persona];
    /* Zero through a variable of its own, set before the Persona is looked
       up: it is what keeps the two answers in the registers the original has
       them in. */
    none = 0;
    p = &g_btl_personas[g_btl_actors[slot].c.list[g_btl_actors[slot].c.entry]];
    i = 0;
    if (p->key == 0) {
        return 0;
    }
    do {
        if ((record + i)[OFFER_WANT_AT] != OFFER_WANT_END
            && ((record + i)[OFFER_WANT_AT] & OFFER_WANT_KIND) == p->key) {
            if (((record + i)[OFFER_WANT_AT] & OFFER_WANT_GOOD) != none) {
                *out = (record + i)[OFFER_ANSWER];
                return OFFER_GIVEN;
            }
            *out = (record + i)[OFFER_ANSWER];
            return OFFER_ANSWERED;
        }
        i++;
    } while (i < OFFER_WANTS);
    *out = (record + i)[OFFER_ANSWER];
    return 0;
}
#else
INCLUDE_ASM("btlp/nonmatchings/offeranswer", BtlOfferAnswer);
#endif

