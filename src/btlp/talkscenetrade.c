/* Persona 1 (JP) - the demon trading its Persona for a card.  BTLP only.
 *   0x800701DC BtlTalkSceneTrade
 *
 * BtlTalkSceneStep runs this while the top of g_btl_talk_scene is 1. The stage
 * opens with the three replies the acting member has for this demon, keyed by
 * its Char.key, and what the player picks decides whether the demon is pressed,
 * refused, or heard out.
 *
 * Pressing it is a level test with a one-in-sixteen roll at the weaker of the
 * two levels, and then a question of room: with none the demon says so and
 * leaves, with room the deal is either closed outright or a card is asked for.
 *
 * Asking for a card runs the stock menu - pick one, see the demon look it over,
 * and answer yes or no - with a second question offering another go. Agreeing
 * takes the card out of the stock and hands the Persona over.
 */
#include <decomp/types.h>
#include <rand.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/offer.h>
#include <persona/common/persona.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/text.h>

/* The stages, in the order the scene walks them. */
#define TRADE_ASK    1  /* open the three replies                    */
#define TRADE_REPLY  2  /* read which one was picked                 */
#define TRADE_REFUSE 3  /* turn the demon down                       */
#define TRADE_HEAR   4  /* let it make its case                      */
#define TRADE_CHECK  5  /* does it want a card, or is it just going? */
#define TRADE_PRESS  6  /* press it, and see whether it will deal    */
#define TRADE_TEASE  7  /* it will not - one more line and back to 5 */
#define TRADE_DEAL   8  /* it will - is there room for the Persona?  */
#define TRADE_STOCK  9  /* open the stock to pick a card             */
#define TRADE_PICK  10  /* read the pick and offer the card          */
#define TRADE_AGAIN 13  /* ask whether to show it another            */
#define TRADE_LEAVE 100 /* the demon goes                            */
#define TRADE_CLOSE 110 /* the deal closes and the Persona changes hands */

/* Scenes this one pushes on top of itself, and the stage they start at. */
#define TALK_SCENE_LINE 9
#define TALK_SCENE_SAY  0xC
#define TALK_STAGE_RUN  1

/* g_btl_talk_scene / g_btl_talk_stage, for a level nothing is running on. */
#define TALK_SCENE_NONE 0xFF
#define TALK_STAGE_FREE 0

/* BtlMenuChoice's two non-answers. */
#define MENU_WAIT   (-0x100)
#define MENU_CANCEL (-1)

/* Which reply leads where. */
#define REPLY_PRESS  0
#define REPLY_REFUSE 1
#define REPLY_HEAR   2

/* The lines it says. All of them are kind 0. */
#define TRADE_MSG_KIND   0
#define TRADE_MSG_REFUSE 4
#define TRADE_MSG_HEAR   5
#define TRADE_MSG_TEASE  7
#define TRADE_MSG_GOING  8
#define TRADE_MSG_LEAVE  9
#define TRADE_MSG_DEAL   10
#define TRADE_MSG_AGAIN  11
#define TRADE_MSG_TAKE   12

/* Where the boxes go. */
#define TRADE_TEXT_X   0x28
#define TRADE_STOCK_Y  0x88
#define TRADE_LEAVE_Y  0x92
#define TRADE_BOX_W    0x11
#define TRADE_BOX_X    0xA0

/* Frames held either side of a menu closing. */
#define TRADE_SETTLE 0xF

/* The offer levels tested, and one roll in sixteen at the weaker one. */
#define TRADE_LEVEL_STRONG 0
#define TRADE_LEVEL_WEAK   1
#define TRADE_ROLL_MASK    0xF

/* Bit 3 of the offer's kinds: this demon wants a card for its Persona. */
#define OFFER_WANTS_CARD 8

/* What BtlStockHasRoom answers when there is room. */
#define STOCK_HAS_ROOM 1

/* Which slot of the message each insert fills. */
#define INSERT_NAME   2
#define INSERT_GIFT   5
#define INSERT_ARCANA 6

/* Where the battle goes once the demon has left. */
#define BTL_PHASE_LEFT 3

/* The Persona cards the party is carrying, reached by address the way the rest
   of the save-game area is. */

/* Declared here rather than taken from persona/btlp/battle.h: hud.c
   defines it int, and the byte the call sites want is what makes the
   andi come out. The two forms are not interchangeable. */
extern char BtlHudState(void);
extern u_char        g_btl_talk_gift;
extern short         g_btl_stock_choice;
extern const u_char *g_btl_talk_reply_menus[][3];
extern const u_char *g_btl_talk_full_lines[];
extern const u_char *g_btl_arcana_names[];
extern const u_char *g_btl_talk_left_script;
extern const u_char *g_btl_talk_pick_gift_script;
extern const u_char *g_btl_talk_gift_shown_script;
extern const u_char *g_btl_talk_gift_taken_script;
extern const u_char  g_btl_talk_give_menu[];
extern const u_char  g_btl_talk_more_menu[];

extern int  BtlOfferLevelTest(int level, u_short slot);
extern void BtlStockRemove(int slot);
extern void BtlTalkStockMenu(void);
extern void BtlTalkEndEffect(void);
extern void BtlSetInsert(int slot, const u_char *text);
extern const u_char *BtlMessage(int line, int kind);
extern void BtlTakeOffer(u_short persona, const u_char *script);
extern void BtlMenuOpen2(const u_char *menu);
extern void BtlMenuOpen3(const u_char **menu);
extern int  BtlMenuChoice(void);
extern int  BtlMenuState(void);
extern void BtlMenuDismiss(void);
extern void BtlMenuHide(void);
extern void BtlMenuUpdate(void);
extern void BtlMenuAsideToggle(void);
extern void BtlIndicatorIcon(void);
extern void BtlTalkUpdate(void);
extern void BtlBoxOpen(short cols, short x, short y, int style);
extern void BtlHudShow(void);
extern void BtlShowAilmentMarks(int show);

void BtlTalkSceneTrade(void)
{
    int choice;
    int reply;
    u_long menus;
    int line;
    int state;

    /* The stage and byte offset share a scratch. Keep the switch byte-sized;
       widening it changes the dispatch registers. */
    state = g_btl_talk_stage[g_btl_talk_depth - 1];
    switch ((u_char)state) {
    case TRADE_ASK:
        BtlEndTalking();
        menus = (u_long)g_btl_talk_reply_menus;
        /* Keep the three-entry index in the same local as the later choices,
           and form its byte offset before adding the menu base. */
        choice = (g_btl_actors[g_btl_actor_slot].c.key - 1) * 3;
        state = choice * sizeof(u_char *);
        menus = state + menus;
        BtlMenuOpen3((const u_char **)menus);
        g_btl_talk_stage[g_btl_talk_depth - 1] = TRADE_REPLY;
        return;

    case TRADE_REPLY:
        reply = BtlMenuChoice();
        if (reply == MENU_WAIT) {
            return;
        }
        if (reply == MENU_CANCEL) {
            return;
        }
        switch (BtlMenuChoice()) {
        case REPLY_PRESS:
            g_btl_talk_stage[g_btl_talk_depth - 1] = TRADE_PRESS;
            break;
        case REPLY_REFUSE:
            g_btl_talk_stage[g_btl_talk_depth - 1] = TRADE_REFUSE;
            break;
        case REPLY_HEAR:
            g_btl_talk_stage[g_btl_talk_depth - 1] = TRADE_HEAR;
            break;
        }
        BtlRunFrames(TRADE_SETTLE);
        BtlMenuDismiss();
        while (BtlMenuState() != 0) {
            BtlMenuUpdate();
            BtlTalkUpdate();
            BtlDrawFrame();
        }
        BtlRunFrames(TRADE_SETTLE);
        return;

    case TRADE_REFUSE:
        g_btl_talk_stage[g_btl_talk_depth - 1] = TRADE_LEAVE;
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_LINE;
        g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_RUN;
        g_btl_talk_depth++;
        BtlSeqRun();
        line = TRADE_MSG_REFUSE;
        goto say_line;

    case TRADE_HEAR:
        g_btl_talk_stage[g_btl_talk_depth - 1] = TRADE_CHECK;
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_SAY;
        g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_RUN;
        g_btl_talk_depth++;
        BtlSeqPlay(BtlMessage(TRADE_MSG_HEAR, TRADE_MSG_KIND));
        return;

    case TRADE_CHECK:
        if ((g_btl_offer[g_btl_offer_slot].kinds & OFFER_WANTS_CARD) != 0) {
            g_btl_talk_stage[g_btl_talk_depth - 1] = TRADE_LEAVE;
            g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_LINE;
            g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_RUN;
            g_btl_talk_depth++;
            BtlSeqRun();
            BtlSeqPlay(BtlMessage(TRADE_MSG_GOING, TRADE_MSG_KIND));
            BtlSeqWaitDone();
            return;
        }
        goto leave;

    case TRADE_PRESS:
        if (BtlOfferLevelTest(TRADE_LEVEL_STRONG, g_btl_offer_slot) != 0
            || (BtlOfferLevelTest(TRADE_LEVEL_WEAK, g_btl_offer_slot) != 0
                && (rand() & TRADE_ROLL_MASK) == 0)) {
            g_btl_talk_stage[g_btl_talk_depth - 1] = TRADE_DEAL;
        } else {
            g_btl_talk_stage[g_btl_talk_depth - 1] = TRADE_TEASE;
        }
        return;

    case TRADE_TEASE:
        g_btl_talk_stage[g_btl_talk_depth - 1] = TRADE_CHECK;
        BtlSeqRun();
        line = TRADE_MSG_TEASE;
        goto say_line;

    case TRADE_DEAL:
        if (BtlStockHasRoom() == STOCK_HAS_ROOM) {
            if ((g_btl_offer[g_btl_offer_slot].kinds & OFFER_WANTS_CARD) != 0) {
                g_btl_talk_stage[g_btl_talk_depth - 1] = TRADE_CLOSE;
                g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_LINE;
                g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_RUN;
                g_btl_talk_depth++;
                BtlSeqRun();
                line = TRADE_MSG_DEAL;
            say_line:
                BtlSeqPlay(BtlMessage(line, TRADE_MSG_KIND));
                BtlSeqRun();
                return;
            }
            g_btl_talk_stage[g_btl_talk_depth - 1] = TRADE_CLOSE;
            BtlSeqRun();
            return;
        }
        g_btl_talk_stage[g_btl_talk_depth - 1] = TRADE_STOCK;
        BtlSeqRun();
        BtlSeqPlay(g_btl_talk_full_lines[g_btl_offer[g_btl_offer_slot].voice]);
        BtlSeqWaitDone();
        return;

    case TRADE_STOCK:
        g_btl_talk_stage[g_btl_talk_depth - 1] = TRADE_PICK;
        BtlSeqClear();
        BtlHudHide();
        BtlTalkStockMenu();
        BtlTextOpen(g_btl_talk_pick_gift_script, TRADE_TEXT_X, TRADE_STOCK_Y);
        BtlBoxOpen(TRADE_BOX_W, TRADE_BOX_X, TRADE_STOCK_Y, 0);
        return;

    case TRADE_PICK:
        if (g_btl_stock_choice == MENU_WAIT) {
            return;
        }
        if (g_btl_stock_choice == MENU_CANCEL) {
            g_btl_talk_stage[g_btl_talk_depth - 1] = TRADE_AGAIN;
            BtlTalkEndEffect();
            BtlBoxClose();
            BtlHudShow();
            while (BtlHudState() != 0) {
                BtlDrawFrame();
            }
            return;
        }
        g_btl_talk_gift = g_persona_stock[g_btl_stock_choice];
        BtlTalkEndEffect();
        BtlBoxClose();
        BtlHudShow();
        while (BtlHudState() != 0) {
            BtlDrawFrame();
        }
        BtlSetInsert(INSERT_GIFT, g_persona_data[g_btl_talk_gift].name);
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
        BtlRunFrames(TRADE_SETTLE);
        if (choice != 0) {
            g_btl_talk_stage[g_btl_talk_depth - 1] = TRADE_AGAIN;
            return;
        }
        BtlRunFrames(TRADE_SETTLE);
        g_btl_talk_stage[g_btl_talk_depth - 1] = TRADE_CLOSE;
        BtlSeqPlay(g_btl_talk_gift_taken_script);
        BtlSeqRun();
        BtlStockRemove(g_btl_talk_gift);
        return;

    case TRADE_AGAIN:
        BtlSeqPlay(BtlMessage(TRADE_MSG_AGAIN, TRADE_MSG_KIND));
        BtlSeqRun();
        BtlMenuOpen2(g_btl_talk_more_menu);
        BtlIndicatorIcon();
        while ((choice = BtlMenuChoice()) < 0) {
            BtlMenuAsideToggle();
            BtlMenuUpdate();
            BtlTalkUpdate();
            BtlDrawFrame();
        }
        BtlIndicatorClear();
        if (choice != 0) {
            BtlMenuDismiss();
            while (BtlMenuState() != 0) {
                BtlMenuUpdate();
                BtlTalkUpdate();
                BtlDrawFrame();
            }
            BtlRunFrames(TRADE_SETTLE);
        leave:
            g_btl_talk_stage[g_btl_talk_depth - 1] = TRADE_LEAVE;
            return;
        }
        BtlRunFrames(TRADE_SETTLE);
        BtlMenuHide();
        g_btl_talk_stage[g_btl_talk_depth - 1] = TRADE_STOCK;
        return;

    case TRADE_LEAVE:
        BtlSetInsert(INSERT_NAME, g_btl_offer[g_btl_offer_slot].name);
        BtlSetInsert(INSERT_ARCANA,
            g_btl_arcana_names[
                g_persona_data[g_btl_offer[g_btl_offer_slot].persona].arcana]);
        BtlSeqRun();
        BtlSeqPlay(BtlMessage(TRADE_MSG_LEAVE, TRADE_MSG_KIND));
        BtlSeqWaitDone();
        BtlTalkersLeave();
        BtlTextOpen(g_btl_talk_left_script, TRADE_TEXT_X, TRADE_LEAVE_Y);
        BtlBoxOpen(TRADE_BOX_W, TRADE_BOX_X, TRADE_LEAVE_Y, 0);
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
        g_btl_phase = BTL_PHASE_LEFT;
        return;

    case TRADE_CLOSE:
        g_btl_talk_depth--;
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
        g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
        BtlTakeOffer(g_btl_offer[g_btl_offer_slot].persona,
                     BtlMessage(TRADE_MSG_TAKE, TRADE_MSG_KIND));
        return;
    }
}

