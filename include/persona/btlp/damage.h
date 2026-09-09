/* Persona 1 (JP) - what an attack is worth, and whether it lands.
 *
 * Four rolls and a table, in the order an attack goes through them: does it
 * land at all, is it a critical, how much raw damage the numbers come to, and
 * finally what the target's own make-up does to that.
 *
 * The affinity chart is two codes to a byte, high nibble first, so one row
 * covers twice as many elements as it has bytes. A row is picked by the
 * target's resistance class - the equipped Persona's own, kept on the actor -
 * and a column by the element the attack carries. The code that comes out is
 * an index into g_btl_affinity_scale, which says both how much of the damage
 * survives, in eighths, and what the fighter is seen to do about it.
 *
 * The top codes have no row in that table: from BTL_AFFINITY_NULL up the
 * attack does nothing, and from BTL_AFFINITY_REPEL up it comes back. Their
 * scale entries are still read, and still scale the damage, which is what
 * lets a repelled attack carry a number.
 */
#ifndef PERSONA_BTLP_DAMAGE_H
#define PERSONA_BTLP_DAMAGE_H

#include <decomp/types.h>
#include <persona/btlp/actor.h>

/* What one affinity code does. */
typedef struct {
    /* 0x0 */ short scale;   /* eighths of the damage that lands */
    /* 0x2 */ short react;   /* the reaction the fighter plays          */
} BtlAffinity;               /* 4 bytes */

/* Bytes to a row of the chart, so twice that many elements. */
#define BTL_AFFINITY_ROW 0x11

/* The two codes with no reaction of their own. */
#define BTL_AFFINITY_NULL  10
#define BTL_AFFINITY_REPEL 13

/* What BtlApplyAffinity answers for those. */
#define BTL_REACT_NULL  (-1)
#define BTL_REACT_REPEL (-2)

/* The damage is divided by this before the scale multiplies it back. */
#define BTL_AFFINITY_UNIT 8.0

/* Ailments that leave the target unable to get out of the way, as a mask over
   the status code: FREEZE, SHOCK, BIND, SLEEP, PALYZE and STONE. */
#define BTL_HELPLESS 0xC0F0

/* Where an even matchup starts, before accuracy and evade pull it about, and
   the widest a roll can come out. */
#define BTL_HIT_EVEN 0xE5
#define BTL_ROLL_MAX 0xFF

extern u_char      g_btl_affinity[];
extern BtlAffinity g_btl_affinity_scale[];
extern int         g_btl_affinity_last;

extern int BtlRollHit(int ownCount, int accuracy, int selfStatus, int foeCount,
                      int evade, int foeStatus);
extern int BtlRollCritical(BtlActor *actor, BtlActor *foe);
extern int BtlDamageBase(int power, int guard);
extern int BtlDamageBoosted(int power, double bonus, int guard);
extern int BtlApplyAffinity(int *damage, int element, int resist);

#endif
