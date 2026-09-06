/* Persona 1 (JP) - asking for a demon's voice line.  BTLP only.
 *   0x8006BFC4 BtlQueueVoice  0x8006C028 BtlUpdateVoices
 *
 * Four halfwords that BtlUpdateVoices picks up on its next pass: what sort of
 * line, which line, and which enemies it is meant for. The last is the offer's
 * own set, so a line asked for during a negotiation reaches everyone taking
 * part in it rather than one demon.
 *
 * The step goes back to zero here, which is what makes a second request cut
 * the first one short.
 *
 * BtlUpdateVoices is the other half, run from every wait loop in the overlay.
 * It takes the demons of the set one at a time - each one it starts speaking
 * is cleared from the set, so the line travels down the group rather than
 * playing over itself - and waits on the object's animation bits before moving
 * to the next. When the set runs dry the request is dropped and the target
 * goes back to whoever the offer says it should be.
 */
#include <types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/offer.h>

#define VOICE_PLAIN 1
#define VOICE_ALT   3

extern short g_btl_voice_kind;
extern short g_btl_voice_line;
extern short g_btl_voice_who;
extern short g_btl_voice_step;
extern short g_btl_offer_slot;
extern short g_btl_talk_target;
extern BtlActor g_btl_enemies[];

extern short BtlPickTalkTarget(short mask);
extern void  BtlTalkReact(u_char kind);

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

/* The two attribute bits that say the object is still playing its reaction:
   the animating bit set and the one above it clear. */
#define BTL_VOICE_BUSY_MASK 0x18000000
#define BTL_VOICE_BUSY      0x10000000

void BtlUpdateVoices(void)
{
    if ((g_btl_voice_kind & VOICE_PLAIN) != 0) {
        if (g_btl_voice_step == 0) {
            g_btl_talk_target = BtlPickTalkTarget(g_btl_voice_who);
            if (g_btl_talk_target != -1) {
                BtlTalkReact(g_btl_voice_line);
                g_btl_voice_step = 1;
                g_btl_voice_who &= ~(1 << g_btl_talk_target);
            } else {
                g_btl_voice_kind = 0;
                g_btl_talk_target =
                    BtlPickTalkTarget(g_btl_offer[g_btl_offer_slot].used);
            }
        } else if ((g_btl_enemies[g_btl_talk_target].obj->attr
                        & BTL_VOICE_BUSY_MASK) != BTL_VOICE_BUSY) {
            g_btl_voice_step = 0;
        }
    }
}
