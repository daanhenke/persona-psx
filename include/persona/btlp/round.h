/* Persona 1 (JP) - what one round of a battle runs on.
 *
 * The stage table in persona/btlp/stage.h says which of the four stages is
 * running; this is the state the round stage itself keeps, plus the handful
 * of routines the stages call that belong to no other unit.
 */
#ifndef PERSONA_BTLP_ROUND_H
#define PERSONA_BTLP_ROUND_H

#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/common/item.h>

/* Counted up once per round, at the point the order to act in is settled.
   The results screen reads it. */
extern int g_btl_round;

/* Frames the round is waiting out. BtlDrawFrame counts it down, so a stage
   that wants to hold still leaves a number here and tests it for zero. */
extern short g_btl_delay;

/* Raised once the battle has decided how it ended and is only playing that
   out; BtlDrawFrame leaves some of its work undone under it. */
extern u_char g_btl_closing;

/* Raised by BtlBattleOutcome when the party is wiped, and read by
   BtlStageClose, which takes the game-over way out instead of preloading a
   scene to go back to. */
extern u_char g_btl_party_lost;

/* Which of the battle's pieces of music is playing, and what kind each one
   is - the kind decides whether the round has to stop it itself. */
extern u_char g_btl_bgm_index;
extern u_char g_btl_bgm_kinds[];

#define BTL_BGM_KIND_KEEP 1  /* leave it running when the battle ends  */
#define BTL_BGM_KIND_STOP 3  /* the round stops it as the first attack
                                lands                                  */

/* The three lines the close puts up itself: the two the party is thanked
   with for turning up, and the one that says they are finished. */
extern u_char g_btl_msg_place1[];
extern u_char g_btl_msg_place2[];
extern u_char g_btl_msg_defeat[];

/* Four bytes per encounter: the map to go back to, and the room in it.
   BtlSetReturnMap copies the pair into g_map_id and g_map_room. */
typedef struct {
    /* 0x0 */ u_short map;
    /* 0x2 */ u_char  room;
    /* 0x3 */ u_char  pad3;
} BtlReturnMap;                 /* 4 bytes */

extern BtlReturnMap g_btl_encounter_maps[];

#define BTL_AI_MOVES 7
#define BTL_AI_MOODS 3

/* What an enemy will do with a turn, one record per mood. The mood is the
   same for the whole fight - ovl_btlp_entry picks it once - so the record is
   really a difficulty setting for that species.

   The two chances are rolled before the move list is looked at, so an enemy
   that is going to run or stand still never gets as far as choosing a spell. */
typedef struct {
    /* 0x0 */ u_char move[BTL_AI_MOVES];
                                /* the moves this mood will consider, best
                                   first; only the ones g_btl_move_ok allows
                                   make it into g_btl_move_choices */
    /* 0x7 */ u_char idle;      /* out of 255: do nothing this turn        */
    /* 0x8 */ u_char flee;      /* out of 255: run. Doubled when the enemy
                                   is ten or more levels under the party   */
    /* 0x9 */ u_char odds;      /* which row of g_btl_move_odds weighs the
                                   list that survived                      */
} BtlEnemyAi;                   /* 10 bytes */

/* Indexed by the fighter's Char.key and then by g_btl_ai_set. */
extern BtlEnemyAi g_btl_enemy_ai[][BTL_AI_MOODS];

/* Which of the three moods this fight runs on. Set once as the overlay opens
   and never moved; two is the one that turns several behaviours off. */
extern u_char g_btl_ai_set;

#define BTL_AI_SET_TAME 2

/* One row per (weighting, number of survivors): the running chance, out of
   255, that each survivor in turn is the one taken. The first entry that the
   roll does not exceed wins, so a row is read in order and never past its own
   length. The row for a list of one is the first, so the count indexes it a
   place back. */
extern u_char g_btl_move_odds[][8][8];

/* Set for every move the fighter could actually make this turn, cleared at
   the top of every choice. Index 0 is the plain attack; 1 to 6 are the six
   spells in BtlActor.spell. */
extern u_char g_btl_move_ok[9];

/* The moves that survived, in the order the mood lists them. */
extern u_char g_btl_move_choices[8];

/* The three moves that are not in the list: doing nothing, running, and the
   two bosses' change of shape. */
#define BTL_MOVE_IDLE  7
#define BTL_MOVE_FLEE  8
#define BTL_MOVE_MORPH 10

/* Set on the fighter alongside BTL_MOVE_IDLE. */
#define BTL_ACTOR_IDLE 0x8000

/* Suppresses running for the rest of the fight. */
extern u_char g_btl_no_flee;

/* The two bosses that change shape rather than act. Each keeps which shape it
   is in and whether it has already changed, and each has its own pair of
   forms for the two directions. */
extern u_char g_btl_boss22_shape;
extern u_char g_btl_boss22_shown;
extern u_char g_btl_boss20_shape;
extern u_char g_btl_boss20_shown;

/* Where the AI looks for a target for one move, and whether anyone on the
   party side can be reached at all. */
extern int BtlPickAiTarget(BtlActor *a, int target);
extern int BtlAnyMemberTargetable(void);

/* The round, step by step. g_btl_turn walks g_btl_turn_order as far as
   g_btl_turns; g_btl_actor_turn is whose turn is being played out. */
extern u_char g_btl_turn;
extern u_char g_btl_turns;
extern u_char g_btl_turn_order[];
extern short  g_btl_actor_turn;

/* A Persona can take the turn away from its owner. The kind says which of
   the two ways it did, and the rest is what the owner's own turn was, put
   back once the Persona is done. */
extern u_char g_btl_act_kind;
extern u_char g_btl_act_actor;
extern u_char g_btl_act_move;
extern u_char g_btl_act_speed;
extern u_long g_btl_act_targets;

/* Frames the line the turn put up still has left. */
extern short g_btl_msg_timer;

/* How a negotiation ended, which decides what the round does next. Not the
   same thing as g_btl_talk_result, which is what the demon thought of the
   last line. */
extern u_char g_btl_talk_outcome;

/* Set when the round is to be given up on rather than played out. */
extern u_char g_btl_leave_round;
extern u_char g_btl_hold_markers;
extern u_char g_btl_pick_slowest;

/* The scripted scene a step can stop to play. */
extern u_char g_btl_scene_hud;
extern u_char g_btl_scene_wanted;
extern u_char g_btl_scene_pending;

/* The pad mask that ends a fight outright, read only with the debug HUD up. */
extern u_short g_btl_key_end;

extern BtlObj *g_btl_hud_obj;
extern BtlObj *g_btl_effect_obj;

/* One line per ailment, and the scripts a fighter changing shape runs. */
extern u_char *g_btl_ailment_lines[];
extern u_char  g_btl_morph_scripts[];
extern u_char  g_btl_msg_ailment[];
extern u_char  g_btl_seq_hud_up[];

/* The lines the round plays for its scripted fights, by encounter. */
extern u_char g_btl_line_enc02[];
extern u_char g_btl_line_enc03a[];
extern u_char g_btl_line_enc03b[];
extern u_char g_btl_line_enc03c[];
extern u_char g_btl_line_enc03d[];
extern u_char g_btl_line_enc04[];
extern u_char g_btl_line_enc05a[];
extern u_char g_btl_line_enc05b[];
extern u_char g_btl_line_enc05c[];
extern u_char g_btl_line_enc05d[];
extern u_char g_btl_line_enc05e[];
extern u_char g_btl_line_enc05f[];
extern u_char g_btl_line_enc08[];
extern u_char g_btl_line_enc11a[];
extern u_char g_btl_line_enc11b[];
extern u_char g_btl_line_enc11c[];
extern u_char g_btl_line_enc11d[];

/* Where a turn comes in the round, and setting one going. */
extern int  BtlSlowestOrder(void);
extern void BtlStartAction(void);
extern void BtlAimEnemyMove(BtlActor *a);
extern void BtlHoldForMarkers(void);
extern void BtlReadPackEntry(int blocking, int entry);
extern void BtlRefreshEnemyAttacks(void);
extern void BtlAimMove(BtlActor *a);
extern int  BtlOrderTurns(u_char *order, int n);
extern int  BtlResetTurnOrder(void);
extern void BtlSetPickable(void);
extern u_long BtlPickableMask(void);

/* Markers: whether they are all still, whether any is up, and taking them
   down again. */
extern int  BtlMarkersIdle(void);
extern int  BtlMarkersHidden(void);
extern void BtlHideMarkers(void);

/* The scenes and the tidying that follow a round or a negotiation. */
extern void BtlPlayScene(int voice, u_char *script);
extern void BtlRoundOverScene(void);
extern void BtlLastEnemyScene(void);
extern void BtlAfterTalk(void);
/* The three ways a negotiation hands the round back, one per value of
   g_btl_talk_outcome; each gives every party member still fighting an action
   again. */
extern void BtlTalkersJoin(void);
extern void BtlTalkersStay(void);
extern void BtlTalkersLeaveField(void);

/* The per-fighter half of those: an action that has already been chosen is
   aimed, given its place in the order and its target mask. Neither chooses
   the action, and both put up marker 5 when it cannot be made. */
extern void BtlReadyItemAction(BtlActor *a, const ItemDef *item);
extern void BtlReadySpellAction(BtlActor *a);

/* Paints the cells a move reaches onto the reach grid, from the shape and the
   flags the item or spell carries, and answers with the first cell it covers
   or -1 when it covers none. Everything that aims a move goes through it. */
extern int BtlMarkMoveArea(BtlActor *a, int shape, int flags);
extern void BtlClearTalkMarks(void);
extern void BtlBoxDismiss(void);

/* Which move one enemy makes with its turn. */
extern u_char BtlChooseEnemyMove(BtlActor *a);

/* What the fight is running up as it goes. Written where damage is applied
   and read by BtlBattleResults; BtlStageOpen clears the lot at the start of
   a battle. Which tally is which is not worked out yet - they are here so
   there is one place to name them once it is. */
extern int   D_800F4D5C;
extern int   D_800F4E20;
extern int   D_800F4AA4;
/* The four counters the fight resolves a hit through, set together as one is
   armed and walked down as it lands. `left` is how many are still to come,
   `walk` the slot the target search has reached and `mask` its bit, and
   `slot` whichever fighter is being resolved right now. armhit.c arms them
   and BtlMemberMotion02 walks them. */
extern short g_btl_hits_left;
extern short g_btl_hit_walk;
extern short g_btl_hit_mask;
extern short g_btl_hit_slot;

extern int   D_800F5A60;
extern int   D_800F5D58;

/* The item the fight leaves behind, as an id into g_item_defs. Cleared as the
   battle opens, written by BtlRollDefeatDrop as a demon goes down and read
   back once the fight is over, so the last kill's prize is the one kept. */
extern u_short g_btl_drop_item;

/* The object the opening is played on, and how far the camera has pulled
   back for it. */
extern BtlObj *g_btl_intro_obj;
extern short   g_btl_intro_dist;

/* Where the intro camera comes to rest. */
#define BTL_INTRO_SETTLED 0x200

/* True once nothing on the field is still playing a script, which is what a
   stage waits for before it hands over. */
extern int BtlActorsIdle(void);

/* The opening, in the order BtlStageOpen runs it: the entrance and the rise
   are each for one pair of encounters, the whitening is for one, and the
   dialogue is for everything below the scripted fights. */
extern void BtlOpenEnemyEntrance(void);
extern void BtlOpenEnemyRise(void);
extern void BtlOpenEnemyWhiten(void);
extern void BtlOpenDialogue(void);

/* Closes the gaps the dead leave in the enemy grid. */
extern void BtlPackEnemyGrid(void);

/* Pulls all six markers back in. */
extern void BtlRetractMarkers(void);

/* Puts g_map_id and g_map_room back to whatever the encounter says the
   party came from. */
extern void BtlSetReturnMap(void);

/* Working out how the battle ended, and then what it was worth. The three
   after the outcome run in that order on the way out. */
extern int  BtlBattleOutcome(void);
extern void BtlBattleResults(void);
extern void func_80097A50(void);
extern void BtlRestoreField(void);

/* Adds one of an item to the party's bag. */
extern void BtlGiveItem(int item);

#endif
