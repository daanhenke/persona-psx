/* Persona 1 (JP) - what a line of negotiation was worth.  BTLP only.
 *   0x800672B8 BtlTalkScoreLine
 *
 * A line carries what it is about and how strongly it lands. Under ten it does
 * not count at all; from ten it counts; from 0x28 it is strong, and then it
 * matters whether the demon has just heard the same thing - saying something
 * new leaves the negotiation where it is, and repeating a strong line moves it
 * on.
 *
 * The result is read as bits rather than a number, which is why the repeat
 * case hands back the whole set and everything else hands back nothing or one.
 */
#include <decomp/types.h>
#include <persona/btlp/talk.h>

/* What a line has to reach to count at all, and to count as strong. */
#define TALK_COUNTS 10
#define TALK_STRONG 0x28

/* g_btl_talk_result */
#define TALK_TOO_WEAK 1
#define TALK_NEW      2
#define TALK_REPEATED 4

/* The step a repeated strong line moves the negotiation to. */
#define TALK_STEP_REPEATED 6

u_short BtlTalkScoreLine(short said, short weight)
{
    short result;
    int   ok;

    if (g_btl_talk_said == said) {
        if (weight >= TALK_COUNTS) {
            if (weight < TALK_STRONG) {
                return 1;
            }
            g_btl_talk_result |= TALK_REPEATED;
            /* Both of these go through the same local the answer comes back
               in. Assigning the step straight, or storing the OR without
               landing it in `result` first, puts the work in the return
               register and costs a second copy of the constant 1 below. */
            result = TALK_STEP_REPEATED;
            g_btl_talk_step = result;
            result = g_btl_talk_result;
            return result;
        }
    } else if (weight >= TALK_COUNTS) {
        goto fresh;
    }
    g_btl_talk_result |= TALK_TOO_WEAK;
    return 0;
    /* Reached only from the test above. The jump forward is what puts the
       "too weak" tail between the two arms, where the original has it. */
fresh:
    /* The 1 is settled before the test so that both arms share it: the
       original has it in the branch's delay slot, where it lands on either
       path. Returning the constant from each arm gets it written twice. */
    ok = 1;
    if (weight < TALK_STRONG) {
        g_btl_talk_said = said;
    } else {
        /* Through a local, and read before the said store: that leaves the
           answer sitting in v0 across the arm, which is where the delay slot
           put it. Reading and writing the flags in place works in v0 and costs
           a second copy of the constant at the end. */
        do {
            result = g_btl_talk_result;
            g_btl_talk_said = said;
            result = result | TALK_NEW;
            g_btl_talk_result = result;
        } while (0);
    }
    return ok;
}

