/* Persona 1 (JP) - asking for a demon's voice line.  BTLP only.
 *   0x8006BFC4 BtlQueueVoice
 *
 * Four halfwords that BtlUpdateVoices picks up on its next pass: what sort of
 * line, which line, and which enemies it is meant for. The last is the offer's
 * own set, so a line asked for during a negotiation reaches everyone taking
 * part in it rather than one demon.
 *
 * The step goes back to zero here, which is what makes a second request cut
 * the first one short.
 */
#include <types.h>
#include <persona/btlp/offer.h>

#define VOICE_PLAIN 1
#define VOICE_ALT   3

extern short g_btl_voice_kind;
extern short g_btl_voice_line;
extern short g_btl_voice_who;
extern short g_btl_voice_step;
extern short g_btl_offer_slot;

void BtlQueueVoice(u_short line, int alt)
{
    int kind;

    /* Written once outright and once again from the branch. The first store
       looks redundant and is in the original. */
    g_btl_voice_kind = VOICE_PLAIN;
    if (alt != 0) {
        kind = VOICE_ALT;
    } else {
        kind = VOICE_PLAIN;
    }
    g_btl_voice_kind = kind;
    g_btl_voice_step = 0;
    g_btl_voice_line = line & 0xFF;
    g_btl_voice_who = g_btl_offer[g_btl_offer_slot].used;
}
