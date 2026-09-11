/* Persona 1 (JP) - the stage the round opens with.  BTLP only.
 *   0x8009A60C BtlStageCommand
 *
 * The picker: six commands along the bottom, and whatever the player chooses
 * runs from a table of six handlers. The stage is one loop from end to end -
 * a switch on g_btl_step with the frame past the last of it - which is what
 * puts its two constants in saved registers before the first frame is drawn.
 *
 *   0  the picker is up. Watches the pad for the four shortcuts that open a
 *      board instead of taking a command, then asks BtlPickUpdate whether one
 *      of the six has been chosen and runs it.
 *   1  the markers are settling after a command was refused; every member
 *      still carrying one is shaken and given marker 5.
 *   2  the per-member walk is running. It is re-entered a frame at a time
 *      until BtlCommandEntry says every member has an action.
 *   3  the member boards are up; any of three keys shuts them.
 *   4  the fifteenth board is up, likewise.
 *   5  the markers are settling after step 2 handed back, and the chosen
 *      command is run again once they are.
 *
 * A handler that answers zero means the player backed out, and the stage goes
 * back to the picker. One that answers non-zero has taken the round's
 * commands, and the stage hands over - except for the two that carry on: the
 * negotiation that ended in the party joining stays on step 2, and the fifth
 * command goes to step 1 to have its markers put up.
 *
 * g_btl_talk_outcome being BTL_TALK_JOIN on the way in is a negotiation that
 * has already chosen for everybody, so the stage starts at step 2 with the
 * talk command already picked.
 *
 * 97.88%, and what is left is the delay-slot filler: gcc puts the step's
 * clear after the handler table's load rather than before it, and hoists the
 * table index above the label step 5 jumps back to. The shape, the shared
 * labels and the walk are all the image's - this is a permuter job.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/board.h>
#include <persona/btlp/input.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/stage.h>
#include <persona/btlp/status.h>
#include <persona/btlp/text.h>

/* The six handlers, one per picker slot. A null entry is a command that does
   nothing. */
extern int (*g_btl_pick_command[])(void);

/* The picker's own choice codes: the negotiation is the third slot and the
   fifth is the one that wants its markers put up before it carries on. */
#define PICK_TALK   2
#define PICK_MARKED 4

/* The outcome that means the demons joined, which is the one the stage both
   starts on and stays on. */
#define BTL_TALK_JOIN 3

/* The marker a member is carrying while the picker waits for it. */
#define MARKER_WAITING 2

/* Put on a member whose command could not be made, with the shake script. */
#define MARKER_REFUSED  5
#define MOTION_REFUSED  4
#define BTL_ACTOR_SHAKE 0x08000000

/* The picker's noises: one for a key that does something, two for a key that
   closes something, three for the cursor. */
#define PICK_SE_SLOT    1
#define PICK_SE_CHOSE   1
#define PICK_SE_CLOSED  2

/* The last of the five member boards, which is the one that finishes moving
   last. */
#define MEMBER_BOARDS_LAST 4

/* Cleared as the stage opens; nothing here reads it again. */
extern u_char D_800F4814;

extern BtlObj *g_btl_member_boards[];

extern u_short g_btl_key_select;
extern u_short g_btl_key_square;
extern u_short g_btl_key_r1;
extern u_short g_btl_key_r2;

extern void BtlPickSettle(void);
extern void BtlOpenMemberBoards(void);
extern void BtlCloseMemberBoards(void);
extern void BtlOpenBoard15(void);
extern void BtlCloseBoard15(void);
extern void BtlOpenConfigBoard(void);
extern void BtlCloseConfigBoard(void);
extern int  BtlConfigMenu(void);
extern int  BtlCommandEntry(void);

/* The two halves of the R1 display, which is held up until a key comes. */
extern void func_800A95BC(void);
extern void func_800A98A8(void);


#ifdef NON_MATCHING
void BtlStageCommand(void)
{
    BtlActor *a;
    int       two;
    int       choice;
    int       slot;

    /* The talk slot and the marker a member carries while the picker waits
       for it are the same number, and the image keeps one saved register for
       them across the whole loop - so they are one value here. Written as two
       constants, gcc materialises each of the three tests on its own and the
       routine comes out a saved register and four instructions short. */
    two = PICK_TALK;
    D_800F4814 = 0;
    BtlShowAilmentMarks(1);
    if (g_btl_talk_outcome == BTL_TALK_JOIN) {
        choice = 0;
        g_btl_step = PICK_TALK;
    }

    for (;;) {
        switch (g_btl_step) {
        case 0:
            BtlPickHighlight(g_btl_pick_help_row);
            if (g_btl_pick_objs[0]->motion != 0) {
                break;
            }
            if (BtlMarkersHidden() && g_btl_member_boards[0] == NULL
                && (g_btl_pad1_edge & g_btl_key_select)) {
                BtlSePlay(PICK_SE_SLOT, PICK_SE_CHOSE);
                BtlCloseMessage(0);
                BtlPickSettle();
                BtlRetractMarkers();
                BtlOpenMemberBoards();
                while (g_btl_member_boards[MEMBER_BOARDS_LAST]->motion != 0) {
                    BtlDrawFrame();
                }
                g_btl_step = BTL_TALK_JOIN;
                break;
            }
            if (g_btl_pad1_edge & g_btl_key_r1) {
                BtlSePlay(PICK_SE_SLOT, PICK_SE_CHOSE);
                BtlCloseMessage(0);
                func_800A95BC();
                BtlDrawFrame();
                while ((g_btl_pad1_edge
                        & (g_btl_key_r1 | g_btl_key_cancel | g_btl_key_abort))
                       == 0) {
                    BtlDrawFrame();
                }
                func_800A98A8();
                break;
            }
            if (BtlMarkersHidden() && (g_btl_pad1_edge & g_btl_key_r2)) {
                BtlSePlay(PICK_SE_SLOT, PICK_SE_CHOSE);
                BtlCloseMessage(0);
                BtlPickSettle();
                BtlRetractMarkers();
                BtlOpenBoard15();
                g_btl_step = PICK_MARKED;
                break;
            }
            if (BtlMarkersHidden() && (g_btl_pad1_edge & g_btl_key_square)) {
                BtlSePlay(PICK_SE_SLOT, PICK_SE_CHOSE);
                BtlCloseMessage(0);
                BtlPickSettle();
                BtlRetractMarkers();
                BtlOpenConfigBoard();
                if (BtlConfigMenu() != -2) {
                    BtlSePlay(PICK_SE_SLOT, PICK_SE_CLOSED);
                    BtlCloseConfigBoard();
                }
                BtlPickRefresh();
                BtlRefreshMarkers();
                break;
            }
            if (g_btl_leave_round != 0) {
                g_btl_stage = BTL_STAGE_CLOSE;
                return;
            }
            choice = BtlPickUpdate(&g_btl_pick_help_row);
            if (g_btl_talk_outcome == BTL_TALK_JOIN) {
                choice = PICK_TALK;
            }
            if (choice < 0) {
                break;
            }
            BtlSePlay(PICK_SE_SLOT, PICK_SE_CHOSE);
            /* Step 5 comes back in here once its markers have settled, and
               steps 2, 3 and 4 leave through the two labels below it. The
               image jumps to all four; written out in each arm instead, none
               of them is merged back together. */
        run:
            g_btl_step = 0;
            if (g_btl_pick_command[choice] != NULL
                && g_btl_pick_command[choice]() != 0) {
                if (choice == two
                    && g_btl_talk_outcome == BTL_TALK_JOIN) {
                    g_btl_step = choice;
                    break;
                }
                if (choice == PICK_MARKED) {
                    g_btl_step = 1;
                    BtlPickHighlight(g_btl_pick_help_row);
                    break;
                }
                goto handover;
            }
            g_btl_talk_outcome = 0;
        back_to_picker:
            g_btl_step = 0;
            break;

        case 1:
            if (!BtlMarkersIdle()) {
                break;
            }
            /* The record is read by slot and only the flag word is carried
               in a pointer of its own, which is the one saved register the
               image keeps for the walk. */
            a = g_btl_actors;
            for (slot = 0; slot < BTL_PARTY; slot++) {
                if (g_btl_actors[slot].c.key != 0
                    && (signed char)g_btl_actors[slot].c.status
                           != BTL_STATUS_DOWN
                    && (a->flags & BTL_ACTOR_OUT) == 0
                    && g_btl_actors[slot].marker == two) {
                    a->flags |= BTL_ACTOR_SHAKE;
                    g_btl_actors[slot].obj->motion = MOTION_REFUSED;
                    BtlShowMarker(slot, 1, MARKER_REFUSED);
                }
                a++;
            }
            g_btl_step++;
            break;

        case 2:
            if (g_btl_pick_objs[0]->motion != 0) {
                break;
            }
            if (!BtlMarkersHidden()) {
                break;
            }
            g_btl_step = 0;
            if (BtlCommandEntry() != 0) {
                g_btl_step = 0;
            handover:
                BtlCloseMessage(0);
                g_btl_stage++;
                return;
            }
            if (choice != two || g_btl_talk_outcome != BTL_TALK_JOIN) {
                goto back_to_picker;
            }
            BtlPickHighlight(g_btl_pick_help_row);
            g_btl_step = 5;
            break;

        case 3:
            if (g_btl_member_boards[MEMBER_BOARDS_LAST]->motion != 0) {
                break;
            }
            if ((g_btl_pad1_edge
                 & (g_btl_key_select | g_btl_key_cancel | g_btl_key_abort))
                == 0) {
                break;
            }
            BtlSePlay(PICK_SE_SLOT, PICK_SE_CLOSED);
            BtlCloseMemberBoards();
            goto closed;

        case 4:
            if ((g_btl_pad1_edge
                 & (g_btl_key_r2 | g_btl_key_cancel | g_btl_key_abort))
                == 0) {
                break;
            }
            BtlSePlay(PICK_SE_SLOT, PICK_SE_CLOSED);
            BtlCloseBoard15();
        closed:
            BtlPickRefresh();
            BtlRefreshMarkers();
            goto back_to_picker;

        case 5:
            if (!BtlMarkersHidden()) {
                break;
            }
            if (BtlMarkersIdle()) {
                goto run;
            }
            break;
        }
        BtlDrawFrame();
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/commandstage", BtlStageCommand);
#endif
