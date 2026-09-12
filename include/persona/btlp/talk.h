#ifndef PERSONA_BTLP_TALK_H
#define PERSONA_BTLP_TALK_H

/* Persona 1 (JP) - the negotiation's own state.
 *
 * A contact is scored a line at a time: g_btl_talk_line is what was just said,
 * g_btl_talk_last_line what came before it and g_btl_talk_said which of the
 * four moods is being worked on, all -1 while nothing has been said.
 * g_btl_talk_step is the weight a line carries, 4 for an ordinary one and 6
 * for a strong one the demon has just heard, and g_btl_talk_result collects
 * what the line did.
 *
 * The four mood gauges are drawn through g_btl_mood_bar, which is aimed at the
 * acting offer's own shorts so the drawing code does not need the offer.
 */
#include <decomp/types.h>

/* Where a contact stands. All three are -1 for "nothing yet". */
extern short g_btl_talk_line;
extern short g_btl_talk_last_line;
extern short g_btl_talk_said;
extern short g_btl_talk_step;
extern short g_btl_talk_result;
/* Raised while a demon that has agreed to go is walking off, and cleared by
   whatever finishes that - the persona motions among them. */
extern u_char g_btl_talk_leaving;
extern u_char g_btl_talk_asked;
extern u_char g_btl_talk_last_scene;
extern int    g_btl_talk_reply;

/* Puts the negotiation back to nothing. Answers non-zero when it had one
   running and the caller should let it finish rather than carry on. */
extern int BtlResetTalk(int open);

/* Where a line starts, before BtlTalkScoreLine moves it. */
#define TALK_STEP_FIRST 4

/* The four gauges and the state kept beside them, and the panel's copy. */
extern short   g_btl_mood_state[];
extern short  *g_btl_mood_bar[];
extern u_short g_btl_panel_gauges;

#define BTL_MOOD_BYTES 8

/* Who is talking to whom. */
extern u_short g_btl_talk_level;
extern u_char  g_btl_offer_count;
extern u_char  g_btl_talker_count;
extern int     g_btl_level_gap;
extern u_char  g_btl_talk_pair;
extern u_short g_btl_choice_row;

extern void BtlTalkStart(void);

/* Answers what the scene ended as; BtlRunTalkScene hands that
   answer straight back to whoever opened the negotiation. */
extern int  BtlTalkSceneStep(void);
extern void BtlTalkLoop(void);
extern int  BtlBeginTalking(void);
extern void BtlClearMemberLines(void);
extern void BtlClearLineHistory(void);
extern void BtlMoodsFromMoon(void);

/* One answer for each unordered pair of the four talk acts. The table lives
   in writable data; const changes the scheduling of its message-group load. */
typedef struct {
    /* 0x0 */ short two_part;
    /* 0x2 */ u_char scene;
    /* 0x3 */ u_char pad03[1];
    /* 0x4 */ short group;
    /* 0x6 */ short index;
} BtlTalkAnswerRow; /* 8 bytes */

extern BtlTalkAnswerRow g_btl_talk_answers[];
extern u_long g_btl_moon_new_partners[];
extern u_long g_btl_moon_full_partners[];
extern short g_btl_talk_pair_acts[];
extern const u_char *g_btl_arcana_names[];

extern int BtlRecentOther(int value);
extern int BtlTalkPairIndex(u_int acts);
extern void BtlTalkAnswer(int slot, u_int act);

#endif
