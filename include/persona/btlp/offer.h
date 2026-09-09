/* Persona 1 (JP) - the Personas a battle offers.
 *
 * Three records, and the code that touches them agrees on three fields: a mask
 * whose lowest set bit ranks the offer, a short that is zero while the slot is
 * free, and the id of the Persona the offer would hand over. Whether the party
 * can take it is a separate question - BtlStockHolds asks whether they already
 * have that one, and BtlStockHasRoom whether there is a slot for it.
 */
#ifndef PERSONA_BTLP_OFFER_H
#define PERSONA_BTLP_OFFER_H

#include <decomp/types.h>
#include <persona/btlp/actor.h>

typedef struct {
    /* 0x00 */ u_long kinds;    /* the lowest set bit ranks the offer */
    /* 0x04 */ u_char voice;    /* which set of lines this demon speaks from;
                                   BtlBuildOffers copies it, and `flags` below,
                                   out of the demon's g_btl_demon_talk_profiles
                                   row, and BtlTalkSceneTrade indexes
                                   g_btl_talk_full_lines with it            */
    /* 0x05 */ u_char species;  /* the demon this offer is of; every enemy
                                   folded into it is the same one          */
    /* 0x06 */ u_char demons;   /* how many enemies were folded into this offer;
                                   BtlBuildOffers bumps it once per enemy and
                                   BtlTalkSceneGift multiplies the money the
                                   demon leaves behind by it                */
    /* 0x07 */ u_char status[BTL_ENEMIES];
                                /* each folded enemy's ailment and how deep
                                   it is, copied in as it joins            */
    /* 0x10 */ u_char ail_level[BTL_ENEMIES];
    /* 0x19 */ u_char level;   /* measured against g_btl_talk_level    */
    /* 0x1A */ u_short talkers; /* of those, the ones with no ailment  */
    /* 0x1C */ u_short used;    /* enemies this offer involves; zero
                                   while the slot is free              */
    /* 0x1E */ u_char pad1E[2];
    /* 0x20 */ u_long damage;   /* what this offer's demons have taken, added
                                   to a hit at a time by BtlOfferScoreEnemy
                                   and never cleared. BtlTalkSceneDemand
                                   prices the blood demand at the offer's
                                   level plus an eighth of it, plus a roll
                                   of sixteen.                            */
    /* 0x24 */ long   hp;       /* the demons' HP added up as they joined,
                                   which is what a round's damage is scaled
                                   against when it moves a gauge          */
    /* 0x28 */ u_long hp_now;    /* and what they have left, summed the same
                                   way as the round is built              */
    /* 0x2C */ u_long hp_was;   /* what hp_now came to the round before, so
                                   the difference is what this round cost
                                   them                                   */
    /* 0x30 */ u_char persona;  /* what it would hand over            */
    /* 0x31 */ u_char name[0xB];  /* its name, ready to drop into a message */
    /* 0x3C */ u_short flags;   /* bit 1 keeps a sum off a round hundred, and
                                   see the two OFFER_ bits below          */
    /* 0x3E */ short  mood[4];  /* what `kinds` is a summary of       */
    /* 0x46 */ u_char pad46[2];
} BtlOffer;                     /* 0x48 bytes */

#define BTL_OFFERS   3
#define BTL_MOODS    4
#define BTL_NO_OFFER 0xFFFF

/* The two levels the gauges are read at. Bits 0..3 of `kinds` say which gauges
   have reached the first and bits 4..7 which have reached the second, so the
   lowest set bit says how well the best gauge is doing. */
#define BTL_MOOD_STRONG 0x5F
#define BTL_MOOD_WEAK   0x46

/* Two flags bits the round's damage is read through: the first says the
   damage counts at all, the second sends it to the frightened gauge rather
   than the angry one - as does a demon nine or more levels under the party. */
#define OFFER_SCORED  0x8000

/* Set on an offer that has already been talked to, so a fresh contact
   passes over it. */
#define OFFER_TAKEN   0x2000
#define OFFER_FEARFUL 0x20

/* The four gauges. Damage lands on one of the middle two. */
#define MOOD_ANGRY 1
#define MOOD_AFRAID 2

/* What a gauge's worth of damage is, before the demons' own HP scales it. */
#define BTL_MOOD_FULL 90

extern BtlOffer g_btl_offer[];

/* One bit per offer, set while that offer still has someone able to answer,
   and one per party member the contact lookup answered for. Both are rebuilt
   from scratch at the top of a round rather than kept up to date. */
extern u_char g_btl_offer_live;
extern u_char g_btl_member_matched;

/* Set while a negotiation is open. */
extern u_char g_btl_talking;

/* What a round's contacts are worth, one word per offer, folded into the
   offers' moods by BtlOfferApplyScores and cleared as it goes. */
extern int g_btl_talk_scratch[];

extern void BtlBuildOffers(void);
extern void BtlOfferScoreEnemy(int enemy, int amount);
extern void BtlOfferApplyScores(void);
extern int  BtlOfferLevelTest(int test, u_short slot);
extern int  BtlOfferMarkStrong(void);
extern int  BtlPhase(void);
extern void BtlTalkResume(void);
extern void BtlMarkOffersLive(void);
extern void BtlMarkMembersMatched(void);
extern int  BtlTalkBlocked(void);
extern void BtlOfferMarkEnemies(u_char status, u_char level);

#endif
