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

extern int  BtlStatusStops(const BtlActor *a);
extern void BtlShowAilmentMarks(int show);
extern int  BtlInflictStatus(BtlActor *a, int status);

#endif
