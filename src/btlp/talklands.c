/* Persona 1 (JP) - did the contact line get through?  BTLP only.
 *   0x80067128 BtlTalkLineLands
 *
 * A demon is talked round by moving four moods. The party member picks one of
 * four contact options, and g_btl_talk_lines says what that option is worth:
 * five records per block - a topic id and a weight for each of the four moods -
 * with eight blocks per character, two per option. Which of the two the demon
 * hears is bit 8 of the offer's flags, and the pair holds the same four topics
 * in a different order.
 *
 * The mood being worked on is g_btl_talk_said. If the option carries no weight
 * for it the line cannot land at all; otherwise it is a coin weighted by
 * g_btl_talk_step, which BtlTalkScoreLine leaves at 4 for an ordinary line and
 * raises to 6 for a strong one the demon has just heard - so a repeat is worth
 * six chances in eight rather than four.
 *
 * The negotiation reads the answer once, at step 0xB: a line that lands makes
 * the demon reply out of the mood it raised, and one that does not leaves the
 * demon to pick a reply of its own. Both counters are put back here, so a
 * spoken line is scored exactly once however long the exchange runs.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/talk.h>

/* One contact line. The topic is what the party member says; the weights are
   how far each of the four moods moves if it lands. */
typedef struct {
    /* 0x0 */ u_char topic;
    /* 0x1 */ u_char mood[BTL_MOODS];
} BtlTalkLine;                      /* 5 bytes */

/* How the blocks are laid out, in records. A character owns eight blocks of
   five lines, an option two of them, and the offer's flag picks which. */
#define TALK_LINES_PER_CHAR   40
#define TALK_LINES_PER_OPTION 10
#define TALK_LINES_PER_BLOCK  5

/* Char.key counts from one, so the block for the first character starts a
   whole character's worth into the table and the first 200 bytes are dead. */
extern const BtlTalkLine g_btl_talk_lines[];

/* g_btl_talk_result, as BtlTalkScoreLine sets it. */
#define TALK_TOO_WEAK 1
#define TALK_NEW      2
#define TALK_REPEATED 4

/* Bit 8 of BtlOffer.flags: the demon hears the second of the two orderings. */
#define TALK_ALT_SHIFT 8

/* A line that lands on a repeat is a coin out of eight; the odds for a fresh
   one are whatever the scorer left in g_btl_talk_step. */
#define TALK_FRESH_ODDS 4
#define TALK_ODDS_OUT_OF 8

/* What the counters are put back to. */
#define TALK_STEP_IDLE 4

extern short    g_btl_talk_picked;

int BtlTalkLineLands(void)
{
    const BtlTalkLine *lines;
    u_int   flags;
    u_short result;
    short   step;
    int     alt;

    result = g_btl_talk_result;
    step   = g_btl_talk_step;
    lines  = g_btl_talk_lines;
    g_btl_talk_result = 0;
    g_btl_talk_step = TALK_STEP_IDLE;

    /* The mask is applied where the block is picked, not here. */
    flags = g_btl_offer[g_btl_offer_slot].flags;
    alt = flags >> TALK_ALT_SHIFT;

    if (lines[g_btl_actors[g_btl_actor_slot].c.key * TALK_LINES_PER_CHAR
              + g_btl_talk_picked * TALK_LINES_PER_OPTION
              + (alt & 1) * TALK_LINES_PER_BLOCK].mood[g_btl_talk_said] != 0) {
        if ((result & TALK_TOO_WEAK) == 0) {
            if ((result & TALK_NEW) != 0 || g_btl_talk_line == g_btl_talk_last_line) {
                goto weighted;
            }
            if ((result & TALK_REPEATED) != 0
                && rand() % TALK_ODDS_OUT_OF < TALK_FRESH_ODDS) {
                return 1;
            }
        }
    }
    return 0;

weighted:
    return rand() % TALK_ODDS_OUT_OF < step;
}
