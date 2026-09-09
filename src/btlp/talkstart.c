/* Persona 1 (JP) - opening a round of contact.  BTLP only.
 *   0x80069478 BtlTalkStart
 *
 * Everything a contact carries goes back to nothing - the face, the panel's
 * gauges, the four moods, the three line numbers, the weight and the result -
 * and the battle moves to phase 1, which is the negotiation. Then the loop is
 * entered and does not come back until the contact is over.
 *
 * BtlResetTalk clears far more than this and is what starts a negotiation from
 * scratch; this is the lighter reset between rounds of one.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/talk.h>

/* Nothing said yet. */
#define TALK_LINE_NONE (-1)

/* The phase the negotiation runs in. */
#define BTL_PHASE_TALK 1

void BtlTalkStart(void)
{
    g_btl_face_id = TALK_LINE_NONE;
    BtlHudLoad();
    g_btl_panel_gauges = 0;
    bzero((u_char *)g_btl_mood_state, BTL_MOOD_BYTES);
    g_btl_talk_line = TALK_LINE_NONE;
    g_btl_talk_last_line = TALK_LINE_NONE;
    g_btl_talk_said = TALK_LINE_NONE;
    g_btl_talk_step = TALK_STEP_FIRST;
    g_btl_talk_result = 0;
    g_btl_talk_asked = 0;
    g_btl_phase = BTL_PHASE_TALK;
    BtlTalkLoop();
}
