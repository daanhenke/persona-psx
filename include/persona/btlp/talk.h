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

/* Three words of the scratch pack's header, reached by name rather than as an
   offset off the buffer's address. The buffer itself stays a bare address -
   0x801C0000 has nothing in its low half, so naming it would cost a word - but
   a header word is read with a %hi/%lo pair either way, and through the name
   the pair is the image's. How far into the pack the choice rows and the acts
   that go with them begin, and how long the whole pack is. */
/* The pack itself, reached two ways because the image reaches it two ways: as
   a bare address where only the address is wanted - 0x801C0000 has nothing in
   its low half, so one `lui` does it and naming it would cost a word - and
   through the symbol where a word is read out of it, which is a %hi/%lo pair
   either way and so costs nothing. */
#define BTL_SCRATCH ((u_char *)0x801C0000)
extern u_char g_btl_scratch[];

extern int g_btl_choice_text;
extern int g_btl_scratch_size;
extern int g_btl_choice_table;
extern int g_btl_choice_acts;

extern void BtlTalkStart(void);

/* Answers what the scene ended as; BtlRunTalkScene hands that
   answer straight back to whoever opened the negotiation. */
extern int  BtlTalkSceneStep(void);
/* Answers nought when an offer that was taken ends the talk at once; every
   other way out leaves the answer unset. */
extern int  BtlTalkLoop(void);
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

/* The talk's own menus, and the contact box a member picks an act from. */
extern void BtlTalkSceneMenu(void);
extern void BtlTalkOpen(int key, int level, const char *name);
extern int  BtlTalkChoice(void);
extern void BtlTalkLive(int cell);
extern void BtlTalkIdle(void);
extern void BtlTalkHide(void);

/* How an act lands: whether the line is taken, the demon's reaction, the
   bonuses a Persona and liked equipment add, and the line said back. */
extern int   BtlTalkLineLands(void);
extern int   BtlPickReaction(int picked);

/* One entry of a reaction row: the condition it is taken under, and the odds
   out of a hundred of each gauge moving. A condition of nought always holds;
   below nine it is a bit of BtlOffer.flags that must be set, counted from one,
   and from nine a bit that must be clear, counted from nine. */
typedef struct {
    /* 0x0 */ u_char cond;
    /* 0x1 */ u_char odds[4];
} BtlReaction;                  /* 5 bytes */

/* By member (key less one), act, and whether the offer is wary. */
extern const BtlReaction g_btl_reactions[][4][2][5];

/* What a shifted offer adds to each gauge's odds, five rows of four. */
extern const short g_btl_reaction_shifts[][4];

extern void  BtlTalkPersonaBonus(int which);
extern int   BtlTalkLikedEquip(void);
extern int   BtlPickLine(int kind);
extern void  BtlTalkPickScript(int who, int verb, int variant);
extern void  BtlSayDemonLine(u_char act, u_char line);
extern void  BtlPushRecent(int value);
extern void  BtlMoodRetire(void);
extern short BtlPickTalkTarget(short mask);
extern void  BtlTintTalkers(void);
extern void  BtlTintParty(void);

/* Reading the talk's scratch pack in: the file, then the entry. The address
   just past the pack's header is left in g_btl_scratch_end. */
extern void    BtlSeekFile(int index);
extern void    BtlLoadScratch(int index, int from_table);
extern u_long *g_btl_scratch_end;

/* The act picked, the most recent acts, and the demon's reaction to the one
   just made: the gauge it moves in the low byte and the weight it adds to the
   push in the next. g_btl_force_reaction is a debug override - a gauge whose
   word is 1 is taken whatever the reaction was. */
extern short g_btl_talk_picked;
extern int   g_btl_recent[];
extern int   g_btl_talk_reaction;
extern int   g_btl_force_reaction[];

/* The gauges as they stood before an act landed, as one block - the four of
   BTL_MOODS, spelt out so this header stands without offer.h. */
typedef struct {
    /* 0x0 */ short mood[4];
} BtlMoods;
extern BtlMoods g_btl_mood_before;

/* Which scratch entry a Persona's own scene is, for the demons that play one
   when they are picked rather than waiting for the party. */
typedef struct {
    /* 0x0 */ u_short persona;
    /* 0x2 */ u_short entry;
} BtlTalkPersonaScene;
extern BtlTalkPersonaScene g_btl_talk_persona_scenes[];

/* The slot whose contact box is up, and the pack a member's scene is read
   from. */
extern int g_btl_talk_open_slot;
extern int g_btl_talk_member_pack;

/* What the menus say: an offer that cannot be talked to, a member with
   nothing to say, the demons walking off, the line a reaction plays, and the
   scene a won act plays by member and act. */
extern const u_char *g_btl_talk_unpickable_script;
extern const u_char *g_btl_talk_mute_script;
extern const u_char *g_btl_talk_turned_down_script;
extern const u_char *g_btl_talk_left_script;
extern const u_char *g_btl_talk_line_script;
extern const u_char *g_btl_talk_win_scripts[][4];

/* The talk scripts, and which one each member says for each act and line. */
#define TALK_VERBS    4
#define TALK_VARIANTS 3
extern const u_char *g_btl_talk_scripts[];
extern const u_char  g_btl_talk_script_ids[][TALK_VERBS][TALK_VARIANTS];

#endif
