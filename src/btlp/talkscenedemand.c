/* Persona 1 (JP) - the demon naming its price.  BTLP only.
 *   0x8006A374 BtlTalkSceneDemand
 *
 * Scene 0x11 of BtlTalkSceneStep. The demon asks for one of four things and
 * the player agrees or refuses.
 *
 * Stage 1 works out what it could ask for. The demon's kind, a byte of its
 * PersonaData, picks a row of g_btl_demand_odds_by_kind - four rising
 * thresholds, one a demand - and that row is copied out and then pruned: the
 * money entry goes if the party cannot pay the sum asked, the charm entry if
 * they have none and have been asked once already, the item entry if they hold
 * nothing the demon will take. A roll then walks what is left and takes the
 * first entry it does not exceed, so a demand the party cannot meet is never
 * made.
 *
 * Stage 2 reads the answer. Agreeing pays: money comes off g_money, an item
 * out of the pack, blood off the acting member's own hit points and onto the
 * weakest demon still standing. Refusing, or agreeing with nothing to give,
 * picks a worse line. Either way the line taken moves the mood gauge it names,
 * and a gauge that fills carries the scene into BtlTalkAnswer.
 */
#include <decomp/types.h>
#include <rand.h>
#include <decomp/include_asm.h>
#include <decomp/libc.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/offer.h>
#include <persona/common/item.h>
#include <persona/common/persona.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/talk.h>

/* The four things it can ask for. */
#define DEMAND_MONEY 0
#define DEMAND_CHARM 1
#define DEMAND_ITEM  2
#define DEMAND_BLOOD 3
#define DEMAND_KINDS 4

/* The item slot the charm lives in, and the run of sixteen it will take. */
#define DEMAND_CHARM_SLOT 0x10
#define DEMAND_ITEM_FIRST 0x56
#define DEMAND_ITEM_COUNT 0x10

/* g_btl_talk_asked remembers which demands have been made once already. */
#define ASKED_MONEY 1
#define ASKED_CHARM 2

/* Where a scene's lines and scripts sit in the scratch buffer. */
#define TALK_SCRIPTS_AT 0x508
#define TALK_LINES_AT   0x512
#define TALK_KIND_ROW   0x18

/* BtlMenuChoice's two non-answers, and the answer that means yes. */
#define MENU_WAIT   (-0x100)
#define MENU_CANCEL (-1)
#define DEMAND_YES  0

/* What the blood demand is worth, by how far up the item run it landed. */
#define BLOOD_HIGH 0x5A
#define BLOOD_MID  0x4B
#define BLOOD_LOW  0x32
#define BLOOD_CUT1 0x58
#define BLOOD_CUT2 0x5E

/* A line whose weight is this or more is spoken aloud. */
#define TALK_VOICE_MIN 10

/* The talk scratch buffer, reached by address rather than through a symbol. */
#define g_btl_scratch ((u_long *)0x801C0000)
extern int       g_btl_demand_kind;
extern int       g_btl_demand_money;
extern int       g_btl_demand_item;
extern int       g_btl_demand_answer;
extern short     g_btl_demand_odds[];
extern short     g_btl_demand_roll;
extern const short g_btl_demand_odds_by_kind[][DEMAND_KINDS];
extern PersonaData g_persona_data[];
extern int       g_btl_menu_aside;
extern const u_char *g_btl_talk_no_money_script;
extern const u_char *g_btl_talk_no_charm_script;
/* The party purse, reached by address here as it is everywhere else. */
#define g_money (*(int *)0x801F2674)

extern int   BtlRoundMoney(int sum);
extern void  BtlSetInsert(int which, const void *what);
extern void  BtlMenuOpen2(const u_char *menu);
extern void  BtlMenuAsideToggle(void);
extern int   BtlMenuChoice(void);
extern int   BtlMenuState(void);
extern void  BtlMenuUpdate(void);
extern void  BtlMenuDismiss(void);
extern void  BtlTalkUpdate(void);
extern void  BtlFaceLoad(int face, int slot);
extern void  BtlFaceOpen(int x, int y, int scale);
extern void  BtlIndicatorIcon(void);
extern void  BtlTalkTakeLine(short line, short kind, short gauge, short weight,
                             short item);
extern void  BtlHighlightBegin(int gauge);
extern void  BtlQueueVoice(int gauge, int which);
extern void  BtlTalkPerform(void);
extern void  BtlRefreshMoodGauges(void);
extern int   BtlOfferRank(int slot);
extern void  BtlTalkAnswer(int slot, int gauge);
extern void  BtlPanelSetImage(int on, u_char image);
extern void  BtlPushRecent(int gauge);

/* The scene's own tables live at a per-scene offset the scratch header holds. */
#define TALK_BASE ((char *)g_btl_scratch + g_btl_scratch[2])

#ifdef NON_MATCHING
void BtlTalkSceneDemand(void)
{
    const u_char *p;
    u_short items[DEMAND_ITEM_COUNT];
    BtlOffer *offer;
    BtlActor *e;
    const char *base;
    int   answer;
    int   line;
    int   gauge;
    int   weight;
    int   take;
    int   roll;
    int   i;
    int   n;
    int   least;
    int   worst;
    int   used;
    u_short item;
    short slot;

    base = TALK_BASE;
    offer = &g_btl_offer[g_btl_offer_slot];
    switch (g_btl_talk_stage[g_btl_talk_depth - 1]) {
    case 1:
        g_btl_talk_stage[g_btl_talk_depth - 1] = 2;
        memmove((u_char *)g_btl_demand_odds,
                (u_char *)g_btl_demand_odds_by_kind[
                    g_persona_data[g_btl_offer[g_btl_offer_slot].persona].arcana],
                sizeof(g_btl_demand_odds_by_kind[0]));

        /* One turn of the loop per thing it could ask for, pruning the ones
           the party cannot meet. The blood demand has nothing to prune, and
           the item scan below runs the counter past the end anyway. */
        for (i = 0; i < DEMAND_KINDS; i++) {
            switch (i) {
            case DEMAND_MONEY:
                n = rand();
                g_btl_demand_money =
                    g_persona_data[g_btl_offer[g_btl_offer_slot].persona].price
                    * ((n % 3) * 5 + 0x50)
                    / 100 * 2 + 1;
                if (g_money < g_btl_demand_money
                    && (g_btl_talk_asked & ASKED_MONEY) != 0) {
                    g_btl_demand_odds[DEMAND_MONEY] = 0;
                }
                g_btl_demand_money = BtlRoundMoney(g_btl_demand_money);
                break;
            case DEMAND_CHARM:
                if ((u_short)BtlItemSlot(DEMAND_CHARM_SLOT) == 1
                    && (g_btl_talk_asked & ASKED_CHARM) != 0) {
                    g_btl_demand_odds[DEMAND_CHARM] = 0;
                }
                break;
            case DEMAND_ITEM:
                g_btl_demand_item = 0;
                for (i = 0; i < DEMAND_ITEM_COUNT; i++) {
                    item = DEMAND_ITEM_FIRST + i;
                    if ((u_short)BtlItemSlot(item) != 1) {
                        items[g_btl_demand_item] = item;
                        g_btl_demand_item++;
                    }
                }
                if (g_btl_demand_item == 0) {
                    g_btl_demand_odds[DEMAND_ITEM] = 0;
                }
                break;
            }
        }

        n = rand();
        roll = n % 0x100;
        g_btl_demand_roll = roll;
        g_btl_demand_kind = 0;
        do {
            if (g_btl_demand_odds[g_btl_demand_kind] != 0
                && g_btl_demand_odds[g_btl_demand_kind] >= roll) {
                break;
            }
            g_btl_demand_kind++;
        } while (g_btl_demand_kind < DEMAND_KINDS);
        if (g_btl_demand_kind == DEMAND_KINDS) {
            g_btl_demand_kind = DEMAND_BLOOD;
        }

        switch (g_btl_demand_kind) {
        case DEMAND_MONEY:
            BtlSetInsert(0, (const void *)g_btl_demand_money);
            break;
        case DEMAND_ITEM:
            n = rand();
            g_btl_demand_item = items[n % g_btl_demand_item];
            BtlSetInsert(3, g_item_defs[g_btl_demand_item].name);
            break;
        }

        p = (const u_char *)g_btl_scratch + g_btl_scratch[0];
        BtlSeqPlay(p + ((const int *)p)[
            *(u_short *)(base + g_btl_demand_kind * TALK_KIND_ROW
                         + TALK_SCRIPTS_AT)]);
        BtlSeqRun();
        /* Both arms work the member's row out the same way; only the blood
           demand steps two menus on. Written out rather than folded. */
        if (g_btl_demand_kind == DEMAND_BLOOD) {
            take = (g_btl_actors[g_btl_actor_slot].c.key - 1) * 4 + 2;
        } else {
            take = (g_btl_actors[g_btl_actor_slot].c.key - 1) * 4;
        }
        /* Two menus per party member: the plain one, and the one that
           asks for blood eight bytes on. */
        BtlMenuOpen2((const u_char *)(take * 4 + 0x800C6DAC));
        BtlFaceLoad(g_btl_actors[g_btl_actor_slot].c.key, 0);
        BtlFaceOpen(0x3C, 0x70, 0x1000);
        BtlIndicatorIcon();
        break;
    case 2:
        BtlMenuAsideToggle();
        answer = BtlMenuChoice();
        if (answer == MENU_WAIT) {
            return;
        }
        if (answer == MENU_CANCEL) {
            return;
        }
        BtlIndicatorClear();
        g_btl_menu_aside = 0;
        g_btl_demand_answer = BtlMenuChoice();
        BtlFaceClose();
        BtlRunFrames(0xF);
        BtlMenuDismiss();
        while (BtlMenuState() != 0) {
            BtlMenuUpdate();
            BtlTalkUpdate();
            BtlDrawFrame();
        }
        BtlRunFrames(0xF);

        line = 2;
        /* The mood flag is only raised on the paths where the demand went
           unmet - paying it, or handing the charm over, leaves it alone. */
        switch (g_btl_demand_kind) {
        case DEMAND_MONEY:
            if (g_btl_demand_answer == DEMAND_YES) {
                if (g_btl_demand_money > g_money) {
                    BtlSeqPlay(g_btl_talk_no_money_script);
                    BtlSeqRun();
                    n = rand();
                    line = n % 2 + 3;
                } else {
                    g_money -= g_btl_demand_money;
                    if (g_money < 0) {
                        g_money = 0;
                    }
                    break;
                }
            } else {
                n = rand();
                line = n % 2 + 5;
            }
            g_btl_talk_asked |= ASKED_MONEY;
            break;
        case DEMAND_CHARM:
            if (g_btl_demand_answer == DEMAND_YES) {
                if ((u_short)BtlItemSlot(DEMAND_CHARM_SLOT) == 1) {
                    BtlSeqPlay(g_btl_talk_no_charm_script);
                    BtlSeqRun();
                    n = rand();
                    line = n % 2 + 3;
                } else {
                    BtlItemRemove(DEMAND_CHARM_SLOT);
                    break;
                }
            } else {
                n = rand();
                line = n % 2 + 5;
            }
            g_btl_talk_asked |= ASKED_CHARM;
            break;
        case DEMAND_ITEM:
            if (g_btl_demand_answer == DEMAND_YES) {
                BtlItemRemove(g_btl_demand_item);
            } else {
                n = rand();
                line = n % 2 + 5;
            }
            /* An item demand carries its own weight, set by how far up the
               run of sixteen the item landed. The first test is dead: every
               item below BLOOD_CUT1 is below BLOOD_CUT2 as well, so BLOOD_HIGH
               is written and then overwritten. Left as the original has it. */
            weight = BLOOD_LOW;
            if (g_btl_demand_item < BLOOD_CUT1) {
                weight = BLOOD_HIGH;
            }
            if (g_btl_demand_item < BLOOD_CUT2) {
                weight = BLOOD_MID;
            }
            break;
        case DEMAND_BLOOD:
            if (g_btl_demand_answer != DEMAND_YES) {
                n = rand();
                line = n % 2 + 5;
                break;
            }
            least = 999;
            worst = 0;
            n = rand();
            /* What it costs the member who agreed: the demon's own level and
               an eighth of unk20, plus a roll of sixteen. */
            take = offer->level + (offer->damage >> 3) + n % 0x10 + 1;
            g_btl_actors[g_btl_actor_slot].unk84 = 0;
            g_btl_actors[g_btl_actor_slot].c.hp -= take;
            if (g_btl_actors[g_btl_actor_slot].c.hp < 1) {
                g_btl_actors[g_btl_actor_slot].c.hp = 1;
            }
            used = offer->used;
            e = g_btl_enemies;
            i = 0;
            do {
                if (((used >> i) & 1) != 0 && e->c.hp <= least) {
                    least = e->c.hp;
                    worst = i;
                }
                i++;
                e++;
            } while (i < 9);
            e = &g_btl_enemies[worst];
            take += e->c.hp;
            if (e->c.hp_max < take) {
                take = e->c.hp_max;
            }
            e->c.hp = take;
            break;
        }

        n = *(u_short *)(base + line * 2
                         + g_btl_demand_kind * TALK_KIND_ROW + TALK_LINES_AT);
        gauge = n & 0xFF;
        if (g_btl_demand_kind == DEMAND_ITEM) {
            /* An item demand brought its own weight; the table only says
               whether the line carries one at all. */
            if ((n >> 8) == 0) {
                weight = 0;
            }
        } else {
            weight = n >> 8;
        }
        slot = line - 2;
        BtlTalkTakeLine(slot, g_btl_demand_kind, gauge, weight, g_btl_demand_item);
        if (weight != 0) {
            BtlHighlightBegin(gauge);
        }
        p = (const u_char *)g_btl_scratch + g_btl_scratch[0];
        BtlSeqPlay(p + ((const int *)p)[
            *(u_short *)(base + line * 2
                         + g_btl_demand_kind * TALK_KIND_ROW + TALK_SCRIPTS_AT)]);
        if (weight > TALK_VOICE_MIN - 1) {
            BtlQueueVoice(gauge & 0xFF, 0);
        }
        if (g_btl_demand_kind == DEMAND_BLOOD
            && g_btl_demand_answer == DEMAND_YES) {
            BtlTalkPerform();
        }
        g_btl_offer[g_btl_offer_slot].mood[gauge] += weight;
        BtlRefreshMoodGauges();
        if (BtlOfferRank(g_btl_offer_slot) == 1) {
            if (((g_btl_panel_gauges >> gauge) & 1) != 0
                && (g_btl_offer[g_btl_offer_slot].kinds & (1 << gauge)) != 0
                && weight > TALK_VOICE_MIN - 1) {
                BtlSeqRun();
                BtlEndTalking();
                g_btl_talk_depth--;
                g_btl_talk_scene[g_btl_talk_depth] = 0xFF;
                g_btl_talk_stage[g_btl_talk_depth] = 0;
                BtlTalkAnswer(g_btl_offer_slot, gauge);
                return;
            }
            BtlSeqWaitDone();
            BtlEndTalking();
            if ((g_btl_offer[g_btl_offer_slot].kinds & (1 << gauge)) != 0) {
                g_btl_panel_gauges |= 1 << gauge;
                BtlPanelSetImage(1, g_btl_panel_gauges);
                BtlPushRecent(gauge);
            }
        } else {
            BtlSeqWaitDone();
            BtlEndTalking();
        }
        g_btl_talk_depth--;
        g_btl_talk_scene[g_btl_talk_depth] = 0xFF;
        g_btl_talk_stage[g_btl_talk_depth] = 0;
        break;
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/talkscenedemand", BtlTalkSceneDemand);
#endif

