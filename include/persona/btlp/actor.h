/* Persona 1 (JP) - everyone taking part in a battle.
 *
 * One table, 0xEC bytes a record: the party in slots 0 to 4 and the enemies in
 * the nine that follow, which is why the target picker walks nine records from
 * slot 5 and the command cursor wraps at five.
 *
 * Each record opens with a whole Char, so the battle reads a fighter's ailment,
 * name and key straight out of it and the field's own code understands the same
 * bytes. The key doubles as "this slot is in use" - it is zero in a record
 * nobody occupies - and the battle indexes the contact labels with it and loads
 * that character's graphics by it.
 *
 * Char.status runs on past the seventeen ailments the status screen can name;
 * the codes below are battle-only and never leave the fight.
 */
#ifndef PERSONA_BTLP_ACTOR_H
#define PERSONA_BTLP_ACTOR_H

#include <decomp/types.h>
#include <persona/common/char.h>
#include <persona/btlp/object.h>

typedef struct BtlActor {
    /* 0x00 */ Char    c;
    /* 0x60 */ BtlObj *obj;       /* what is drawn for this actor */
    /* 0x64 */ u_long  flags;
    /* 0x68 */ int     damage_dealt; /* what this fighter's spells have taken
                                      off everyone they hit, a kill counting
                                      only as far as nought               */
    /* 0x6C */ int     won_share;  /* what the results weigh this member's part
                                      of the fight at: what its spells took,
                                      and its share of the hp the party lost  */
    /* 0x70 */ int     casts;      /* how many times it called a Persona out;
                                      g_btl_won_casts is the party's total   */
    /* 0x74 */ int     unk74;      /* what the negotiation's parting gift adds
                                      its experience to, once Char.unk14 is
                                      under the cap                          */
    /* 0x78 */ int     unk78;      /* drawn beside unk74 on the board a won
                                      fight is shown on                    */
    /* 0x7C */ u_short drop;       /* PersonaData.drop, copied in as the record
                                      is filled from a Persona: the item this
                                      fighter leaves behind and how freely   */
    /* 0x7E */ u_short price;      /* the low half of PersonaData.price, from
                                      the same copy                          */
    /* 0x80 */ u_short targets;    /* who this fighter's action hits, one bit
                                      per slot. BtlPickableMask fills it in
                                      for a chosen target and the aiming
                                      routines for one the move picks itself */
    /* 0x82 */ u_short targets_kept; /* the copy the party's standing orders
                                        keep, so "the same again" can put the
                                        chosen action back                  */
    /* 0x84 */ u_short hit_amount; /* what the last blow took off hp, which
                                      the number put up over the fighter
                                      shows; cleared as the demon strikes
                                      the acting member on its way out     */
    /* 0x86 */ u_short melee_atk;  /* the fight's own copy of Char 0x2E..0x3C.
                                      BtlActorDeriveStats takes it field for
                                      field as the actor joins and then bends
                                      it - halving the two attack numbers,
                                      zeroing evade - for whatever ailment and
                                      equipment the actor is under. Everything
                                      that swings reads these, not Char's. */
    /* 0x88 */ u_short melee_hit;
    /* 0x8A */ u_short gun_atk;
    /* 0x8C */ u_short gun_hit;
    /* 0x8E */ u_short defence;
    /* 0x90 */ u_short evade;
    /* 0x92 */ u_short unk3A;      /* Char.unk3A, which the negotiation weighs
                                      the acting member's against the demon's
                                      to decide whether a contact lands    */
    /* 0x94 */ u_short unk3C;
    /* 0x96 */ u_short persona_num[4];
                                   /* the equipped Persona's +0x10..+0x16, put
                                      here by BtlApplyPersona. The first two go
                                      on into Char.unk3A and Char.unk3C, which
                                      is where the negotiation reads the one it
                                      weighs a contact with. */
    /* 0x9E */ u_short persona_sum; /* half its fourth stat plus its fourth plus
                                       twice its third */
    /* 0xA0 */ u_char  stat[5];    /* Char.stat, copied and bent the same way */
    /* 0xA5 */ u_char  persona_stat[5];
                                   /* the Persona's own five, which Char.stat is
                                      held at or above */
    /* 0xAA */ u_char  equip_stat[5];
                                   /* what the seven equipped items add to each
                                      stat, totalled out of ItemDef's three
                                      bonus bytes */
    /* 0xAF */ u_char  spell[6];   /* what this fighter can cast: PersonaData
                                      +0x2E..+0x33, kept whole. BtlChooseEnemyMove
                                      walks these and indexes g_spell_data with
                                      each, which is what says what they are. */
    /* 0xB5 */ u_char  species;    /* which demon this is; the negotiation
                                      matches its tables against it       */
    /* 0xB6 */ u_char  persona_rank;
                                   /* which row of g_btl_persona_ranks the
                                      Persona's key earns - the first whose
                                      first word the key does not exceed */
    /* 0xB7 */ u_char  pickable;   /* a whole side is switched on at once
                                      before a pick; BtlPickMember draws a
                                      member without it at a fifth of the
                                      brightness and leaves its palette alone */
    /* 0xB8 */ u_char  marker;     /* 3 while the member's marker is up, 0
                                      when it has been taken away          */
    /* 0xB9 */ u_char  initiative; /* what decides where this fighter comes in
                                      the round: a roll off Char.stat[3] most
                                      of the time, stat[4] on a good one, and
                                      both added on a very good one       */
    /* 0xBA */ u_char  turn_slot;  /* which of the fighter's spells the turn
                                      casts, copied off the object's
                                      spell_slot as the round hands the turn
                                      out; the enemy motions pick the
                                      species' tables for the move by it   */
    /* 0xBB */ u_char  move;       /* the move this fighter is making. Set from
                                      the menu for a member and by
                                      BtlChooseEnemyMove for an enemy, and read
                                      by BtlAimMove to work out what it hits */
    /* 0xBC */ u_char  ail_line;   /* which of g_btl_ailment_lines is put up
                                      when the ailment stops the turn. The
                                      item command leaves the item's id here */
    /* 0xBD */ u_char  turn_move;  /* the move the turn plays out, copied from
                                      `move` beside turn_slot - nought for a
                                      plain attack. The enemy motions take
                                      the blow's aim and count from its
                                      g_spell_data entry                   */
    /* 0xBE */ u_char  move_kept;  /* move and ail_line kept the same way and
                                      restored together with the two above  */
    /* 0xBF */ u_char  ail_line_kept;
    /* 0xC0 */ u_char  padC0[1];
    /* 0xC1 */ u_char  form;      /* which shape a fighter that changes into
                                     something is in. It picks the script out
                                     of the object's own table and the key the
                                     actor takes while it is in that shape. */
    /* 0xC2 */ u_char  order;      /* where this fighter comes in the round.
                                      An interruption takes the slowest place
                                      and five more, so it acts last        */
    /* 0xC3 */ u_char  order_kept;
    /* 0xC4 */ u_char  script_pick; /* chooses between two of the model's
                                       scripts when the actor is set going */
    /* 0xC5 */ u_char  resume_motion; /* the motion and phase the object was on
                                        when something took it over, put back
                                        once that is done; both cleared as a
                                        record is filled                   */
    /* 0xC6 */ u_char  resume_phase;
    /* 0xC7 */ u_char  mark_kind;  /* which marker goes up over the fighter;
                                      5 is the one that says the action it
                                      was given cannot be made             */
    /* 0xC8 */ u_char  tactic;     /* which of the tactics page's three
                                      columns the member is set to; the page
                                      cycles it with left and right, the save
                                      keeps it, and it is put back to zero
                                      alongside Char.unk5D when the gun turns
                                      out to be unusable                    */
    /* 0xC9 */ u_char  action;     /* what the round is doing for this
                                      fighter this turn - the switch
                                      BtlStageRound opens the turn with.
                                      0xFF once the turn is spent          */
    /* 0xCA */ u_char  clut_len;   /* entries in this actor's palette, which is
                                      what BtlStepCluts walks               */
    /* 0xCB */ u_char  summon;     /* the key of the Persona a cast calls up:
                                      the member's own, or the one an enemy's
                                      key picks for its Persona move       */
    /* 0xCC */ u_char  unkCC;      /* the five below are cleared as a record  */
    /* 0xCD */ u_char  level_up;   /* is filled. BtlBattleResults raises it as
                                      the fight's experience takes the level
                                      up, and board 0x1F lights a mark for
                                      it                                   */
    /* 0xCE */ u_char  unk56_up;   /* the same for Char.unk56               */
    /* 0xCF */ u_char  unkCF;
    /* 0xD0 */ u_char  unkD0;      /* counted up on the acting fighter by two
                                      of the effect handlers - move 0x4C's,
                                      when the fight is one that may be run
                                      from, and the ward the 0x8C family
                                      leaves; nothing has been found that
                                      reads it                              */
    /* 0xD1 */ u_char  wound;      /* rounds the wound at BTL_ACTOR_WOUND has
                                      run: each round's end takes one more
                                      than this in hp and counts it up, to
                                      at most 0x7F                         */
    /* 0xD2 */ u_char  build;      /* how often enemy move 0xDC has landed,
                                      one to eight; each count adds an
                                      eighth to the blow                   */
    /* 0xD3 */ u_char  unkD3;
    /* 0xD4 */ u_char  unkD4;
    /* 0xD5 */ u_char  counter;     /* a counter-attack is armed: the fighter
                                      answers the blow it was just dealt
                                      before its own turn resumes. Cleared
                                      with unkCC as a turn ends           */
    /* 0xD6 */ u_char  counter_slot; /* whose blow it is answering       */
    /* 0xD7 */ u_char  counter_turn; /* raised when the counter was armed on
                                        a fighter whose own turn was still
                                        to come, so that putting the
                                        counter away gives the turn back
                                        rather than spending it          */
    /* 0xD8 */ u_char  unkD8;      /* stops a member's marker being taken away
                                      as the turn ends, and is cleared there  */
    /* 0xD9 */ u_char  place_col;  /* the cell a member was put on in the */
    /* 0xDA */ u_char  place_row;  /* placement menu, and fell on         */
    /* 0xDB */ u_char  revive_slot; /* the fallen member a revival this
                                       fighter carries is for; cleared with
                                       revive_mark for every member once a
                                       negotiation is over                 */
    /* 0xDC */ signed char revive_mark;
                                   /* BTL_REVIVE_STOOD on a fallen member who
                                      can be raised where they stood, and
                                      BTL_REVIVE_CARRIED on the fighter whose
                                      revival is aimed at them            */
    /* 0xDD */ u_char  pick_saved; /* where BtlAilmentTurnMad puts the
                                     pickable list aside while it tries a
                                     shape on one candidate after another,
                                     and puts it back from            */
    /* 0xDE */ u_char  unkDE;      /* raised on the slowest combatant by the
                                      round and only ever lasts it: the round's
                                      end clears it and attribute bit 0x40 with
                                      it                                     */
    /* 0xDF */ u_char  unkDF;
    /* 0xE0 */ u_char  stage[7];   /* how far the stage moves have driven the
                                      fighter. BtlFxFinish53 puts the first
                                      three up by one for 0x53..0x55, to at
                                      most four, and the last four for
                                      0x57..0x5A, to at most seven, and 0x56
                                      and 0x5B clear each group; the move menu
                                      offers those two only while some enemy
                                      holds one of the first three or some
                                      member one of the last four. An offer
                                      that is done with sets the first on
                                      every enemy it involved             */
    /* 0xE7 */ signed char ward_turns; /* rounds the ward at BTL_ACTOR_WARDS has
                                      left to run. BtlFxFinish8C sets it as
                                      the ward lands and the round counts it
                                      down, clearing all four bits together
                                      when it reaches nought. Cleared with
                                      the run above as a record is filled   */
    /* 0xE8 */ signed char timed_a; /* rounds BTL_ACTOR_TIMED_A has left, and */
    /* 0xE9 */ signed char timed_b; /* BTL_ACTOR_TIMED_B; the round's end counts
                                      both down and drops the flag at nought */
    /* 0xEA */ u_char  ail_turns;  /* how long the ailment at Char.status is
                                      meant to last. BtlInflictStatus sets it
                                      from the ailment - ten turns for the
                                      lasting ones, three for a cloak, two for
                                      the rest - and a negotiation ending puts
                                      the demon's on the same two           */
    /* 0xEB */ u_char  padEB[1];
} BtlActor;                       /* 0xEC bytes */

#define BTL_PARTY   5     /* slots 0..4 */
#define BTL_ENEMIES 9     /* slots 5..13 */
#define BTL_ACTORS  14    /* both sides, in one walk */

/* Char.status indexes g_status_names, twenty-four eight-byte labels the status
   screen draws. Code 0 is the healthy one, so a non-zero status is an ailment:
   GOOD HAPPY PANIC CHARM FREEZE SHOCK BIND SLEEP CLOSE BLIND UNLUCK TERROR
   GUILT POISON PALYZE STONE SICK DEAD CLOAK PUPPET COUNTR BARSAK MAD WOLF.

   The two below are the ones the battle leans on. DEAD is out of the fight -
   not a target, no HUD row, skipped by the cursor - and PUPPET only stops the
   cursor: the actor is alive and still a target, it just cannot be given an
   order. */
#define BTL_STATUS_DOWN    0x11    /* DEAD   */
#define BTL_STATUS_LIFTED  0x12    /* lifted off the floor; the actor goes
                                      plain white and loses its shadow, and
                                      a random pick steps over it        */
#define BTL_STATUS_NOINPUT 0x13    /* PUPPET */
#define BTL_STATUS_COUNTER 0x14    /* COUNTR: a blow that lands gives the
                                      fighter a turn of its own at once  */

/* The four a contact can end in, one scene each. BtlTalkEndStatus puts the
   code on every demon the offer involved. */
#define BTL_STATUS_HAPPY 1
#define BTL_STATUS_PANIC 2
#define BTL_STATUS_CHARM 3
#define BTL_STATUS_BIND  6

/* And the one the critical roll singles out: a fighter turned to stone is
   still a target, and an easy one. */
#define BTL_STATUS_STONE 0xF

/* The one beside it that also leaves a fighter unable to act, which the
   battle's outcome counts the same way. */
#define BTL_STATUS_PALYZE 0xE

/* Three more that stop a member's command: GUILT a swing outright and a cast
   once it has any level, CLOSE and BLIND a cast at their last. */
/* The eight BtlDeriveBattleStats bends a fighter's numbers for. */
#define BTL_STATUS_SLEEP  7
#define BTL_STATUS_UNLUCK 0xA
#define BTL_STATUS_POISON 0xD
#define BTL_STATUS_BARSAK 0x15
#define BTL_STATUS_MAD    0x16
#define BTL_STATUS_WOLF   0x17

#define BTL_STATUS_CLOSE 8
#define BTL_STATUS_BLIND 9
#define BTL_STATUS_GUILT 0xC

/* Where a fighter's own order is kept while a counter-attack borrows it, so
   the turn it interrupted can be put back. The target mask beside it at
   0x800F5AAC is not here: it is written as a whole word and read back as a
   half, so the two units that touch it declare it for themselves. */
extern u_char g_btl_counter_order;

/* An actor flag with the same effect as BTL_STATUS_DOWN everywhere it is
   tested; the two are always checked together. */
#define BTL_ACTOR_OUT 0x4000

/* Set on a member whose command could not be made - the round shakes it and
   gives it action 4 whatever its tactic says - and cleared again as its
   ailment decides the turn. A fighter under it is not given the turn a
   counter-attack would slip into the order. */
#define BTL_ACTOR_REFUSED 0x8000000

/* The wound enemy move 0xE3 leaves: at every round's end the fighter loses
   one more hp than the round before, counted in `wound`. */
#define BTL_ACTOR_WOUND 0x1000000

/* Set on a member that has flinched: its flinch is the script it stands in
   until something clears it. */
#define BTL_ACTOR_FLINCHED 0x2000

/* The ward moves 0x8C..0x8F leave behind, one bit each and mutually
   exclusive: BtlFxFinish8C clears all four before it sets the one its own
   move grants, and the round clears all four together when `ward_turns` runs
   out. They are named for the move that grants them, the way the effect
   handlers themselves are, because nothing else says what each one turns
   aside. BtlStageRound greys all four moves out on a fighter already carrying
   one, and move 0x86 will not reach a fighter behind the last two. */
#define BTL_ACTOR_WARD_8C 0x200
#define BTL_ACTOR_WARD_8D 0x400
#define BTL_ACTOR_WARD_8E 0x800
#define BTL_ACTOR_WARD_8F 0x1000
#define BTL_ACTOR_WARDS   0x1E00

/* Two more conditions that wear off by the round, each with a counter of its
   own at `timed_a` and `timed_b`. Nothing read so far says what either is. */
#define BTL_ACTOR_TIMED_A 0x100000
#define BTL_ACTOR_TIMED_B 0x200000

/* Three more set by the stage moves' finish and named for the move that sets
   them. BTL_ACTOR_5C goes on with unkDE cleared and comes off again with
   unkDE as the round ends; the other two are set on every fighter 0x5D or
   0x5E reaches and only last the turn, and the move menu offers 0x5D only
   while some enemy is still without its bit. */
#define BTL_ACTOR_5C 0x40
#define BTL_ACTOR_5D 0x80
#define BTL_ACTOR_5E 0x100

/* Set by the recovery moves' finish for 0xF5, on a fighter
   BTL_ACTOR_TIMED_A does not hold. */
#define BTL_ACTOR_F5 0x2000000

/* BtlActor.marker: up over the fighter, what a member who has moved is given,
   and what one who has been given a command carries until the round takes it
   or the command is taken back. */
#define BTL_MARKER_UP      3
#define BTL_MARKER_MOVED   2
#define BTL_MARKER_ORDERED 1

/* BtlActor.revive_mark: on a fallen member a revival can raise where they
   stood, and on the member carrying that revival. */
#define BTL_REVIVE_STOOD   1
#define BTL_REVIVE_CARRIED (-1)

/* Three flags of the fighter's own. The first is turned over into the second
   the moment the scripted turn is handed out, and the third is the same trade
   one encounter makes on its own. A spell cast under the second always lands
   and takes all of the target's hp; under the third it lands for half, or a
   quarter from fighter 0xB8, and under the first an enemy's spell is not
   rolled for at all. */
#define BTL_ACTOR_SCRIPT_READY 0x20000000
#define BTL_ACTOR_SCRIPT_DONE  0x40000000
#define BTL_ACTOR_SCRIPT_ALT   0x04000000

extern BtlActor g_btl_actors[];

/* Takes a level off a fighter and works its experience out again.
   personagrow.c. */
extern void BtlDrainLevel(BtlActor *a);

/* The same table reached through a pointer. Several units walk it this way
   rather than by the array, and the two are the same records. */
extern BtlActor *g_btl_combatants;

extern int BtlActorIsDown(int slot);

extern int BtlDownMemberNext(int slot);
extern int BtlDownMemberPrev(int slot);
extern int BtlPickableNext(int slot);
extern int BtlPickablePrev(int slot);

/* Reads one fighter's artwork in over whatever was there. */
/* Answers nought always, and no caller reads it. */
extern int  BtlLoadActorGfx(int actor);

/* Fills the status board in with one enemy's derived numbers. */
extern void BtlShowEnemyStatus(int slot);

/* Unlike the walkers above these stop at the ends of the party instead of
   wrapping, and answer -1 when no slot is left. */
extern int BtlUnreadyMemberNext(int slot);
extern int BtlUnreadyMemberPrev(int slot);

/* A member's record brought up to date with the equipped Persona, and the
   fight's own stats derived from the record again. applypersona.c and
   recalc.c. */
extern void BtlApplyPersona(BtlActor *a);
extern void BtlRecalcStats(BtlActor *a);

/* The fight's own copy of a fighter's numbers worked out from the record
   and whatever it is carrying: what it hits for, what it hits with, what
   it shrugs off, and what its ailment does to all of that. derivestats.c.
   */
extern void BtlDeriveBattleStats(BtlActor *a);

#endif
