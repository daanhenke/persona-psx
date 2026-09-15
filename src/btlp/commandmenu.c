/* Persona 1 (JP) - the menus a round opens with.  BTLP only.
 *   0x8009CA38 BtlConfigMenu    0x8009CCE4 BtlCommandEntry
 *   0x8009DD74 BtlRunTalkScene  0x8009DDC8 BtlOrdersMenu
 *
 * BtlStageCommand opens the round by asking the player what the party does,
 * and these are what it opens. Two of them are whole menus:
 *
 * BtlCommandEntry is the one that walks the party. It puts the pick cursor on
 * a member, takes a command for them, and moves on to the next, and it is the
 * only one of the five big enough to hold the whole of that.
 *
 * BtlOrdersMenu is the shorter way round: rather than a command each, the
 * party takes one standing order between them, and the three it can choose
 * are the same three a finished negotiation picks from - it hands straight to
 * BtlTalkersLeaveField or BtlTalkersJoin.
 *
 * BtlConfigMenu is the settings page. It walks a table of pointers, one per
 * row, to the settings themselves - g_btl_confirm is the first of them - and
 * left and right step each one through its own list of values rather than
 * counting, so a row can hold whatever values it likes.
 *
 * BtlRunTalkScene is the one-liner the other two lean on: it starts the
 * negotiation, takes the answer the scene ends with, and then turns the frame
 * over until every fighter has finished moving, so whoever called it comes
 * back to a still field.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libsnd.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/board.h>
#include <persona/btlp/choice.h>
#include <persona/btlp/debug.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/input.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/stage.h>
#include <persona/btlp/status.h>
#include <persona/btlp/talk.h>
#include <persona/btlp/text.h>

INCLUDE_ASM("btlp/nonmatchings/commandmenu", BtlConfigMenu);

/* BtlCommandEntry's steps. It opens on the next member without a command and
   the picker over them; the rest are the ways out of it. */
#define ENTRY_NEXT    0 /* the next member without a command                */
#define ENTRY_PICK    1 /* the picker over that member                        */
#define ENTRY_DONE    2 /* nobody is left: keep the commands and leave        */
#define ENTRY_UNDO    3 /* whether the formation goes back to how it stood    */
#define ENTRY_UNDONE  4 /* it has: the moved members' markers are taken down  */
#define ENTRY_CONFIRM 5 /* whether the commands stand                         */
#define ENTRY_REVISE  6 /* they do not: back to the last member               */
#define ENTRY_CLEAR   7 /* the third key: every command is taken back         */
#define ENTRY_CLEARED 8 /* and the picker comes back once the markers are in  */

/* What a command answers besides the abort: it came to nothing, or the member
   has an order. */
#define COMMAND_NONE 0
#define COMMAND_MADE 1

/* Where the question about the formation goes; the confirmation takes the
   help line's place. */
#define ENTRY_UNDO_Y 0xBC

int BtlCommandEntry(void)
{
    int    i;
    int    prev;
    int    choice;
    int    row;
    int    col;
    int    cell;
    u_char kind;
    /* Sixteen bytes of the frame the image reserves and never touches; with
       nothing here the routine opens 0x68 bytes down against the image's
       0x78, and every register save is out by the difference. */
    u_char unused[16];

    for (i = 0; i < BTL_PARTY; i++) {
        if (g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0
            && g_btl_actors[i].marker == 0) {
            break;
        }
    }
    if (i >= BTL_PARTY) {
        BtlPickSettle();
        BtlShowAilmentMarks(0);
        BtlPartyResetGfx();
        g_btl_step = ENTRY_DONE;
    }
    if (SsIsEos(g_btl_seq[0], 0) == 0) {
        SsSepStop(g_btl_seq[0], 2);
        BtlSePlay(0, 0);
    }
    BtlPickShowPage(1);
    g_btl_actor_turn = -1;

    for (;;) {
        switch (g_btl_step) {
        case ENTRY_NEXT:
            g_btl_actor_turn = BtlUnreadyMemberNext(g_btl_actor_turn);
            if (g_btl_actor_turn >= 0) {
                g_btl_pick_help_row2 = g_btl_actors[g_btl_actor_turn].c.unk5D & BTL_CMD_ROW;
                BtlSingleOutMember(g_btl_actor_turn);
                g_btl_step++;
                break;
            }
            for (i = 0; i < BTL_PARTY; i++) {
                g_btl_marker_obj[i]->attr &= ~BTL_MARK_CHOSEN;
            }
            BtlPickSettle();
            BtlPartyResetGfx();
            if (g_btl_confirm == 0) {
                BtlOpenMessage(0, 0, g_btl_msg_commands_ok, PICK_HELP_X, PICK_HELP_Y);
                BtlOpenChoice1();
                g_btl_choice1_row = 0;
                g_btl_step = ENTRY_CONFIRM;
            } else {
                g_btl_step = ENTRY_DONE;
            }
            break;

        case ENTRY_PICK:
            choice = BtlPickUpdate(&g_btl_pick_help_row2);
        pick:
            switch (choice) {
            case BTL_PICK_WAIT:
                break;

            case BTL_PICK_CANCEL:
                if (g_btl_actor_turn < 0) {
                    g_btl_actor_turn = 0;
                }
                prev = g_btl_actor_turn;
                g_btl_actor_turn = BtlUnreadyMemberPrev(prev);
                if (g_btl_actor_turn >= 0) {
                    if (g_btl_marker_shown[g_btl_actor_turn] == NULL && BtlMarkersIdle() != 0) {
                        if (g_btl_actor_turn >= 0
                            && g_btl_actors[g_btl_actor_turn].revive_mark == BTL_REVIVE_CARRIED) {
                            g_btl_actors[g_btl_actor_turn].revive_mark = 0;
                            for (i = 0; i < g_btl_actor_turn; i++) {
                                if (g_btl_actors[i].revive_mark == BTL_REVIVE_CARRIED
                                    && g_btl_actors[i].revive_slot
                                           == g_btl_actors[g_btl_actor_turn].revive_slot) {
                                    break;
                                }
                            }
                            if (i >= g_btl_actor_turn) {
                                g_btl_actors[g_btl_actors[g_btl_actor_turn].revive_slot].revive_mark = 0;
                            }
                        }
                        BtlPartyResetGfx();
                        BtlSingleOutMember(g_btl_actor_turn);
                        g_btl_pick_help_row2 = g_btl_actors[g_btl_actor_turn].c.unk5D & BTL_CMD_ROW;
                        if (g_btl_actors[g_btl_actor_turn].marker == BTL_MARKER_ORDERED) {
                            BtlShowMarker(g_btl_actor_turn, 0, g_btl_pick_help_row2);
                            g_btl_actors[g_btl_actor_turn].marker = 0;
                        }
                        g_btl_actors[g_btl_actor_turn].flags &= ~BTL_ACTOR_FLINCHED;
                        break;
                    }
                    g_btl_actor_turn = BtlUnreadyMemberNext(g_btl_actor_turn);
                    break;
                }
                if (g_btl_pick_objs[0]->motion == 0) {
                    if (g_btl_formation_moved != 0) {
                        BtlSePlay(PICK_SE_BANK, PICK_SE_CLOSED);
                        BtlPartyResetGfx();
                        g_btl_marker_obj[prev]->attr &= ~BTL_MARK_CHOSEN;
                        BtlOpenMessage(0, 0, g_btl_msg_undo_formation, PICK_HELP_X, ENTRY_UNDO_Y);
                        g_btl_choice1_row = 0;
                        BtlRetractMarkers();
                        BtlPickSettle();
                        BtlOpenChoice1();
                        g_btl_step = ENTRY_UNDO;
                        break;
                    }
                    g_btl_actor_turn = -1;
                    if (g_btl_pick_objs[0]->motion == 0) {
                        g_btl_marker_obj[prev]->attr &= ~BTL_MARK_CHOSEN;
                        BtlSePlay(PICK_SE_BANK, PICK_SE_CLOSED);
                        BtlRestoreMarkers();
                        BtlPickShowPage(0);
                        BtlPartyResetGfx();
                        return 0;
                    }
                    break;
                }
                g_btl_actor_turn = BtlUnreadyMemberNext(g_btl_actor_turn);
                break;

            case BTL_PICK_ABORT:
                if (BtlMarkersHidden() != 0 && BtlMarkersIdle() != 0) {
                    g_btl_marker_obj[g_btl_actor_turn]->attr &= ~BTL_MARK_CHOSEN;
                    g_btl_step = ENTRY_CLEAR;
                }
                break;

            default:
                if (g_btl_marker_shown[g_btl_actor_turn] == NULL && BtlMarkersIdle() != 0) {
                    BtlSePlay(PICK_SE_BANK, PICK_SE_CHOSE);
                    g_btl_actors[g_btl_actor_turn].c.unk5D =
                        (g_btl_actors[g_btl_actor_turn].c.unk5D & BTL_CMD_KEPT) | g_btl_pick_help_row2;
                    g_btl_actors[g_btl_actor_turn].mark_kind = g_btl_pick_help_row2;
                    g_btl_step = ENTRY_NEXT;
                    switch (g_btl_command_fn[g_btl_pick_help_row2]()) {
                    case COMMAND_MADE:
                        g_btl_step = ENTRY_NEXT;
                        g_btl_actors[g_btl_actor_turn].marker = BTL_MARKER_ORDERED;
                        BtlShowMarker(g_btl_actor_turn, 1, g_btl_pick_help_row2);
                        break;
                    case COMMAND_NONE:
                        g_btl_step = ENTRY_PICK;
                        g_btl_actors[g_btl_actor_turn].marker = 0;
                        break;
                    default:
                        g_btl_marker_obj[g_btl_actor_turn]->attr &= ~BTL_MARK_CHOSEN;
                        g_btl_actors[g_btl_actor_turn].marker = 0;
                        g_btl_step = ENTRY_CLEAR;
                        break;
                    }
                }
                break;
            }
            break;

        case ENTRY_DONE:
            if ((g_btl_actor_turn < 0 || g_btl_marker_shown[g_btl_actor_turn] == NULL)
                && BtlMarkersIdle() != 0 && g_btl_pick_objs[0]->motion == 0) {
                for (i = 0; i < BTL_PARTY; i++) {
                    if ((g_btl_actors[i].flags & BTL_ACTOR_REFUSED) == 0) {
                        kind = g_btl_actors[i].c.unk5D & BTL_CMD_ROW;
                        g_btl_actors[i].c.unk5D = kind | (kind << 4);
                        g_btl_actors[i].move_kept = g_btl_actors[i].move;
                        g_btl_actors[i].ail_line_kept = g_btl_actors[i].ail_line;
                        g_btl_actors[i].order_kept = g_btl_actors[i].order;
                        g_btl_actors[i].targets_kept = g_btl_actors[i].targets;
                    }
                }
                BtlShowAilmentMarks(0);
                BtlPickShowPage(0);
                return 1;
            }
            break;

        case ENTRY_UNDO:
            choice = BtlChoiceUpdate(&g_btl_choice1_row);
            switch (choice) {
            case BTL_MENU_WAIT:
                break;
            case -1:
            case 1:
                BtlSePlay(PICK_SE_BANK, PICK_SE_CLOSED);
                BtlCloseMessage(0);
                BtlCloseChoice1();
                BtlRefreshMarkers();
                BtlPickRefresh();
                g_btl_actor_turn = BtlUnreadyMemberNext(g_btl_actor_turn);
                BtlSingleOutMember(g_btl_actor_turn);
                g_btl_step = ENTRY_PICK;
                break;
            case 0:
                BtlSePlay(PICK_SE_BANK, PICK_SE_CHOSE);
                for (row = 0, i = 0; row < GRID_H; row++) {
                    for (col = 0; col < GRID_W; col++) {
                        g_btl_formation[i] = g_btl_formation_before[i];
                        if (g_btl_formation[i] != CELL_EMPTY) {
                            BtlPlaceMember(g_btl_formation[i], col, row);
                            g_btl_actors[g_btl_formation[i]].flags &= ~BTL_ACTOR_REFUSED;
                        }
                        i++;
                    }
                }
                g_btl_formation_moved = 0;
                BtlBuildMarkers();
                BtlCloseMessage(0);
                BtlCloseChoice1();
                BtlRefreshMarkers();
                BtlPickRefresh();
                BtlPickHighlight(g_btl_pick_help_row);
                BtlPartyResetGfx();
                g_btl_step++;
                break;
            }
            break;

        case ENTRY_UNDONE:
            if (BtlMarkersIdle() != 0) {
                BtlPickShowPage(0);
                for (i = 0; i < BTL_PARTY; i++) {
                    if (g_btl_actors[i].c.key != 0
                        && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
                        && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0
                        && g_btl_actors[i].marker == BTL_MARKER_MOVED) {
                        g_btl_actors[i].marker = 0;
                        g_btl_actors[i].flags &= ~BTL_ACTOR_FLINCHED;
                        g_btl_actors[i].flags &= BTL_ACTOR_REFUSED;
                        BtlShowMarker(i, 0, BTL_MARK_KIND_REFUSED);
                    }
                }
                return 0;
            }
            break;

        case ENTRY_CONFIRM:
            choice = BtlChoiceUpdate(&g_btl_choice1_row);
            if (choice != BTL_MENU_WAIT) {
                if (choice == 0) {
                    BtlSePlay(PICK_SE_BANK, PICK_SE_CHOSE);
                    BtlCloseChoice1();
                    BtlCloseMessage(0);
                    g_btl_step = ENTRY_DONE;
                } else {
                    BtlSePlay(PICK_SE_BANK, PICK_SE_CLOSED);
                    BtlCloseChoice1();
                    BtlCloseMessage(0);
                    BtlPickRefresh();
                    BtlPickHighlight(g_btl_pick_help_row2);
                    g_btl_step++;
                    if (choice == BTL_PICK_ABORT) {
                        g_btl_step++;
                    }
                }
            }
            break;

        case ENTRY_REVISE:
            g_btl_actor_turn = BtlUnreadyMemberPrev(BTL_PARTY);
            if (g_btl_marker_shown[g_btl_actor_turn] == NULL && BtlMarkersIdle() != 0) {
                BtlSingleOutMember(g_btl_actor_turn);
                g_btl_pick_help_row2 = g_btl_actors[g_btl_actor_turn].c.unk5D & BTL_CMD_ROW;
                if (g_btl_actors[g_btl_actor_turn].marker == BTL_MARKER_ORDERED) {
                    BtlShowMarker(g_btl_actor_turn, 0, g_btl_pick_help_row2);
                    g_btl_actors[g_btl_actor_turn].marker = 0;
                }
                g_btl_actors[g_btl_actor_turn].flags &= ~BTL_ACTOR_FLINCHED;
                if (g_btl_actors[g_btl_actor_turn].revive_mark == BTL_REVIVE_CARRIED) {
                    g_btl_actors[g_btl_actor_turn].revive_mark = 0;
                    for (i = 0; i < g_btl_actor_turn; i++) {
                        if (g_btl_actors[i].revive_mark == BTL_REVIVE_CARRIED
                            && g_btl_actors[i].revive_slot
                                   == g_btl_actors[g_btl_actor_turn].revive_slot) {
                            break;
                        }
                    }
                    if (i >= g_btl_actor_turn) {
                        g_btl_actors[g_btl_actors[g_btl_actor_turn].revive_slot].revive_mark = 0;
                    }
                }
                g_btl_step = ENTRY_PICK;
            }
            break;

        case ENTRY_CLEAR:
            if (BtlMarkersHidden() != 0 && BtlMarkersIdle() != 0
                && g_btl_pick_objs[0]->motion == 0) {
                for (i = 0; i < BTL_PARTY; i++) {
                    if (g_btl_actors[i].marker == BTL_MARKER_ORDERED) {
                        BtlShowMarker(i, 0, g_btl_pick_help_row2);
                        g_btl_actors[i].marker = 0;
                    }
                    if (g_btl_actors[i].revive_mark == BTL_REVIVE_CARRIED) {
                        g_btl_actors[i].revive_mark = 0;
                        g_btl_actors[g_btl_actors[i].revive_slot].revive_mark = 0;
                    }
                }
                g_btl_actor_turn = -1;
                choice = BTL_PICK_CANCEL;
                if (g_btl_formation_moved != 0) {
                    g_btl_step++;
                    break;
                }
                g_btl_step = ENTRY_PICK;
                goto pick;
            }
            break;

        case ENTRY_CLEARED:
            if (BtlMarkersHidden() != 0 && BtlMarkersIdle() != 0) {
                g_btl_step = ENTRY_PICK;
                goto pick;
            }
            break;
        }
        BtlDrawFrame();
    }
}

/* The answer is taken before the wait, not after: the scene is finished with
   by then and the field is only being let catch up. */
int BtlRunTalkScene(void)
{
    int outcome;

    BtlTalkStart();
    outcome = BtlTalkSceneStep();
    BtlPackEnemyGrid();
    while (BtlActorsIdle() == 0) {
        BtlDrawFrame();
    }
    return outcome;
}

/* The steps of the orders menu: taking an order, carrying it out once the
   markers have settled, and the tactics board the order can lead to. */
#define ORDERS_PICK    0
#define ORDERS_APPLY   1
#define ORDERS_TACTICS 2
#define ORDERS_JOIN    3

/* The three standing orders, in the order the board lists them. */
#define ORDER_STAY  0
#define ORDER_LEAVE 1
#define ORDER_JOIN  2

int BtlOrdersMenu(void)
{
    int order;

    if ((g_btl_pad1 & g_btl_key_cancel) && g_btl_debug_hud != 0) {
        BtlDebugMenu();
        return 0;
    }
    BtlRetractMarkers();
    BtlOpenOrdersBoard();
    for (;;) {
        BtlDrawFrame();
        switch (g_btl_step) {
        case ORDERS_PICK:
            order = BtlOrdersMenuUpdate();
            if (order == BTL_PICK_WAIT) {
                break;
            }
            if (order == -1) {
                if (BtlMarkersIdle() != 0) {
                    BtlSePlay(1, 2);
                    BtlCloseOrdersBoard();
                    BtlRefreshMarkers();
                    return 0;
                }
                break;
            }
            if (BtlMarkersIdle() == 0) {
                break;
            }
            BtlSePlay(1, 1);
            BtlCloseOrdersBoard();
            BtlRefreshMarkers();
            if (order != ORDER_JOIN) {
                BtlPickSettle();
            }
            g_btl_step++;
            break;

        case ORDERS_APPLY:
            if (BtlMarkersIdle() == 0) {
                break;
            }
            switch (order) {
            case ORDER_STAY:
                BtlTalkersStay();
                break;
            case ORDER_LEAVE:
                BtlTalkersLeaveField();
                break;
            case ORDER_JOIN:
                BtlTalkersJoin();
                break;
            }
            BtlShowAilmentMarks(0);
            return 1;

        case ORDERS_TACTICS:
            order = BtlTacticsMenuUpdate();
            if (order == BTL_PICK_WAIT) {
                break;
            }
            BtlCloseTacticsBoard();
            BtlRefreshMarkers();
            g_btl_step++;
            break;

        case ORDERS_JOIN:
            if (BtlMarkersIdle() == 0) {
                break;
            }
            BtlTalkersJoin();
            return 1;
        }
    }
}
