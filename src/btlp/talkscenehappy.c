/* Persona 1 (JP) - the negotiation scene that leaves the demons happy.
 *   0x8007288C BtlTalkSceneHappy    BTLP only.
 *
 * BtlTalkSceneStep runs this while the top of g_btl_talk_scene is 4. It is one
 * of four scenes of the same shape - the others end in PANIC, CHARM and BIND -
 * and the stage below the top drives it a step at a time.
 *
 * The running stage seeds the generator from the beam position and tosses a
 * coin for which of two ways the exchange goes: one says another line first,
 * the other goes straight to the contact.
 *
 * The contact itself is decided by weighing the acting member's number against
 * the demon's. Below it the demons only grin and nothing else happens; at or
 * above it the contact ends, every demon the offer involved is left happy, and
 * the closing line is said. Either way the offer is marked so it cannot be
 * tried this way again.
 *
 * The last two stages take the negotiation down: one leaves the battle over,
 * the other puts it back to the command phase.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/text.h>

/* g_btl_talk_stage. The first three are shared with the other scenes; this one
   picks between the last two with a coin. */
#define TALK_STAGE_FREE  0
#define TALK_STAGE_RUN   1
#define TALK_STAGE_POP   2
#define TALK_STAGE_MORE  3
#define TALK_STAGE_TOUCH 4
#define TALK_STAGE_OVER  5
#define TALK_STAGE_BACK  6

/* g_btl_talk_scene, for a level nothing is running on. */
#define TALK_SCENE_NONE 0xFF

/* What the stages push: the scene that says one more line, and the one the
   pop stage goes back to. */
#define TALK_SCENE_MORE 9
#define TALK_SCENE_BACK 1

/* Which lines it says, out of BtlMessage's groups. */
#define TALK_MSG_KIND 6
#define TALK_MSG_MORE 4
#define TALK_MSG_GO   5

/* Where the closing box goes. */
#define TALK_TEXT_X 0x28
#define TALK_TEXT_Y 0x92
#define TALK_BOX_W  0x11
#define TALK_BOX_X  0xA0

/* The pack BtlTalkEndStatus plays as the demons take the status. */
#define TALK_PACK_HAPPY 0xE

/* How long the demons are left grinning before the box comes up. */
#define TALK_SMIRK_FRAMES 0x1E

/* Set on the offer once it has been contacted this way. */
#define OFFER_CONTACTED 0x10000000

/* Bit 0 of g_btl_talk_flags: a contact has ended. */
#define TALK_FLAG_ENDED 1

/* Where the battle goes afterwards. */
#define BTL_PHASE_COMMAND 0
#define BTL_PHASE_OVER    3

extern const u_char *g_btl_talk_left_script;
extern const u_char *g_btl_talk_smirk_script;
extern const u_char *g_btl_talk_happy_script;

extern int   VSync(int mode);
extern const u_char *BtlMessage(int group, int index);
extern void  BtlTalkEndStatus(u_char status, int pack);
extern void  BtlTalkEndStep(void);
extern void  BtlBoxOpen(short cols, short x, short y, int style);
extern void  BtlShowAilmentMarks(int show);

void BtlTalkSceneHappy(void)
{
    switch (g_btl_talk_stage[g_btl_talk_depth - 1]) {
    case TALK_STAGE_RUN:
        srand(VSync(-1));
        g_btl_talk_stage[g_btl_talk_depth - 1] = rand() % 2 + TALK_STAGE_MORE;
        break;

    case TALK_STAGE_POP:
        g_btl_talk_depth--;
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
        g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_BACK;
        g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_RUN;
        g_btl_talk_depth++;
        g_btl_talk_stage[g_btl_talk_depth - 1] = TALK_STAGE_BACK;
        break;

    case TALK_STAGE_MORE:
        g_btl_talk_stage[g_btl_talk_depth - 1] = TALK_STAGE_OVER;
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_MORE;
        g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_RUN;
        g_btl_talk_depth++;
        BtlSeqRun();
        BtlSeqPlay(BtlMessage(TALK_MSG_MORE, TALK_MSG_KIND));
        BtlSeqRun();
        break;

    case TALK_STAGE_TOUCH:
        g_btl_talk_stage[g_btl_talk_depth - 1] = TALK_STAGE_BACK;
        if (g_btl_actors[g_btl_actor_slot].unk3A
            >= g_btl_enemies[g_btl_talk_target].unk3A) {
            BtlEndTalking();
            BtlTalkEndStatus(BTL_STATUS_HAPPY, TALK_PACK_HAPPY);
            BtlTextOpen(g_btl_talk_happy_script, TALK_TEXT_X, TALK_TEXT_Y);
            BtlBoxOpen(TALK_BOX_W, TALK_BOX_X, TALK_TEXT_Y, 0);
            BtlWaitAnyKey();
            BtlTalkEndStep();
        } else {
            BtlRunFrames(TALK_SMIRK_FRAMES);
            BtlTextOpen(g_btl_talk_smirk_script, TALK_TEXT_X, TALK_TEXT_Y);
            BtlBoxOpen(TALK_BOX_W, TALK_BOX_X, TALK_TEXT_Y, 0);
            BtlWaitAnyKey();
        }
        g_btl_talk_flags |= TALK_FLAG_ENDED;
        g_btl_offer[g_btl_offer_slot].kinds |= OFFER_CONTACTED;
        break;

    case TALK_STAGE_OVER:
        g_btl_talk_depth--;
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
        g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
        BtlSeqRun();
        BtlSeqPlay(BtlMessage(TALK_MSG_GO, TALK_MSG_KIND));
        BtlSeqWaitDone();
        BtlTalkersLeave();
        BtlTextOpen(g_btl_talk_left_script, TALK_TEXT_X, TALK_TEXT_Y);
        BtlBoxOpen(TALK_BOX_W, TALK_BOX_X, TALK_TEXT_Y, 0);
        BtlWaitAnyKey();
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
