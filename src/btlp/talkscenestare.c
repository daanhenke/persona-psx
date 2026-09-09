/* Persona 1 (JP) - the demons staring the acting member down.  BTLP only.
 *   0x80071660 BtlTalkSceneStare
 *
 * BtlTalkSceneStep runs this while the top of g_btl_talk_scene is 3. The
 * running stage rolls for what the demons do: half the rolls reach the stare,
 * the rest close the offer and leave, and where the party is high enough for
 * the Persona on offer a single roll out of 256 hands it over instead.
 *
 * The stare is a second roll, and how the offer is doing changes the odds:
 * when its best gauge is the one asked about the member holds out two times in
 * three, otherwise only one in five. Losing puts the member on status 6, kind
 * 1 and 2 at +0xEA, swaps its object and shadow scripts, and leaves the battle
 * caught out; holding it says so and the battle carries on.
 *
 * Either way the offer is marked so it cannot be tried this way again.
 */
#include <decomp/types.h>
#include <rand.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/text.h>

/* Three arguments where the definition takes two: this unit passes the
   pause BtlTextSetState already sets for itself, and the register that
   loads is in the ROM. Declared here rather than in text.h because it is
   true of these call sites and no others. */
extern void BtlTextSetState(short state, int timer, int pause);

/* g_btl_talk_stage: the running stage, then one per outcome. */
#define TALK_STAGE_FREE  0
#define TALK_STAGE_RUN   1
#define TALK_STAGE_STARE 2
#define TALK_STAGE_CLOSE 3
#define TALK_STAGE_TAKE  4

/* g_btl_talk_scene, for a level nothing is running on. */
#define TALK_SCENE_NONE 0xFF

/* Which lines it says. The second argument is the scene's kind. */
#define TALK_MSG_KIND  3
#define TALK_MSG_TAKE1 4
#define TALK_MSG_TAKE2 5

/* Where the boxes go. */
#define TALK_TEXT_X 0x28
#define TALK_TEXT_Y 0x92
#define TALK_BOX_W  0x11
#define TALK_BOX_X  0xA0

/* The text state the line is put in while it is read. */
#define TALK_TEXT_STATE 5

/* Which offer level the Persona is gated on, and what BtlStockHasRoom answers
   when there is room. */
#define TALK_OFFER_LEVEL 0
#define STOCK_HAS_ROOM   1

/* The outcomes the roll can reach, and how the level test opens the last. */
#define TALK_ODDS        3
#define TALK_ODDS_ALL    0x100
#define TALK_ODDS_ALMOST 0xFF

/* How the stare goes: one in three when the offer is doing well, and one in
   five the rest of the time - the sense of the test is opposite either way. */
#define STARE_GOOD 3
#define STARE_BAD  5

/* What losing the stare leaves on the member. */
#define STARE_STATUS 6
#define STARE_LEVEL   1
#define STARE_UNK_EA 2

/* The shadow's own code, and which member's key and pick the motion is looked
   up by - the second one, not the acting one. */
#define STARE_SHADOW_CE 0x46
#define STARE_MOTION_OF 1

/* Set on the offer once it has been tried this way. */
#define OFFER_CONTACTED 0x10000000

/* Bit 0 of g_btl_talk_flags: a contact has ended. */
#define TALK_FLAG_ENDED 1

/* Which member's key and pick the motion table is keyed by. */
#define TALK_MOTION_KEY  0x28
#define TALK_MOTION_PICK 10

/* The sound slot the ambush closes. */
#define TALK_SLOT_VOICE 3

/* Where the battle goes afterwards. */
#define BTL_PHASE_COMMAND   0
#define BTL_PHASE_WON       2
#define BTL_PHASE_SURPRISED 4

extern u_char        g_btl_talk_motion[];
extern u_char       *g_btl_actor_gfx;
extern const u_char *g_btl_talk_menace_script;
extern const u_char *g_btl_talk_lost_script;
extern const u_char *g_btl_talk_glared_script;
extern const u_char *g_btl_talk_offer_over_script;
extern const u_char *g_btl_talk_surprised_script;

extern int  BtlOfferLevelTest(int level, u_short slot);
extern void BtlOfferFinish(void);
extern const u_char *BtlMessage(int line, int kind);
extern void BtlTakeOffer(u_short persona, const u_char *script);
extern void BtlBoxOpen(short cols, short x, short y, int style);
extern void BtlShowAilmentMarks(int show);
extern void BtlSoundClose(int slot);

#ifdef NON_MATCHING
void BtlTalkSceneStare(void)
{
    /* Kept in .rodata and copied onto the stack, so the level test can rewrite
       two of them. */
    int  odds[3] = { 0x80, TALK_ODDS_ALL, 0 };
    BtlActor *a;
    int  held;
    int  roll;
    int  i;
    int *p;
    const u_char *line;

    switch (g_btl_talk_stage[g_btl_talk_depth - 1]) {
    case TALK_STAGE_RUN:
        if (BtlOfferLevelTest(TALK_OFFER_LEVEL, g_btl_offer_slot) != 0
            && BtlStockHasRoom() == STOCK_HAS_ROOM) {
            odds[1] = TALK_ODDS_ALMOST;
            odds[2] = TALK_ODDS_ALL;
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
        g_btl_talk_stage[g_btl_talk_depth - 1] = i + TALK_STAGE_STARE;
        return;

    case TALK_STAGE_STARE:
        BtlTextOpen(g_btl_talk_menace_script, TALK_TEXT_X, TALK_TEXT_Y);
        held = 0;
        BtlBoxOpen(TALK_BOX_W, TALK_BOX_X, TALK_TEXT_Y, 0);
        BtlWaitAnyKey();
        BtlEndTalking();
        if (BtlOfferLevelTest(TALK_OFFER_LEVEL, g_btl_offer_slot) == 1) {
            held = rand() % STARE_GOOD == 0;
        } else {
            if (rand() % STARE_BAD != 0) {
                held = 1;
            }
        }
        g_btl_talk_flags |= TALK_FLAG_ENDED;
        g_btl_offer[g_btl_offer_slot].kinds |= OFFER_CONTACTED;
        /* The two text calls are shared and the glared arm tested first; the
           original branches the other way round, which gcc will not invert
           back - so this is the best of the shapes tried so far, not exact. */
        BtlTextSetState(TALK_TEXT_STATE, 0, 1);
        BtlTextWaitDone();
        if (held == 0) {
            BtlTextOpen(g_btl_talk_glared_script, TALK_TEXT_X, TALK_TEXT_Y);
            BtlBoxOpen(TALK_BOX_W, TALK_BOX_X, TALK_TEXT_Y, 0);
            BtlWaitAnyKey();
            goto command;
        }
        a = &g_btl_actors[g_btl_actor_slot];
        a->c.status = STARE_STATUS;
        a->c.ail_level = STARE_LEVEL;
        a->ail_turns = STARE_UNK_EA;
        BtlObjSetScript(a->obj,
            a->obj->scripts[g_btl_talk_motion[
                g_btl_actors[STARE_MOTION_OF].c.key * TALK_MOTION_KEY
                + g_btl_actors[STARE_MOTION_OF].script_pick * TALK_MOTION_PICK]]);
        BtlObjSetScript(a->obj->mark,
                        *(const u_long **)(g_btl_actor_gfx + 0x8C));
        a->obj->mark->unkCE = STARE_SHADOW_CE;
        a->obj->mark->attr &= ~BTL_OBJ_HIDDEN;
        line = g_btl_talk_lost_script;
        break;

    case TALK_STAGE_CLOSE:
        held = 0;
        BtlOfferFinish();
        BtlTextOpen(g_btl_talk_offer_over_script, TALK_TEXT_X, TALK_TEXT_Y);
        BtlBoxOpen(TALK_BOX_W, TALK_BOX_X, TALK_TEXT_Y, 0);
        BtlWaitAnyKey();
        if (BtlOfferLevelTest(TALK_OFFER_LEVEL, g_btl_offer_slot) == 1) {
            held = rand() % STARE_GOOD == 0;
        } else {
            if (rand() % STARE_BAD != 0) {
                held = 1;
            }
        }
        g_btl_talk_flags |= TALK_FLAG_ENDED;
        g_btl_offer[g_btl_offer_slot].kinds |= OFFER_CONTACTED;
        if (held == 0) {
        command:
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
            return;
        }
        BtlTextSetState(TALK_TEXT_STATE, 0, 1);
        BtlTextWaitDone();
        line = g_btl_talk_surprised_script;
        break;

    case TALK_STAGE_TAKE:
        BtlSeqRun();
        BtlSeqPlay(BtlMessage(TALK_MSG_TAKE1, TALK_MSG_KIND));
        BtlSeqRun();
        BtlTakeOffer(g_btl_offer[g_btl_offer_slot].persona,
                     BtlMessage(TALK_MSG_TAKE2, TALK_MSG_KIND));
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
        return;

    default:
        return;
    }

    BtlTextOpen(line, TALK_TEXT_X, TALK_TEXT_Y);
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
    BtlSoundClose(TALK_SLOT_VOICE);
    g_btl_phase = BTL_PHASE_SURPRISED;
}
#else
INCLUDE_ASM("btlp/nonmatchings/talkscenestare", BtlTalkSceneStare);
#endif

