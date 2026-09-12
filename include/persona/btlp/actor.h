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
    /* 0x68 */ u_char  pad68[0xC];
    /* 0x74 */ int     unk74;      /* what the negotiation's parting gift adds
                                      its experience to, once Char.unk14 is
                                      under the cap                          */
    /* 0x78 */ u_char  pad78[4];
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
    /* 0x84 */ u_short unk84;      /* cleared as the demon strikes the acting
                                      member on its way out                  */
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
    /* 0xBA */ u_char  unkBA;      /* taken off the object as an interruption
                                      is set up                            */
    /* 0xBB */ u_char  move;       /* the move this fighter is making. Set from
                                      the menu for a member and by
                                      BtlChooseEnemyMove for an enemy, and read
                                      by BtlAimMove to work out what it hits */
    /* 0xBC */ u_char  ail_line;   /* which of g_btl_ailment_lines is put up
                                      when the ailment stops the turn       */
    /* 0xBD */ u_char  unkBD;      /* cleared and then copied from unkBA    */
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
    /* 0xC5 */ u_char  unkC5;      /* both cleared as a record is filled     */
    /* 0xC6 */ u_char  unkC6;
    /* 0xC7 */ u_char  mark_kind;  /* which marker goes up over the fighter;
                                      5 is the one that says the action it
                                      was given cannot be made             */
    /* 0xC8 */ u_char  unkC8;      /* put back to zero alongside Char.unk5D
                                      when the gun turns out to be unusable */
    /* 0xC9 */ u_char  action;     /* what the round is doing for this
                                      fighter this turn - the switch
                                      BtlStageRound opens the turn with.
                                      0xFF once the turn is spent          */
    /* 0xCA */ u_char  clut_len;   /* entries in this actor's palette, which is
                                      what BtlStepCluts walks               */
    /* 0xCB */ u_char  padCB[1];
    /* 0xCC */ u_char  unkCC;      /* the five below are cleared as a record  */
    /* 0xCD */ u_char  padCD[3];   /* is filled, and nothing has read them    */
    /* 0xD0 */ u_char  unkD0;      /* counted up on the acting fighter by two
                                      of the effect handlers - move 0x4C's,
                                      when the fight is one that may be run
                                      from, and the ward the 0x8C family
                                      leaves; nothing has been found that
                                      reads it                              */
    /* 0xD1 */ u_char  padD1[1];
    /* 0xD2 */ u_char  unkD2;
    /* 0xD3 */ u_char  unkD3;
    /* 0xD4 */ u_char  unkD4;
    /* 0xD5 */ u_char  unkD5;      /* cleared with unkCC as a turn ends    */
    /* 0xD6 */ u_char  padD6[2];
    /* 0xD8 */ u_char  unkD8;      /* stops a member's marker being taken away
                                      as the turn ends, and is cleared there  */
    /* 0xD9 */ u_char  padD9[2];
    /* 0xDB */ u_char  unkDB;      /* both cleared for every member once a
                                      negotiation is over                  */
    /* 0xDC */ u_char  unkDC;
    /* 0xDD */ u_char  padDD[2];
    /* 0xDF */ u_char  unkDF;
    /* 0xE0 */ u_char  offered;    /* set for each enemy an offer involved
                                      once that offer is done with; the
                                      battle only ever clears it again   */
    /* 0xE1 */ u_char  unkE1[6];   /* cleared with `offered`, as one run     */
    /* 0xE7 */ u_char  ward_turns; /* rounds the ward at BTL_ACTOR_WARDS has
                                      left to run. BtlFxFinish8C sets it as
                                      the ward lands and the round counts it
                                      down, clearing all four bits together
                                      when it reaches nought. Cleared with
                                      the run above as a record is filled   */
    /* 0xE8 */ u_char  padE8[2];
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

/* The four a contact can end in, one scene each. BtlTalkEndStatus puts the
   code on every demon the offer involved. */
#define BTL_STATUS_HAPPY 1
#define BTL_STATUS_PANIC 2
#define BTL_STATUS_CHARM 3
#define BTL_STATUS_BIND  6

/* And the one the critical roll singles out: a fighter turned to stone is
   still a target, and an easy one. */
#define BTL_STATUS_STONE 0xF

/* An actor flag with the same effect as BTL_STATUS_DOWN everywhere it is
   tested; the two are always checked together. */
#define BTL_ACTOR_OUT 0x4000

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

extern BtlActor g_btl_actors[];

/* The same table reached through a pointer. Several units walk it this way
   rather than by the array, and the two are the same records. */
extern BtlActor *g_btl_combatants;

extern int BtlActorIsDown(int slot);

extern int BtlDownMemberNext(int slot);
extern int BtlDownMemberPrev(int slot);
extern int BtlPickableNext(int slot);
extern int BtlPickablePrev(int slot);

/* Reads one fighter's artwork in over whatever was there. */
extern void BtlLoadActorGfx(int slot);

/* Fills the status board in with one enemy's derived numbers. */
extern void BtlShowEnemyStatus(int slot);

/* Unlike the walkers above these stop at the ends of the party instead of
   wrapping, and answer -1 when no slot is left. */
extern int BtlUnreadyMemberNext(int slot);
extern int BtlUnreadyMemberPrev(int slot);

#endif
