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

/* The settings page: four rows, three values a row, and the last row is the
   way through to the tactics board rather than a setting. */
#define CONFIG_ROWS    4
#define CONFIG_VALUES  3
#define CONFIG_TACTICS 3
#define BTL_CONFIG_VALUES CONFIG_VALUES

/* The glyph a value cell carries: the mark on the one chosen, a blank on the
   other two. */
#define CONFIG_BLANK 0x20
#define CONFIG_MARK  0x21

/* Which slot the menu's sounds come out of, and the four it plays. */
#define BTL_SE_SLOT   1
#define SE_MENU_MOVE  0
#define SE_MENU_STEP  3
#define SE_MENU_OPEN  1
#define SE_MENU_BACK  2

/* What BtlTacticsMenuUpdate answers with while it is still running, and when
   its own cancel takes the whole page down. */
#define BTL_MENU_RUNNING (-0x100)
#define BTL_MENU_DONE    (-2)

/* One cell of the settings board: where it sits, the glyphs it draws, and the
   byte at 9 that carries the mark. The four rows come first and the three
   columns of values follow them, four cells to a column, so a row's value
   cells are four, eight and twelve cells past its own. */
typedef struct {
    /* 0x0 */ short         x;
    /* 0x2 */ short         y;
    /* 0x4 */ const u_char *text;
    /* 0x8 */ u_char        pad8[1];
    /* 0x9 */ u_char        mark;
    /* 0xA */ u_char        padA[2];
} BtlConfigCell;                       /* 0xC bytes */

extern BtlConfigCell  g_btl_config_cells[];
extern u_char        *g_btl_config_rows[];
extern u_char   g_btl_config_nav[][2];
extern u_char   g_btl_config_step[][BTL_CONFIG_VALUES][2];
extern BtlMenuSpot g_btl_config_spots[];

/* The settings page. Four rows, each a pointer to the setting itself, and
   left and right step the row's own value through a table rather than
   counting - so a row can hold whatever values it likes and the last row,
   which is the way through to the tactics board, holds none.

   Confirm on that last row opens the tactics board and turns frames over
   until it answers: its own cancel comes back here, its confirm takes the
   whole page down, and anything else puts the settings board back. */
int BtlConfigMenu(void)
{
    BtlGfxCell    *cursor;
    BtlConfigCell *cell;
    u_char       **rows;
    int            keys;
    int            at;
    int            answer;
    int            i;

    at = 0;
    do {
        /* Read again every frame: lifted out of the whole page the table's
           address is one held register more, and the cell walk's own
           pointer comes out in front of the two values the loop optimiser
           lifts rather than behind them. */
        rows = g_btl_config_rows;
        keys = BtlMenuKey();
        if ((keys & (PAD_UP | PAD_DOWN)) != 0) {
            BtlSePlay(BTL_SE_SLOT, SE_MENU_MOVE);
        }
        if ((keys & (PAD_LEFT | PAD_RIGHT)) != 0 && at != CONFIG_TACTICS) {
            BtlSePlay(BTL_SE_SLOT, SE_MENU_STEP);
        }
        if ((keys & PAD_UP) != 0) {
            at = g_btl_config_nav[at][0];
        }
        if ((keys & PAD_DOWN) != 0) {
            at = g_btl_config_nav[at][1];
        }
        if ((keys & PAD_LEFT) != 0) {
            *rows[at] = g_btl_config_step[at][*rows[at]][0];
        }
        if ((keys & PAD_RIGHT) != 0) {
            *rows[at] = g_btl_config_step[at][*rows[at]][1];
        }
        for (i = 0, cursor = g_btl_menu_cursor; i < BTL_CURSOR_CELLS;
             i++, cursor++) {
            cursor->x = g_btl_config_spots[at].x - BTL_CURSOR_NUDGE;
            cursor->y = g_btl_config_spots[at].y;
        }
        for (i = 0, cell = g_btl_config_cells; i < CONFIG_ROWS; i++, cell++) {
            cell[CONFIG_ROWS].mark     = CONFIG_BLANK;
            cell[CONFIG_ROWS * 2].mark = CONFIG_BLANK;
            cell[CONFIG_ROWS * 3].mark = CONFIG_BLANK;
            cell[CONFIG_ROWS + CONFIG_ROWS * *rows[i]].mark = CONFIG_MARK;
        }
        if ((g_btl_pad1_edge & g_btl_key_confirm) != 0 && at == CONFIG_TACTICS) {
            BtlSePlay(BTL_SE_SLOT, SE_MENU_OPEN);
            BtlCloseConfigBoard();
            BtlOpenTacticsBoard();
            g_btl_tactics_row = 0;
            BtlDrawFrame();
            for (;;) {
                answer = BtlTacticsMenuUpdate();
                if (answer == BTL_MENU_DONE) {
                    BtlSePlay(BTL_SE_SLOT, SE_MENU_BACK);
                    BtlCloseTacticsBoard();
                    return BTL_MENU_DONE;
                }
                if (answer != BTL_MENU_RUNNING) {
                    break;
                }
                BtlDrawFrame();
            }
            BtlSePlay(BTL_SE_SLOT, SE_MENU_BACK);
            BtlCloseTacticsBoard();
            BtlOpenConfigBoard();
        }
        BtlDrawFrame();
    } while ((g_btl_pad1_edge & (g_btl_key_cancel | g_btl_key_abort)) == 0);
    return 0;
}

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
