/* Persona 1 (JP) - the command that changes a member's Persona, and the one
 * that refuses.  BTLP only.
 *   0x800A33BC BtlCommandChangePersona  0x800A3768 BtlCommandRefuse
 *
 * Entries 4 and 5 of g_btl_command_fn, which BtlCommandEntry runs for the row
 * of the command list the player chose. Both answer 1 once the member has an
 * order, 0 when the command came to nothing, and -2 on the third key.
 *
 * BtlCommandChangePersona runs the frames itself, on g_btl_step. First it
 * turns the member away with a line while it cannot change - its list is
 * blocked, it has no second Persona, or it is held - and waits for any key.
 * Otherwise the markers are drawn in, once the previous member's marker has
 * gone, and the stock board goes up. Then the swap is picked on the board: a
 * cancel or the third key takes the board down again, a choice records which
 * entry the member changes to, and the command waits for the member's marker
 * to come to rest.
 *
 * BtlCommandRefuse puts the member straight on motion 4 and answers 1.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/board.h>
#include <persona/btlp/input.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/stage.h>
#include <persona/btlp/text.h>

/* The member's list entry that holds its second Persona, and the actor flag
   that holds a Persona where it is. */
#define CHANGE_SECOND 1
#define CHANGE_HELD   BTL_ACTOR_TIMED_B

/* The answers the swap board gives besides a choice. */
#define CHANGE_CANCEL (-1)
#define CHANGE_ABORT  (-2)

/* The motion a refused member is put on. */
#define REFUSE_MOTION 4

extern short        g_btl_swap_row;

int BtlCommandChangePersona(void)
{
    int pick;
    int prev;

    BtlDrawFrame();
    while (1) {
        switch (g_btl_step) {
        case 0:
            if (g_btl_actors[g_btl_actor_turn].c.blocked != 0) {
                BtlOpenMessage(0, 0, g_btl_msg_persona_blocked, PICK_HELP_X,
                               PICK_HELP_Y);
                while (g_btl_pad1_edge == 0) {
                    BtlDrawFrame();
                }
                if (g_btl_no_help != 0) {
                    BtlCloseMessage(0);
                }
                return 0;
            }
            if (g_btl_actors[g_btl_actor_turn].c.list[CHANGE_SECOND] == 0xFF) {
                BtlOpenMessage(0, 0, g_btl_msg_persona_alone, PICK_HELP_X,
                               PICK_HELP_Y);
                while (g_btl_pad1_edge == 0) {
                    BtlDrawFrame();
                }
                if (g_btl_no_help != 0) {
                    BtlCloseMessage(0);
                }
                return 0;
            }
            if (g_btl_actors[g_btl_actor_turn].flags & CHANGE_HELD) {
                BtlOpenMessage(0, 0, g_btl_msg_persona_held, PICK_HELP_X,
                               PICK_HELP_Y);
                while (g_btl_pad1_edge == 0) {
                    BtlDrawFrame();
                }
                if (g_btl_no_help != 0) {
                    BtlCloseMessage(0);
                }
                return 0;
            }
            BtlCloseMessage(0);
            prev = BtlUnreadyMemberPrev(g_btl_actor_turn);
            if (prev >= 0 && g_btl_marker_shown[prev] != NULL) {
                break;
            }
            BtlRetractMarkers();
            BtlOpenStockBoard();
            g_btl_swap_row = 0;
            g_btl_step++;
            break;
        case 1:
            pick = BtlPersonaSwapUpdate(&g_btl_swap_row);
            /* A switch, laid out as the image has it: a compare tree over
               the three answers, and the cancel's arm ahead of the abort's. */
            switch (pick) {
            case CHANGE_CANCEL:
                BtlSePlay(1, 2);
                BtlCloseStockBoard();
                BtlRefreshMarkers();
                return 0;
            case CHANGE_ABORT:
                BtlSePlay(1, 2);
                BtlCloseStockBoard();
                BtlRefreshMarkers();
                return CHANGE_ABORT;
            case BTL_PICK_WAIT:
                break;
            default:
                BtlSePlay(1, 1);
                BtlCloseStockBoard();
                BtlRefreshMarkers();
                g_btl_actors[g_btl_actor_turn].form = pick;
                g_btl_step++;
                break;
            }
            break;
        case 2:
            if (BtlObjChainAtMotion(g_btl_marker_obj[g_btl_actor_turn], 0)
                != 0) {
                return 1;
            }
            break;
        }
        BtlDrawFrame();
    }
}

int BtlCommandRefuse(void)
{
    g_btl_actors[g_btl_actor_turn].obj->motion = REFUSE_MOTION;
    return 1;
}
