/* Persona 1 (JP) - what the demon leaves behind.  BTLP only.
 *   0x80073090 BtlTalkSceneGift
 *
 * BtlTalkSceneStep runs this while the top of g_btl_talk_scene is 9. One roll
 * out of 255 is taken at the top and stage 1 walks the six cumulative
 * thresholds of the row g_btl_talk_pair picks, which lands on one of stages 2
 * to 7 - a row of zero rules that gift out entirely.
 *
 * The stages are the gifts in order, and each falls through to the next when it
 * cannot be given: an item rolled against the moon, one fixed item, money,
 * experience, a heal, and finally the demon striking the acting member on its
 * way out. The money and the experience are both worked out from what the
 * Persona on offer is worth and stop at their caps, which is what makes the
 * fall-through necessary.
 */
#include <decomp/types.h>
#include <rand.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/model.h>
#include <persona/btlp/object.h>
#include <persona/btlp/offer.h>
#include <persona/common/item.h>
#include <persona/common/persona.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/talk.h>

/* The stages, which are also the gifts in the order they are tried. */
#define GIFT_ROLL  1  /* pick which one this demon leaves      */
#define GIFT_ITEM  2  /* an item rolled against the moon       */
#define GIFT_CHARM 3  /* the one fixed item                    */
#define GIFT_MONEY 4
#define GIFT_EXP   5
#define GIFT_HEAL  6
#define GIFT_NONE  7  /* nothing, which becomes the parting hit */
#define GIFT_BLESS 8  /* the larger heal                        */
#define GIFT_HIT   9  /* the demon strikes the acting member    */

/* Six thresholds a row, out of 255. */
#define GIFT_KINDS 6
#define GIFT_ROLLS 255

/* g_btl_talk_scene / g_btl_talk_stage. */
#define TALK_SCENE_NONE 0xFF
#define TALK_SCENE_SAY  0xC
#define TALK_STAGE_FREE 0
#define TALK_STAGE_RUN  1

/* Moon phases that change what an item roll can turn up. */
#define MOON_NEW       0
#define MOON_FULL      8
#define MOON_CRESCENT  1
#define MOON_GIBBOUS   5
#define MOON_WANING    9
#define MOON_OLD       13

/* The fixed item, and the message slot its name goes in. */
#define GIFT_CHARM_ITEM 0x10
#define INSERT_AMOUNT   0
#define INSERT_ITEM     3

/* Where the two rewards stop. */
#define MONEY_CAP 999999999
#define EXP_CAP   9999999

/* How much the money and the experience swing: a third of the way either side
   of the Persona's own worth, per demon in the offer. */
#define MONEY_STEP  5
#define MONEY_BASE  0x50
#define EXP_STEP    10
#define EXP_BASE    0xB4
#define REWARD_SPAN 3
#define REWARD_UNIT 100

/* The heal's own spread, and the largest the bless can be. */
#define HEAL_SPREAD 16
#define BLESS_MAX   1000

/* The pack the healing sound comes out of, and the two sound slots this scene
   opens and closes. */
#define GIFT_HEAL_PACK 0xF
#define GIFT_HEAL_SND  4
#define GIFT_HIT_SND   6

/* Frames held while the demon lands the blow. */
#define GIFT_HIT_PAUSE 0xF
#define GIFT_HIT_HOLD  0x1E

/* The motion the struck member plays. */
#define GIFT_HIT_MOTION 7

/* g_cd_busy while a read is outstanding. */
#define CD_IDLE (-1)

extern int           g_money;
extern volatile int  g_cd_busy;
extern u_char        g_btl_banks[];
extern const short   g_btl_gift_odds[][GIFT_KINDS];
extern const u_char *g_btl_talk_gift_script;
extern const u_char *g_btl_talk_item_gift_script;
extern const u_char *g_btl_talk_money_script;
extern const u_char *g_btl_talk_exp_script;
extern const u_char *g_btl_talk_heal_script;
extern const u_char *g_btl_talk_bless_script;
extern const u_char *g_btl_talk_parting_hit_script;

extern int   BtlRollDrop(void);
extern int   BtlRollUncommon(void);
extern int   BtlRollCommon(void);
extern int   BtlRoundMoney(int amount);
extern void  BtlSetInsert(int slot, u_long value);
extern int   BtlSeqState(void);
extern void  BtlLoadPackEntry(int entry);
extern void  BtlBgmOpen(void);
extern void  SsVabTransCompleted(long immediate);
extern void  BtlSePlay(int slot, int seq);
/* This unit declares both of these itself: its BtlSoundOpen takes the
   banks as a u_char array and the key as a byte, which is not the
   prototype sound.h carries. */
extern void  BtlSoundOpen(u_char *banks, int slot, u_char key);
extern void  BtlSoundClose(int slot);

#ifdef NON_MATCHING
void BtlTalkSceneGift(void)
{
    /* Kept in .rodata and copied onto the stack: which row of g_btl_gift_odds
       each pair of acts is answered with, and -1 for the pair that has none. */
    short row[10] = { 0, 1, 2, 3, -1, 4, 5, 6, 7, 8 };
    const short *p;
    BtlActor    *a;
    BtlActor    *e;
    short        roll;
    int          item;
    int          amount;
    int          hp;
    int          slot;
    int          i;

    roll = rand() % GIFT_ROLLS + 1;

    switch (g_btl_talk_stage[g_btl_talk_depth - 1]) {
    case GIFT_ROLL:
        i = 0;
        p = g_btl_gift_odds[row[g_btl_talk_pair]];
        /* A plain `while`: written as a do/while gcc peels the first test,
           which the original does not. */
        while (i < GIFT_KINDS) {
            if (roll <= *p) {
                break;
            }
            i++;
            p++;
        }
        if (i == GIFT_KINDS) {
            i = 0;
        }
        g_btl_talk_stage[g_btl_talk_depth - 1] = i + GIFT_ITEM;
        BtlSeqRun();
        break;

    case GIFT_ITEM:
        if (g_btl_moon == MOON_NEW || g_btl_moon == MOON_FULL) {
            item = BtlRollDrop();
        } else if (g_btl_moon == MOON_CRESCENT || g_btl_moon == MOON_GIBBOUS
                   || g_btl_moon == MOON_WANING || g_btl_moon == MOON_OLD) {
            item = BtlRollUncommon();
        } else {
            item = BtlRollCommon();
        }
        if ((u_short)item != 0) {
            BtlItemAdd((short)item);
            g_btl_talk_depth--;
            g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
            g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
            BtlSetInsert(INSERT_ITEM,
                         (u_long)g_item_defs[(u_short)item].name);
            BtlSeqPlay(g_btl_talk_gift_script);
            BtlSeqWaitDone();
            return;
        }
        goto try_money;

    case GIFT_CHARM:
        if ((short)BtlItemSlot(GIFT_CHARM_ITEM) != 0) {
            BtlItemAdd(GIFT_CHARM_ITEM);
            g_btl_talk_depth--;
            g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
            g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
            BtlSeqPlay(g_btl_talk_item_gift_script);
            while (BtlSeqState() != 0) {
                BtlDrawFrame();
            }
            return;
        }
    try_money:
        g_btl_talk_stage[g_btl_talk_depth - 1] = GIFT_MONEY;
        break;

    case GIFT_MONEY:
        if (g_money < MONEY_CAP) {
            amount = BtlRoundMoney(
                (rand() % REWARD_SPAN * MONEY_STEP + MONEY_BASE)
                * g_persona_data[g_btl_offer[g_btl_offer_slot].persona].price
                / REWARD_UNIT
                * g_btl_offer[g_btl_offer_slot].demons + 1);
            g_money += amount;
            if (g_money > MONEY_CAP) {
                g_money = MONEY_CAP;
            }
            BtlSetInsert(INSERT_AMOUNT, amount);
            g_btl_talk_depth--;
            g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
            g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
            BtlSeqPlay(g_btl_talk_money_script);
            while (BtlSeqState() != 0) {
                BtlDrawFrame();
            }
        } else {
            g_btl_talk_stage[g_btl_talk_depth - 1] = GIFT_EXP;
        }
        break;

    case GIFT_EXP:
        if (g_btl_actors[g_btl_actor_slot].c.unk14 < EXP_CAP) {
            g_btl_talk_depth--;
            g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
            g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
            g_btl_actors[g_btl_actor_slot].unk74 +=
                (rand() % REWARD_SPAN * EXP_STEP + EXP_BASE)
                * g_persona_data[g_btl_offer[g_btl_offer_slot].persona].exp
                / REWARD_UNIT + 1;
            BtlSetInsert(INSERT_AMOUNT, 0);
            BtlSeqPlay(g_btl_talk_exp_script);
            while (BtlSeqState() != 0) {
                BtlDrawFrame();
            }
        } else {
            g_btl_talk_stage[g_btl_talk_depth - 1] = GIFT_HEAL;
        }
        break;

    case GIFT_HEAL:
        g_btl_talk_stage[g_btl_talk_depth - 1] = GIFT_BLESS;
        BtlSetInsert(INSERT_ITEM,
                     (u_long)g_item_defs[GIFT_CHARM_ITEM].name);
        amount = rand();
        slot = g_btl_actor_slot;
        hp = g_btl_actors[slot].stat[4]
             + (g_btl_enemies[g_btl_talk_target].stat[4] >> 1)
             + amount % HEAL_SPREAD
             + g_btl_actors[slot].c.hp;
        if (hp > g_btl_actors[slot].c.hp_max) {
            hp = g_btl_actors[slot].c.hp_max;
        }
        g_btl_actors[slot].c.hp = hp;
        BtlLoadPackEntry(GIFT_HEAL_PACK);
        while (g_cd_busy != CD_IDLE) {
            BtlDrawFrame();
        }
        BtlBgmOpen();
        SsVabTransCompleted(1);
        BtlSePlay(GIFT_HEAL_SND, 0);
        BtlSeqPlay(g_btl_talk_heal_script);
        BtlSeqRun();
        i = GIFT_HEAL_SND;
        goto close_sound;

    case GIFT_NONE:
        g_btl_talk_stage[g_btl_talk_depth - 1] = GIFT_HIT;
        break;

    case GIFT_BLESS:
        g_btl_talk_depth--;
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
        g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
        amount = rand();
        BtlSetInsert(INSERT_AMOUNT, amount % BLESS_MAX);
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_SAY;
        g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_RUN;
        g_btl_talk_depth++;
        BtlSeqPlay(g_btl_talk_bless_script);
        while (BtlSeqState() != 0) {
            BtlDrawFrame();
        }
        hp = amount % BLESS_MAX + g_btl_actors[g_btl_actor_slot].c.hp;
        if (hp > g_btl_actors[g_btl_actor_slot].c.hp_max) {
            hp = g_btl_actors[g_btl_actor_slot].c.hp_max;
        }
        g_btl_actors[g_btl_actor_slot].c.hp = hp;
        break;

    case GIFT_HIT:
        g_btl_talk_depth--;
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
        g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
        amount = rand();
        amount = g_persona_data[
                     g_btl_offer[g_btl_offer_slot].persona].level + 1
                 + amount % HEAL_SPREAD;
        BtlSetInsert(INSERT_AMOUNT, amount);
        slot = g_btl_actor_slot;
        hp = g_btl_actors[slot].c.hp - amount;
        g_btl_actors[slot].c.hp = hp;
        if (hp < 1) {
            g_btl_actors[slot].c.hp = 1;
        }
        e = &g_btl_enemies[g_btl_talk_target];
        g_btl_actors[g_btl_actor_slot].unk84 = 0;
        BtlObjSetScript(e->obj,
                        e->obj->scripts[g_btl_models[e->c.key].talk]);
        BtlRunFrames(GIFT_HIT_PAUSE);
        BtlObjSetScript(e->obj,
                        e->obj->scripts[g_btl_models[e->c.key].pad02[0]]);
        slot = g_btl_actor_slot;
        a = &g_btl_actors[slot];
        BtlSoundOpen(g_btl_banks, GIFT_HIT_SND, a->c.key);
        BtlDrawFrame();
        BtlSePlay(GIFT_HIT_SND, 0);
        BtlObjSetMotion(a->obj, GIFT_HIT_MOTION);
        while (a->obj->motion != 0) {
            BtlDrawFrame();
        }
        BtlRunFrames(GIFT_HIT_HOLD);
        BtlSeqPlay(g_btl_talk_parting_hit_script);
        BtlSeqWaitDone();
        i = GIFT_HIT_SND;
    close_sound:
        BtlSoundClose(i);
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/talkscenegift", BtlTalkSceneGift);
#endif

