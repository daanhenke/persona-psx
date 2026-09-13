/* Persona 1 (JP) - what is wrong with a fighter, and how far it has gone.
 *
 * An ailment is a pair: Char.status says which one, Char.ail_level how deep,
 * and everything here is indexed by one or the other.
 *
 * Landing one is a three-part question. A demon has to be able to catch it at
 * all, which g_btl_demon_statuses answers by species; the ailment already in
 * place has to be one this one may land over, which g_btl_status_over answers
 * per level; and once it has landed, g_btl_status_allowed says whether the
 * fighter can still act.
 *
 * The marker floating over the fighter is drawn from two tables at once - the
 * ailment picks the marker's own script out of the actor graphics and the
 * level picks the pips attached to it.
 */
#ifndef PERSONA_BTLP_STATUS_H
#define PERSONA_BTLP_STATUS_H

#include <decomp/types.h>
#include <persona/btlp/actor.h>

/* Keys from here up are demons rather than party members, and only they are
   asked whether they can catch an ailment at all. */
#define BTL_KEY_DEMON 0x99

/* Levels an ailment can be driven to, and the last of them. */
#define BTL_AIL_LEVELS 3
#define BTL_AIL_DEEPEST 2

/* One mask per species of what it can be given. */
extern const u_long g_btl_demon_statuses[];

/* Which ailments a new one may land over, a row of BTL_AIL_LEVELS per ailment
   indexed by the level already reached. */
extern const u_long g_btl_status_over[];

/* One mask per level of the ailments that still let a fighter act. */
extern const u_long g_btl_status_allowed[];

/* The pips hung on the marker, indexed by the level. */
extern const u_long *g_btl_ail_level_marks[];

/* The actor graphics. The table of script pointers at +0x74 is reached both
   by model, when an actor is set going, and by ailment code, when its marker
   is put up - one array, indexed two ways by two callers. */
extern u_char *g_btl_actor_gfx;

#define BTL_GFX_SCRIPTS 0x74

/* The ailment code goes into the marker's +0xCE with this added. */
#define BTL_MARK_BIAS 0x40

/* From here up the attached pips stay hidden. */
#define BTL_AIL_MARK_ONLY 13

/* How long a fresh ailment is meant to last, by ailment. */
#define BTL_AIL_TURNS_LONG  10
#define BTL_AIL_TURNS_CLOAK 3
#define BTL_AIL_TURNS_SHORT 2

/* The marker's own animation while it is being put up. */
#define BTL_MARK_MOTION 0xC
#define BTL_MARK_TIMER  0x78


/* What an ailment does to the fighter's turn, one handler per Char.status and
   twenty-four of them, so the table is indexed straight by the code. Entry
   nought - a healthy fighter - is empty and BtlAilmentTakeTurn skips the call
   for it; the rest are named for the label g_status_names draws, in the order
   the codes run:

     1 HAPPY  2 PANIC   3 CHARM  4 FREEZE  5 SHOCK  6 BIND    7 SLEEP
     8 CLOSE  9 BLIND  10 UNLUCK 11 TERROR 12 GUILT 13 POISON 14 PALYZE
    15 STONE 16 SICK   17 DEAD   18 CLOAK  19 PUPPET 20 COUNTR 21 BARSAK
    22 MAD   23 WOLF

   A handler is handed the fighter and a byte to leave its answer in, and most
   of them are two instructions: the seven ailments that stop a fighter acting
   write nought there and the eight that do not touch the turn at all write
   nothing. Nothing calls one by hand - they are reached only through the
   table - which is why eight of them were sitting under one data label. */
extern void (*g_btl_ailment_turn[])();

extern void BtlAilmentTurnHappy();
extern void BtlAilmentTurnPanic();
extern void BtlAilmentTurnCharm();
extern void BtlAilmentTurnFreeze();
extern void BtlAilmentTurnShock();
extern void BtlAilmentTurnBind();
extern void BtlAilmentTurnSleep();
extern void BtlAilmentTurnClose();
extern void BtlAilmentTurnBlind();
extern void BtlAilmentTurnUnluck();
extern void BtlAilmentTurnTerror();
extern void BtlAilmentTurnGuilt();
extern void BtlAilmentTurnPoison();
extern void BtlAilmentTurnParalyse();
extern void BtlAilmentTurnStone();
extern void BtlAilmentTurnSick();
extern void BtlAilmentTurnDead();
extern void BtlAilmentTurnCloak();
extern void BtlAilmentTurnPuppet();
extern void BtlAilmentTurnCounter();
extern void BtlAilmentTurnBarsak();
extern void BtlAilmentTurnMad();
extern void BtlAilmentTurnWolf();

/* What a handler leaves in the byte. Nought is a lost turn; a redirected turn
   is run as AIMED, or as GUN when it is the member's gun that is turned; a
   fleeing one as FLEE, and Barsak leaves NONE when it finds nothing worth
   swinging at. A panicking member who runs to another cell leaves MOVED. */
#define AIL_ACT_LOST  0
#define AIL_ACT_AIMED 2
#define AIL_ACT_NONE  4
#define AIL_ACT_GUN   0xC
#define AIL_ACT_FLEE  0xE
#define AIL_ACT_MOVED 0x13

/* Hands the fighter's turn to whichever of the above its ailment picks, and
   then acts on the byte the handler left. */
extern void BtlAilmentTakeTurn(BtlActor *a, u_char *act);

/* Which palette entry a fighter's whitening starts from, one byte per Char
   key - nought for most and one for the four keys whose first entry has to be
   left alone. BtlActorClutFirst is the only reader. */
extern const u_char g_btl_key_clut_first[];
extern int BtlActorClutFirst(const BtlActor *a);

extern int  BtlStatusStops(const BtlActor *a);
extern void BtlShowAilmentMarks(int show);
extern int  BtlInflictStatus(BtlActor *a, int status);

#endif
