#ifndef PERSONA_BTLP_STRIKE_H
#define PERSONA_BTLP_STRIKE_H

/* Persona 1 (JP) - how a blow lands, which both sides play out the same way:
 * BtlMemberStrike (memberact.c) for a member's swing and BtlEnemyStrike
 * (enemymotion2.c) for an enemy's attack.
 *
 * The blow is rolled against the target's evasion; what lands is capped and
 * passed through the target's affinity, whose answer picks one of three ends -
 * absorbed, it heals; nulled, the striker reels from its own blow; otherwise
 * the target takes it.
 */
#include <persona/btlp/actor.h>

/* The voice slot a blow is sounded in, the sounds out of the shared slot, and
   the motions it leaves each side on. */
#define STRIKE_VOICE        6
#define STRIKE_SE_SLOT      2
#define STRIKE_SE_MISS      8
#define STRIKE_SE_CRITICAL  9
#define STRIKE_SE_NULL      0xA
#define STRIKE_SE_REPEL     0xB
#define STRIKE_MOTION_HURT  7
#define STRIKE_MOTION_DOWN  8
#define STRIKE_MOTION_HEAL  0xF
#define STRIKE_MOTION_REEL  0x10
#define STRIKE_MOTION_MISS  0x11

/* The wards that keep a blow off altogether. */
#define STRIKE_BLOCKED (BTL_ACTOR_WARD_8E | BTL_ACTOR_WARD_8F | BTL_ACTOR_5E)

/* The most a blow takes, how far the strike art is lifted, and how long the
   striker waits behind a blow that missed or took nothing. */
#define STRIKE_CAP  9999
#define STRIKE_LIFT 0x180000
#define STRIKE_HOLD 0x1E

/* A member hit where it stands is marked further down the field than the
   square it is on. */
#define STRIKE_MEMBER_DROP 0xC80000

#endif
