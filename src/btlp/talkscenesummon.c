/* Persona 1 (JP) - summoning a Persona at the demon.  BTLP only.
 *   0x8006F14C BtlTalkSceneSummon
 *
 * Scene 0xB of BtlTalkSceneStep. The acting member calls its Persona out in
 * front of the demon and the demon reacts to it.
 *
 * The scene only runs if the member has an entry to summon with, is not
 * blocked, nobody is already talking, and the stock has room; otherwise it pops
 * straight off again.
 *
 * BtlOfferAnswer decides how the demon takes it, and the finer answer it leaves
 * in g_btl_summon_answer picks a row of g_btl_summon_lines - nine script
 * indices covering every way the scene can go. One outcome summons the Persona
 * and, if the party already holds this one, ends the negotiation there. Two
 * summons it, the demon hands its own Persona over and leaves, and the battle
 * goes back to phase 0.
 */
#include <decomp/types.h>
#include <rand.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/offer.h>
#include <persona/common/persona.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/text.h>

/* How many entries a member can be carrying. */
#define SUMMON_ENTRIES 3

/* The stages this scene walks, in the order it reaches them. */
#define SUMMON_OPEN   1   /* ask the demon and see how it takes it     */
#define SUMMON_SPEAK  2   /* it is impressed - say so                  */
#define SUMMON_OFFER  3   /* put the trade to the player               */
#define SUMMON_FULL   4   /* no room, so pick something to give up     */
#define SUMMON_TAKE   5   /* the swap goes through                     */
#define SUMMON_REFUSE 9   /* it is not impressed                       */
#define SUMMON_LEAVE 10   /* it goes                                   */

/* Which of the row's nine indices each pair of lines starts at. */
#define SUMMON_LINE_OPEN   0
#define SUMMON_LINE_GOOD   1
#define SUMMON_LINE_GOOD2  2
#define SUMMON_LINE_PART   3   /* it turns the offer down and goes */
#define SUMMON_LINE_PART2  4
#define SUMMON_LINE_BAD    5
#define SUMMON_LINE_BAD2   6
#define SUMMON_LINE_NO     7
#define SUMMON_LINE_NO2    8

/* g_btl_talk_scene / g_btl_talk_stage, for a level nothing is running on. */
#define TALK_SCENE_NONE 0xFF
#define TALK_STAGE_FREE 0

/* What the offer's kinds word is stamped with once the demon has been won. */
#define OFFER_CONTACTED 0x10000002
#define OFFER_ALL_MOODS 0x5F

extern u_char    g_btl_talking;
extern short     g_btl_stock_choice;
extern u_long    g_btl_scratch[];
extern u_char    g_btl_summon_lines[][9];
extern u_char   *g_btl_summon_line;
extern int       g_btl_summon_answer;
extern int       g_btl_summon_obj;
extern u_char    g_btl_talk_leaving;
extern PersonaData g_persona_data[];
extern const u_char *g_btl_arcana_names[];
extern const u_char *g_btl_talk_full_lines[];
extern const u_char *g_btl_talk_left_script;
extern const u_char *g_btl_talk_pick_gift_script;
extern const u_char *g_btl_talk_gift_shown_script;
extern const u_char *g_btl_talk_gift_taken_script;
extern const u_char *g_btl_talk_joined_script;
extern const u_char  g_btl_talk_give_menu[];

extern int   BtlStockHolds(const BtlOffer *offer);
extern void  BtlStockAdd(int persona);
extern void  BtlStockRemove(int slot);
extern int   BtlOfferAnswer(u_short actor, int slot, int *answer);
extern int   BtlOfferLevelTest(int level, u_short slot);
extern int   BtlSummonPersona(int actor, int mode);
extern void  BtlSeekFile(int file);
extern void  BtlLoadScratch(int a, int b);
extern void  BtlTalkHide(void);
extern void  BtlTalkLeave(int obj);
extern void  BtlTalkStockMenu(void);
extern void  BtlTalkEndEffect(void);
extern void  BtlTalkUpdate(void);
extern void  BtlLoadActorGfx(int actor);
extern void  BtlSetInsert(int which, const void *what);
extern void  BtlBeginAction(void);
extern void  BtlBoxOpen(int w, int x, int y, int kind);
extern void  BtlMenuOpen2(const u_char *menu);
extern int   BtlMenuChoice(void);
extern int   BtlMenuState(void);
extern void  BtlMenuUpdate(void);
extern void  BtlMenuAsideToggle(void);
extern void  BtlMenuDismiss(void);
extern void  BtlIndicatorIcon(void);
extern void  BtlShowAilmentMarks(int on);
extern void  BtlHudShow(void);
extern char  BtlHudState(void);
extern int   BtlAnyEnemy(void);

/* A scene's scripts are reached through the scratch buffer's own first word:
   it holds where the table starts, and the table holds where each script does. */
#define TALK_SCRIPT(n) \
    ((const u_char *)g_btl_scratch + g_btl_scratch[0] \
     + ((const int *)((const char *)g_btl_scratch + g_btl_scratch[0]))[n])

#ifdef NON_MATCHING
void BtlTalkSceneSummon(void)
{
    BtlOffer *offer;
    int    slot;
    int    i;
    int    answer;
    int    choice;
    int    more;
    u_char pick;

    i = 0;
    offer = &g_btl_offer[g_btl_offer_slot];
    slot = g_btl_actor_slot;
    for (i = 0; i < SUMMON_ENTRIES; i++) {
        if (g_btl_actors[slot].c.entry == i) {
            break;
        }
    }
    if (i == SUMMON_ENTRIES
        || g_btl_actors[slot].c.blocked != 0
        || g_btl_talking != 0
        || BtlStockHasRoom() != 1) {
        goto pop;
    }

    switch (g_btl_talk_stage[g_btl_talk_depth - 1]) {
    case SUMMON_OPEN:
        answer = BtlOfferAnswer(g_btl_actor_slot, g_btl_offer_slot,
                                &g_btl_summon_answer);
        switch (answer) {
        case 0:
            /* It will not even hear the offer: unwind the scene. */
            goto pop;
        case 1:
            BtlPanelClose();
            BtlTalkHide();
            g_btl_summon_obj = BtlSummonPersona(g_btl_actor_slot, 0);
            BtlSeekFile(0x1C);
            BtlLoadScratch(0, 0);
            g_btl_summon_line = g_btl_summon_lines[g_btl_summon_answer];
            if (BtlStockHolds(&g_btl_offer[g_btl_offer_slot]) != 0) {
                BtlSeqPlay(TALK_SCRIPT(g_btl_summon_line[SUMMON_LINE_OPEN]));
                BtlSeqWaitDone();
                BtlTalkersLeave();
                BtlTextOpen(g_btl_talk_left_script, 0x28, 0x92);
                BtlBoxOpen(0x11, 0xA0, 0x92, 0);
                BtlWaitAnyKey();
                g_btl_talk_depth--;
                g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
                g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
                BtlShowAilmentMarks(1);
                BtlFaceClose();
                BtlPanelClose();
                BtlBoxClose();
                BtlSeqClear();
                BtlHudHide();
                BtlEnemiesReset();
                BtlPartyReset();
                g_btl_phase = 2;
                return;
            }
            /* Its own level is enough, or it is within the wider band and
               a one-in-sixteen roll goes its way. */
            if (BtlOfferLevelTest(0, g_btl_offer_slot) == 1
                || (BtlOfferLevelTest(2, g_btl_offer_slot) == 1
                    && (rand() & 0xF) == 0)) {
                g_btl_talk_stage[g_btl_talk_depth - 1] = SUMMON_SPEAK;
            } else {
                g_btl_talk_stage[g_btl_talk_depth - 1] = SUMMON_REFUSE;
            }
            return;
        case 2:
                BtlPanelClose();
            BtlTalkHide();
            g_btl_summon_obj = BtlSummonPersona(g_btl_actor_slot, 1);
            g_btl_summon_line = g_btl_summon_lines[g_btl_summon_answer];
            BtlSeekFile(0x1C);
            BtlLoadScratch(0, 0);
            offer->mood[1] = OFFER_ALL_MOODS;
            g_btl_talk_depth--;
            offer->kinds |= OFFER_CONTACTED;
            g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
            g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
            BtlSeqPlay(TALK_SCRIPT(g_btl_summon_line[SUMMON_LINE_PART]));
            if (g_btl_summon_line[SUMMON_LINE_PART2] != 0) {
                BtlSeqRun();
                BtlSeqPlay(TALK_SCRIPT(g_btl_summon_line[SUMMON_LINE_PART2]));
        }
        BtlSeqWaitDone();
        BtlTalkLeave(g_btl_summon_obj);
        while (g_btl_talk_leaving != 0) {
            BtlDrawFrame();
        }
        i = 0;
        BtlOfferFinish();
        do {
            i++;
            BtlDrawFrame();
        } while (i < 0xB4);
        BtlShowAilmentMarks(1);
        BtlFaceClose();
        BtlPanelClose();
        BtlBoxClose();
        BtlSeqClear();
        BtlHudHide();
        BtlEnemiesReset();
        BtlPartyReset();
        g_btl_phase = 2;
        return;
        }
        return;

    case SUMMON_SPEAK:
        g_btl_talk_stage[g_btl_talk_depth - 1] = SUMMON_OFFER;
        BtlSeqPlay(TALK_SCRIPT(g_btl_summon_line[SUMMON_LINE_GOOD]));
        if (g_btl_summon_line[SUMMON_LINE_GOOD2] != 0) {
            BtlSeqRun();
            BtlSeqPlay(TALK_SCRIPT(g_btl_summon_line[SUMMON_LINE_GOOD2]));
        }
        BtlSeqRun();
        break;

    case SUMMON_OFFER:
        BtlMenuOpen2((const u_char *)
            (((g_btl_actors[g_btl_actor_slot].c.key - 1) * 0x10 | 8)
             + 0x800C6DAC));
        BtlIndicatorIcon();
        while ((choice = BtlMenuChoice()) < 0) {
            BtlMenuAsideToggle();
            BtlMenuUpdate();
            BtlTalkUpdate();
            BtlDrawFrame();
        }
        BtlIndicatorClear();
        BtlRunFrames(0xF);
        BtlMenuDismiss();
        while (BtlMenuState() != 0) {
            BtlMenuUpdate();
            BtlTalkUpdate();
            BtlDrawFrame();
        }
        BtlRunFrames(0xF);
        if (choice == 0) {
            if (BtlStockHasRoom() == 1) {
            BtlPanelClose();
            BtlTalkLeave(g_btl_summon_obj);
            while (g_btl_talk_leaving != 0) {
                BtlDrawFrame();
            }
            BtlLoadActorGfx(g_btl_actor_slot);
            BtlSetInsert(2, &g_btl_offer[g_btl_offer_slot].name);
            BtlSetInsert(6, g_btl_arcana_names[
                g_persona_data[g_btl_offer[g_btl_offer_slot].persona].arcana]);
            BtlSeqPlay(g_btl_talk_joined_script);
            BtlSeqRun();
            BtlSeqPlay(TALK_SCRIPT(g_btl_summon_line[SUMMON_LINE_OPEN]));
            BtlSeqWaitDone();
            BtlStockAdd(g_btl_offer[g_btl_offer_slot].persona);
            g_btl_talk_depth--;
            g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
            g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
            BtlShowAilmentMarks(1);
            BtlFaceClose();
            BtlPanelClose();
            BtlBoxClose();
            BtlSeqClear();
            BtlHudHide();
            BtlEnemiesReset();
            BtlPartyReset();
            g_btl_phase = 2;
            return;
            } else {
                g_btl_talk_stage[g_btl_talk_depth - 1] = SUMMON_FULL;
                return;
            }
        } else {
                g_btl_talk_stage[g_btl_talk_depth - 1] = SUMMON_LEAVE;
                BtlSeqPlay(TALK_SCRIPT(g_btl_summon_line[SUMMON_LINE_NO]));
                if (g_btl_summon_line[SUMMON_LINE_NO2] != 0) {
                    BtlSeqRun();
                    BtlSeqPlay(TALK_SCRIPT(g_btl_summon_line[SUMMON_LINE_NO2]));
                }
                BtlSeqWaitDone();
        }
        break;

    case SUMMON_FULL:
        more = 1;
        BtlSeqPlay(g_btl_talk_full_lines[g_btl_offer[g_btl_offer_slot].voice]);
        BtlSeqWaitDone();
        BtlSeqClear();
        BtlHudHide();
        BtlTalkStockMenu();
        BtlTextOpen(g_btl_talk_pick_gift_script, 0x28, 0x88);
        BtlBoxOpen(0x11, 0xA0, 0x88, 0);
        do {
            choice = g_btl_stock_choice;
            if (choice != -0x100 && choice != -1) {
                pick = g_persona_stock[choice];
                BtlTalkEndEffect();
                BtlBoxClose();
                BtlHudShow();
                while (BtlHudState() != 0) {
                    BtlDrawFrame();
                }
                BtlSetInsert(5, &g_persona_data[pick].name);
                BtlSeqPlay(g_btl_talk_gift_shown_script);
                BtlSeqRun();
                BtlMenuOpen2(g_btl_talk_give_menu);
                BtlIndicatorIcon();
                while ((choice = BtlMenuChoice()) < 0) {
                    BtlMenuAsideToggle();
                    BtlMenuUpdate();
                    BtlTalkUpdate();
                    BtlDrawFrame();
                }
                BtlIndicatorClear();
                BtlMenuDismiss();
                while (BtlMenuState() != 0) {
                    BtlMenuUpdate();
                    BtlTalkUpdate();
                    BtlDrawFrame();
                }
                BtlRunFrames(0xF);
                if (choice != 0) {
                    g_btl_talk_stage[g_btl_talk_depth - 1] = SUMMON_LEAVE;
                    BtlSeqPlay(TALK_SCRIPT(g_btl_summon_line[SUMMON_LINE_NO]));
                    if (g_btl_summon_line[SUMMON_LINE_NO2] != 0) {
                        BtlSeqRun();
                        BtlSeqPlay(
                            TALK_SCRIPT(g_btl_summon_line[SUMMON_LINE_NO2]));
                    }
                    more = 0;
                    BtlSeqWaitDone();
                } else {
                    BtlStockRemove(pick);
                    more = 0;
                    BtlSeqPlay(g_btl_talk_gift_taken_script);
                    g_btl_talk_stage[g_btl_talk_depth - 1] = SUMMON_TAKE;
                }
            }
            BtlDrawFrame();
        } while (more);
        break;

    case SUMMON_TAKE:
        g_btl_talk_stage[g_btl_talk_depth - 1] = SUMMON_LEAVE;
        BtlTalkLeave(g_btl_summon_obj);
        while (g_btl_talk_leaving != 0) {
            BtlDrawFrame();
        }
        BtlBeginAction();
        BtlSetInsert(2, &g_btl_offer[g_btl_offer_slot].name);
        BtlSetInsert(6, g_btl_arcana_names[
            g_persona_data[g_btl_offer[g_btl_offer_slot].persona].arcana]);
        BtlSeqRun();
        BtlSeqPlay(g_btl_talk_joined_script);
        BtlSeqRun();
        BtlSeqPlay(TALK_SCRIPT(g_btl_summon_line[SUMMON_LINE_OPEN]));
        BtlSeqWaitDone();
        BtlStockAdd(g_btl_offer[g_btl_offer_slot].persona);
        g_btl_talk_depth--;
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
        g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
        BtlShowAilmentMarks(1);
        BtlFaceClose();
        BtlPanelClose();
        BtlBoxClose();
        BtlSeqClear();
        BtlHudHide();
        BtlEnemiesReset();
        BtlPartyReset();
        g_btl_phase = 2;
        break;

    case SUMMON_REFUSE:
        g_btl_talk_stage[g_btl_talk_depth - 1] = SUMMON_LEAVE;
        BtlSeqPlay(TALK_SCRIPT(g_btl_summon_line[SUMMON_LINE_BAD]));
        if (g_btl_summon_line[SUMMON_LINE_BAD2] != 0) {
            BtlSeqRun();
            BtlSeqPlay(TALK_SCRIPT(g_btl_summon_line[SUMMON_LINE_BAD2]));
        }
        BtlSeqWaitDone();
        break;

    case SUMMON_LEAVE:
        BtlTalkersLeave();
        BtlTextOpen(g_btl_talk_left_script, 0x28, 0x92);
        BtlBoxOpen(0x11, 0xA0, 0x92, 0);
        BtlWaitAnyKey();
        g_btl_talk_depth--;
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
        g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
        if (BtlAnyEnemy() == 1) {
            BtlTalkLeave(g_btl_summon_obj);
        }
        BtlShowAilmentMarks(1);
        BtlFaceClose();
        BtlPanelClose();
        BtlBoxClose();
        BtlSeqClear();
        BtlHudHide();
        BtlEnemiesReset();
        BtlPartyReset();
        g_btl_phase = 3;
        break;
    }
    return;

pop:
    g_btl_talk_depth--;
    g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
    g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
}
#else
INCLUDE_ASM("btlp/nonmatchings/talkscenesummon", BtlTalkSceneSummon);
#endif

