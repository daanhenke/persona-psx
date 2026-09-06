/* Persona 1 (JP) - scoring a line with its verb in hand.  BTLP only.
 *   0x80067658 BtlTalkTakeLine
 *
 * The other half of the negotiation's scoring. BtlTalkScoreLine reads a line
 * on its own terms; this one is used where the verb is known, and the verb
 * decides which measure is read. One of them is answered by the mood the
 * demon is in rather than by how hard the line lands, and it is the only one
 * that can reach step 7.
 *
 * The thresholds for the rest are the ones BtlTalkScoreLine uses: under ten
 * the line does not count at all, and from 0x28 it is strong.
 *
 * The first argument decides whose value is recorded as what was said - the
 * caller's, or a flat 3. The caller passes a slot index less two, so slot two
 * is the one that gets the flat value.
 */
#include <types.h>

/* The verb answered by the demon's mood instead of the line's weight. */
#define TALK_VERB_MOOD 3

/* What a line has to reach to count at all, and to count as strong. */
#define TALK_COUNTS 10
#define TALK_STRONG 0x28

/* Bits of g_btl_talk_result. */
#define TALK_TOO_WEAK 1
#define TALK_NEW      2

/* Where the negotiation is left. */
#define TALK_STEP_PLAIN  3
#define TALK_STEP_STRONG 4
#define TALK_STEP_BEST   7

extern short g_btl_talk_said;
extern short g_btl_talk_step;
extern short g_btl_talk_result;

extern int BtlMoodBand(short value);

int BtlTalkTakeLine(short slot, short verb, short said, short weight,
                    short mood)
{
    int band;

    if (verb != TALK_VERB_MOOD) {
        g_btl_talk_result |= TALK_NEW;
        if (slot == 0) {
            g_btl_talk_said = TALK_STEP_PLAIN;
            g_btl_talk_step = TALK_STEP_PLAIN;
            return 1;
        }
        if (weight < TALK_COUNTS) {
            g_btl_talk_result |= TALK_TOO_WEAK;
            return 0;
        }
        if (weight < TALK_STRONG) {
            g_btl_talk_step = TALK_STEP_PLAIN;
        } else {
            g_btl_talk_step = TALK_STEP_STRONG;
        }
        g_btl_talk_said = said;
        return 1;
    }

    g_btl_talk_result |= TALK_NEW;
    if (slot == 0) {
        g_btl_talk_said = TALK_STEP_PLAIN;
        band = BtlMoodBand(mood);
        if (band == 1) {
            g_btl_talk_step = TALK_STEP_STRONG;
        } else if (band == 2) {
            g_btl_talk_step = TALK_STEP_BEST;
        } else {
            g_btl_talk_step = TALK_STEP_PLAIN;
        }
        return 1;
    }
    g_btl_talk_said = said;
    band = BtlMoodBand(mood);
    if (band > 0 && band < 3) {
        g_btl_talk_step = TALK_STEP_PLAIN;
        return 1;
    }
    g_btl_talk_result |= TALK_TOO_WEAK;
    return 1;
}
