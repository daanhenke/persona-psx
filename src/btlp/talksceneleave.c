/* Persona 1 (JP) - the negotiation scene that ends with the demons leaving.
 *   0x80071484 BtlTalkSceneLeave    BTLP only.
 *
 * The frame loop runs this while the top of g_btl_talk_scene is 2.
 *
 * On the running stage a coin decides how the group goes: one way it says a
 * single line and the stage is done, the other it pushes scene 9 on top with a
 * line of its own, so there is one more exchange before anybody moves.
 *
 * On the finished stage the level is popped - the scene back to none and the
 * stage to free - the group walks off, the box says who left, and everything
 * the negotiation put up comes down before the battle moves on.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/text.h>

/* The negotiation stack. Only the top entry is ever read, and the top is
   depth - 1. */
#define BTL_TALK_LEVELS 8

/* g_btl_talk_stage */
#define TALK_STAGE_FREE 0
#define TALK_STAGE_RUN  1
#define TALK_STAGE_DONE 2

/* g_btl_talk_scene, for a level nothing is running on. */
#define TALK_SCENE_NONE 0xFF

/* What this one pushes when the coin says there is more to say. */
#define TALK_SCENE_MORE 9

/* Which lines it says, out of BtlMessage's groups. */
#define TALK_MSG_KIND 2
#define TALK_MSG_GO   4
#define TALK_MSG_MORE 5

/* Where the parting line's box goes. */
#define TALK_LEFT_TEXT_X 0x28
#define TALK_LEFT_TEXT_Y 0x92
#define TALK_LEFT_BOX_W  0x11
#define TALK_LEFT_BOX_X  0xA0

/* The battle is over once the group has gone. */
#define BTL_PHASE_OVER 3

extern const u_char *g_btl_talk_left_script;

extern const u_char *BtlMessage(int group, int index);
extern void  BtlBoxOpen(short cols, short x, short y, int style);
extern void  BtlShowAilmentMarks(int show);

void BtlTalkSceneLeave(void)
{
    int depth;

    depth = g_btl_talk_depth;
    switch (g_btl_talk_stage[depth - 1]) {
    case TALK_STAGE_RUN:
        if ((rand() & 1) != 0) {
            g_btl_talk_stage[g_btl_talk_depth - 1] = TALK_STAGE_DONE;
            BtlSeqRun();
            BtlSeqPlay(BtlMessage(TALK_MSG_GO, TALK_MSG_KIND));
            BtlSeqWaitDone();
        } else {
            g_btl_talk_stage[g_btl_talk_depth - 1] = TALK_STAGE_DONE;
            g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_MORE;
            g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_RUN;
            g_btl_talk_depth++;
            BtlSeqRun();
            BtlSeqPlay(BtlMessage(TALK_MSG_MORE, TALK_MSG_KIND));
            BtlSeqRun();
        }
        break;

    case TALK_STAGE_DONE:
        g_btl_talk_depth = depth - 1;
        g_btl_talk_scene[g_btl_talk_depth] = TALK_SCENE_NONE;
        g_btl_talk_stage[g_btl_talk_depth] = TALK_STAGE_FREE;
        BtlTalkersLeave();
        BtlTextOpen(g_btl_talk_left_script, TALK_LEFT_TEXT_X, TALK_LEFT_TEXT_Y);
        BtlBoxOpen(TALK_LEFT_BOX_W, TALK_LEFT_BOX_X, TALK_LEFT_TEXT_Y, 0);
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
    }
}
