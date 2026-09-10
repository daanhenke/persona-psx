/* Persona 1 (JP) - putting the negotiation back to nothing.  BTLP only.
 *   0x8006903C BtlResetTalk
 *
 * Everything the negotiation keeps goes back: the phase, the acting member,
 * the offer and target slots, the counts and flags, the line history for both
 * sides, the mood state, the panel's gauges, and the scene stack - scenes to
 * none and stages to free. The four gauge pointers are aimed at the offer's
 * own shorts so the drawing code does not need the offer itself.
 *
 * Then the offers are rebuilt from the enemies present and given the moods the
 * moon says they start in.
 *
 * The answer is whether a negotiation actually opened, which only happens past
 * map 0x22, in an encounter that allows it, and when the caller asked.
 */
#include <decomp/types.h>
#include <rand.h>
#include <decomp/libc.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/talk.h>

/* Members, and how much of their record BtlResetTalk clears. */
#define BTL_MEMBER_BYTES 0xBE

/* Three offers of 0x48 bytes. */
#define BTL_OFFER_BYTES 0xD8

/* Levels of the scene stack. */
#define BTL_TALK_LEVELS 8

/* g_btl_talk_scene, for a level nothing is running on. */
#define TALK_SCENE_NONE 0xFF

/* Nothing said yet. */
#define TALK_LINE_NONE (-1)

/* The four mood gauges, and the state kept beside them. */
#define BTL_MOODS      4

/* Twelve bytes of the negotiation's own scratch. */
#define TALK_SCRATCH_BYTES 0xC

/* Maps below this have no negotiation, and neither does encounter kind 1. */
#define BTL_TALK_FIRST_MAP 0x22
#define BTL_BATTLE_NO_TALK 1

extern u_char   g_btl_member[];
extern u_char   g_btl_talking;
extern short    g_map_id;

extern int  VSync(int mode);
extern void BtlPanelSetImage(int group, u_char image);
extern void BtlInsertHeroName(void);
extern void BtlBuildOffers(void);

int BtlResetTalk(int open)
{
    short *m;
    int    on;

    BtlPanelSetImage(0, 0);
    srand(VSync(-1));
    /* Through a pointer to the first gauge: written out one by one gcc takes
       the second as its base and every offset comes out shifted. */
    m = g_btl_offer[0].mood;
    g_btl_mood_bar[1] = m + 1;
    g_btl_mood_bar[2] = m + 2;
    g_btl_mood_bar[0] = m;
    g_btl_mood_bar[3] = m + 3;
    g_btl_talk_reply = 0;
    g_btl_phase = 0;
    g_btl_actor_slot = 0;
    g_btl_offer_slot = 0;
    g_btl_talk_target = 0;
    g_btl_member_matched = 0;
    g_btl_offer_live = 0;
    g_btl_talk_level = 0;
    g_btl_offer_count = 0;
    g_btl_talker_count = 0;
    g_btl_talk_flags = 0;
    g_btl_level_gap = 0;
    g_btl_talk_pair = 0;
    g_btl_choice_row = 0;
    g_btl_talking = 0;
    BtlInsertHeroName();
    bzero(g_btl_member, BTL_MEMBER_BYTES);
    bzero((u_char *)g_btl_offer, BTL_OFFER_BYTES);
    BtlClearMemberLines();
    BtlClearLineHistory();
    g_btl_panel_gauges = 0;
    bzero((u_char *)g_btl_mood_state, BTL_MOOD_BYTES);
    g_btl_talk_line = TALK_LINE_NONE;
    g_btl_talk_last_line = TALK_LINE_NONE;
    g_btl_talk_said = TALK_LINE_NONE;
    g_btl_talk_step = TALK_STEP_FIRST;
    g_btl_talk_result = 0;
    g_btl_talk_asked = 0;
    g_btl_talk_depth = 0;
    memset(g_btl_talk_scene, TALK_SCENE_NONE, BTL_TALK_LEVELS);
    memset(g_btl_talk_stage, 0, BTL_TALK_LEVELS);
    g_btl_talk_last_scene = TALK_SCENE_NONE;
    bzero((u_char *)g_btl_talk_scratch, TALK_SCRATCH_BYTES);
    BtlBuildOffers();
    BtlMoodsFromMoon();
    if (g_map_id <= BTL_TALK_FIRST_MAP) {
        return 0;
    }
    /* Read as a byte, which is what the image does here - the variable
       itself is a word, and most of the overlay reads all of it. */
    if ((u_char)g_btl_battle_kind == BTL_BATTLE_NO_TALK) {
        return 0;
    }
    if (open == 0) {
        return 0;
    }
    on = BtlBeginTalking();
    return on;
}
