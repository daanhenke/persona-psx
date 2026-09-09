/* Persona 1 (JP) - the negotiation scene that can leave the demons panicked.
 *   0x80070CB0 BtlTalkScenePanic    BTLP only.
 *
 * BtlTalkSceneStep runs this while the top of g_btl_talk_scene is 7. It is the
 * richest of the four scenes of this shape - the others can end in HAPPY,
 * CHARM and BIND - and the stage below the top drives it a step at a time.
 *
 * The running stage rolls for what the demons do. Five thresholds this time:
 * they run off, they hand an item over, the contact is tried, the offer is
 * closed and they leave, or they give their Persona up. Where the party is
 * high enough for the Persona on offer and has room for it, the last two
 * thresholds are rewritten so the roll can reach that outcome.
 *
 * The contact weighs the acting member's number against the demon's: at or
 * above it the negotiation ends with every demon the offer involved panicked,
 * below it they are unshaken. Either way the offer is marked so it cannot be
 * tried this way again.
 *
 * The outcome that closes the offer has a second test after it. If the demons
 * walking away are quicker than the acting member the party is caught out and
 * the battle goes to phase 4 rather than back to the command phase.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/text.h>

/* Three arguments where the definition takes two: this unit passes the
   pause BtlTextSetState already sets for itself, and the register that
   loads is in the ROM. Declared here rather than in text.h because it is
   true of these call sites and no others. */
extern void BtlTextSetState(short state, int timer, int pause);

/* g_btl_talk_stage: the running stage, then one per outcome of its roll, then
   the two ways the scene can be left. */
#define TALK_STAGE_FREE   0
#define TALK_STAGE_RUN    1
#define TALK_STAGE_GO     2
#define TALK_STAGE_ITEM   3
#define TALK_STAGE_TOUCH  4
#define TALK_STAGE_CLOSE  5
#define TALK_STAGE_GIVE   6
#define TALK_STAGE_CAUGHT 7
#define TALK_STAGE_BACK   8

/* g_btl_talk_scene, for a level nothing is running on, and the one pushed when
   the item will not fit. */
#define TALK_SCENE_NONE 0xFF
#define TALK_SCENE_MORE 9

/* Which lines it says. The second argument is the scene's kind. */
#define TALK_MSG_KIND 1
#define TALK_MSG_ITEM 4
#define TALK_MSG_GIVE 5

/* Where the boxes go. */
#define TALK_TEXT_X 0x28
#define TALK_TEXT_Y 0x92
#define TALK_BOX_W  0x11
#define TALK_BOX_X  0xA0

/* The pack BtlTalkEndStatus plays as the demons take the status. */
#define TALK_PACK_PANIC 0xB

/* Which of the two offer levels the Persona is gated on here. */
#define TALK_OFFER_LEVEL 0

/* What BtlStockHasRoom answers when there is somewhere to put it. */
#define STOCK_HAS_ROOM 1

/* The outcomes the roll can reach, and how the level test opens the last. */
#define TALK_ODDS 5
#define TALK_ODDS_ALL    0x100
#define TALK_ODDS_ALMOST 0xFF

/* Which of the five stats decides who is quicker. */

/* The text state the parting line is put in while it is read, and the sound
   slots the two exits close. */
#define TALK_TEXT_STATE 5
#define TALK_SLOT_VOICE 3
#define TALK_SLOT_CUE   4

/* What BtlSeqState answers once the joining sequence has run far enough. */
#define TALK_SEQ_READY 8

/* The state the actor's graphics are reloaded under, and for how long. */
#define TALK_SEQ_JOIN  8
#define TALK_SEQ_FRAMES 4

/* Set on the offer once it has been contacted this way. */
#define OFFER_CONTACTED 0x10000000

/* Bit 0 of g_btl_talk_flags: a contact has ended. */
#define TALK_FLAG_ENDED 1

/* Where the battle goes afterwards. */
#define BTL_PHASE_COMMAND   0
#define BTL_PHASE_WON       2
#define BTL_PHASE_OVER      3
#define BTL_PHASE_SURPRISED 4

extern const u_char *g_btl_talk_fled_script;
extern const u_char *g_btl_talk_panic_script;
extern const u_char *g_btl_talk_unshaken_script;
extern const u_char *g_btl_talk_offer_over_script;
extern const u_char *g_btl_talk_surprised_script;
extern const u_char *g_btl_talk_joining_script;
extern const u_char *g_btl_talk_joined_script;

/* Its slot is a u_short, which is what reads g_btl_offer_slot unsigned here
   and signed where the offer itself is indexed. */
extern int   BtlOfferLevelTest(int level, u_short slot);
extern int   BtlTalkGiveItem(void);
extern void  BtlOfferFinish(void);
extern int   BtlSeqState(void);
extern void  BtlSeqSetState(int state, int frames);
extern const u_char *BtlMessage(int line, int kind);
extern void  BtlTalkEndStatus(u_char status, int pack);
extern void  BtlTalkEndStep(void);
extern void  BtlStockAdd(int persona);
extern void  BtlBoxOpen(short cols, short x, short y, int style);
extern void  BtlIndicatorBar(void);
extern void  BtlLoadActorGfx(int slot);
extern void  BtlSoundClose(int slot);
extern void  BtlShowAilmentMarks(int show);

void BtlTalkScenePanic(void)
{
    /* Kept in .rodata and copied onto the stack on every call, which is what
       lets the level test below rewrite two of them. */
    int  odds[TALK_ODDS] = { 0x2B, 0x56, 0xD6, TALK_ODDS_ALL, 0 };
    int  roll;
    int  i;
    int *p;

    switch (g_btl_talk_stage[g_btl_talk_depth - 1]) {
    case TALK_STAGE_RUN:
        if (BtlOfferLevelTest(TALK_OFFER_LEVEL, g_btl_offer_slot) != 0
            && BtlStockHasRoom() == STOCK_HAS_ROOM) {
            odds[3] = TALK_ODDS_ALMOST;
            odds[4] = TALK_ODDS_ALL;
        }
        roll = rand() % TALK_ODDS_ALL;
        i = 0;
        p = odds;
        while (i < TALK_ODDS) {
            if (roll < *p) {
                break;
            }
            i++;
            p++;
        }
        g_btl_talk_stage[g_btl_talk_depth - 1] = i + TALK_STAGE_GO;
        break;

    case TALK_STAGE_GO:
        BtlTalkersLeave();
        BtlTextOpen(g_btl_talk_fled_script, TALK_TEXT_X, TALK_TEXT_Y);
        BtlBoxOpen(TALK_BOX_W, TALK_BOX_X, TALK_TEXT_Y, 0);
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
        g_btl_phase = BTL_PHASE_OVER;
        break;

    case TALK_STAGE_ITEM:
        g_btl_talk_stage[g_btl_talk_depth - 1] = TALK_STAGE_GO;
        BtlSeqRun();
        BtlSeqPlay(BtlMessage(TALK_MSG_ITEM, TALK_MSG_KIND));
        BtlSeqRun();
        if (BtlTalkGiveItem() == 0) {
            g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_MORE;
            g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_RUN;
            g_btl_talk_depth++;
        }
        break;

    case TALK_STAGE_TOUCH:
        g_btl_talk_stage[g_btl_talk_depth - 1] = TALK_STAGE_BACK;
        if (g_btl_actors[g_btl_actor_slot].unk3A
            >= g_btl_enemies[g_btl_talk_target].unk3A) {
            BtlEndTalking();
            BtlTalkEndStatus(BTL_STATUS_PANIC, TALK_PACK_PANIC);
            BtlTextOpen(g_btl_talk_panic_script, TALK_TEXT_X, TALK_TEXT_Y);
            BtlBoxOpen(TALK_BOX_W, TALK_BOX_X, TALK_TEXT_Y, 0);
            BtlWaitAnyKey();
            BtlTalkEndStep();
        } else {
            BtlTextOpen(g_btl_talk_unshaken_script, TALK_TEXT_X, TALK_TEXT_Y);
            BtlBoxOpen(TALK_BOX_W, TALK_BOX_X, TALK_TEXT_Y, 0);
            BtlWaitAnyKey();
        }
        g_btl_talk_flags |= TALK_FLAG_ENDED;
        g_btl_offer[g_btl_offer_slot].kinds |= OFFER_CONTACTED;
        break;

    case TALK_STAGE_CLOSE:
        BtlOfferFinish();
        BtlTextOpen(g_btl_talk_offer_over_script, TALK_TEXT_X, TALK_TEXT_Y);
        BtlBoxOpen(TALK_BOX_W, TALK_BOX_X, TALK_TEXT_Y, 0);
        BtlWaitAnyKey();
        /* Spelt with the quicker member first: gcc lays the blocks out the
           other way round from the obvious reading. */
        if (g_btl_actors[g_btl_actor_slot].c.stat_base[STAT_AGILITY]
            >= g_btl_enemies[g_btl_talk_target].c.stat_base[STAT_AGILITY]) {
            g_btl_talk_stage[g_btl_talk_depth - 1] = TALK_STAGE_BACK;
        } else {
            BtlTextSetState(TALK_TEXT_STATE, 0, 1);
            BtlTextWaitDone();
            BtlTextOpen(g_btl_talk_surprised_script, TALK_TEXT_X, TALK_TEXT_Y);
            BtlBoxOpen(TALK_BOX_W, TALK_BOX_X, TALK_TEXT_Y, 0);
            BtlWaitAnyKey();
            g_btl_talk_stage[g_btl_talk_depth - 1] = TALK_STAGE_CAUGHT;
        }
        BtlSoundClose(TALK_SLOT_CUE);
        g_btl_talk_flags |= TALK_FLAG_ENDED;
        g_btl_offer[g_btl_offer_slot].kinds |= OFFER_CONTACTED;
        break;

    case TALK_STAGE_GIVE:
        g_btl_talk_stage[g_btl_talk_depth - 1] = TALK_STAGE_BACK;
        BtlTextOpen(g_btl_talk_joining_script, TALK_TEXT_X, TALK_TEXT_Y);
        BtlBoxOpen(TALK_BOX_W, TALK_BOX_X, TALK_TEXT_Y, 0);
        BtlWaitAnyKey();
        BtlSeqRun();
        BtlPanelClose();
        BtlSeqPlay(BtlMessage(TALK_MSG_GIVE, TALK_MSG_KIND));
        while (BtlSeqState() != TALK_SEQ_READY) {
            BtlDrawFrame();
        }
        BtlSeqSetState(0, 0);
        BtlIndicatorClear();
        BtlBoxClose();
        BtlLoadActorGfx(g_btl_actor_slot);
        BtlSeqSetState(TALK_SEQ_JOIN, TALK_SEQ_FRAMES);
        BtlIndicatorBar();
        BtlSeqRun();
        BtlSeqPlay(g_btl_talk_joined_script);
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
        g_btl_phase = BTL_PHASE_WON;
        break;

    case TALK_STAGE_CAUGHT:
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
        BtlSoundClose(TALK_SLOT_VOICE);
        g_btl_phase = BTL_PHASE_SURPRISED;
        break;

    case TALK_STAGE_BACK:
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
        g_btl_phase = BTL_PHASE_COMMAND;
        break;
    }
}
