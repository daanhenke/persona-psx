/* Persona 1 (JP) - the row the party stands in.  BTLP only.
 *   0x8009E000 BtlPlaceMenu
 *
 * Entry 4 of g_btl_pick_command: the command that puts the party's own
 * formation up and lets a member be moved between the front and the back.
 * It runs its own frame loop rather than being ticked, the way every other
 * command in that table does.
 *
 * It opens on a two-way choice. Moving members puts the grid up, lets the
 * cursor pick members up and put them down, and on the way out asks whether
 * the new formation is kept as one of the stored layouts - and which, and
 * whether to write over one already there. Standing on a stored layout puts
 * the formation board up instead, refused while any member is down or has
 * already moved. Either way the menu answers 1 once a member really moved,
 * with g_btl_formation_moved raised, and 0 when nothing did.
 *
 * The third key unwinds the whole thing: g_btl_place_abort is raised, and
 * every step it passes through on the way back takes it as its own abort.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/choice.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/input.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/stage.h>
#include <persona/btlp/text.h>

/* BtlPlaceMenu's steps. */
#define PLACE_ASK       0 /* move members, or stand on a stored layout      */
#define PLACE_OPEN      1 /* the grid coming up                             */
#define PLACE_PICK      2 /* the cursor over the grid, choosing a member    */
#define PLACE_CARRY     3 /* carrying the member picked up                  */
#define PLACE_KEEP      4 /* whether the new formation is stored            */
#define PLACE_STORE     5 /* which layout it is stored as                   */
#define PLACE_OVERWRITE 6 /* whether that layout is written over            */
#define PLACE_RECALL    7 /* a stored layout chosen to stand on             */

/* The opening choice's two rows. */
#define PLACE_CHOICE_MOVE   0
#define PLACE_CHOICE_RECALL 1

/* What the grid's cursor answers besides a cancel: a member picked up or put
   down, the player done moving, and the cancel an abort from further in is
   passed down as. */
#define PLACE_GRID_DONE   0
#define PLACE_GRID_FINISH (-2)
#define PLACE_GRID_UNWIND (-3)

/* How long the grid is given to come up, and the question to settle. */
#define PLACE_OPEN_FRAMES 30
#define PLACE_KEEP_FRAMES 17

/* Where the questions about the layouts go. */
#define PLACE_ASK_Y 0xBC

/* The attribute a stored layout's preview leaves on the party. */
#define PLACE_PREVIEW_ATTR 1

#ifdef NON_MATCHING
int BtlPlaceMenu(void)
{
    int choice;
    int slot;
    int blocked;
    int row;
    int col;
    int i;

    g_btl_place_abort = 0;
    BtlDrawFrame();
    g_btl_place_member = BtlUnreadyMemberNext(-1);
    if (g_btl_place_member < 0) {
        BtlOpenMessage(0, 0, g_btl_msg_nobody_to_move, PICK_HELP_X, PICK_HELP_Y);
        while (g_btl_pad1_edge == 0) {
            BtlDrawFrame();
        }
        if (g_btl_no_help != 0) {
            BtlCloseMessage(0);
        }
        return 0;
    }
    *(BtlFormation *)g_btl_formation_before = *(BtlFormation *)g_btl_formation;
    BtlDimEnemies();
    BtlPickSettle();
    BtlOpenChoice0();

    for (;;) {
        switch (g_btl_step) {
        case PLACE_ASK:
            choice = BtlChoiceUpdate(&g_btl_choice0_row);
            if (g_btl_place_abort != 0) {
                choice = BTL_PICK_ABORT;
            }
            switch (choice) {
            case BTL_MENU_WAIT:
                break;

            case BTL_PICK_ABORT:
            case BTL_PICK_CANCEL:
                BtlSePlay(PICK_SE_BANK, PICK_SE_CLOSED);
                BtlCloseChoice0();
                BtlPickRefresh();
                BtlEnemiesResetGfx();
                return 0;

            case PLACE_CHOICE_MOVE:
                if (g_btl_grid_tail == NULL) {
                    BtlCloseMessage(0);
                    BtlSePlay(PLACE_SE_SLOT, PLACE_SE_OPEN);
                    BtlSpawnPickGrid();
                    g_btl_place_row = 0;
                    g_btl_place_col = 0;
                    g_btl_place_member = BtlUnreadyMemberNext(-1);
                    g_btl_delay = PLACE_OPEN_FRAMES;
                    g_btl_step++;
                }
                break;

            case PLACE_CHOICE_RECALL:
                for (i = 0, blocked = 0; i < BTL_PARTY; i++) {
                    if ((g_btl_actors[i].c.key != 0
                         && (signed char)g_btl_actors[i].c.status == BTL_STATUS_DOWN)
                        || g_btl_actors[i].marker >= BTL_MARKER_MOVED
                        || (g_btl_actors[i].c.key != 0
                            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) != 0)) {
                        blocked = 1;
                        break;
                    }
                }
                if (blocked) {
                    BtlSePlay(PICK_SE_BANK, PICK_SE_CHOSE);
                    BtlOpenMessage(0, 0, g_btl_msg_place_blocked, PICK_HELP_X, PICK_HELP_Y);
                    do {
                        BtlDrawFrame();
                    } while (g_btl_pad1_edge == 0);
                    BtlSePlay(PICK_SE_BANK, PICK_SE_CLOSED);
                    if (g_btl_no_help != 0) {
                        BtlCloseMessage(0);
                    }
                } else if (g_btl_grid_tail == NULL) {
                    BtlCloseMessage(0);
                    BtlSePlay(PLACE_SE_SLOT, PLACE_SE_OPEN);
                    BtlCloseChoice0();
                    BtlRetractMarkers();
                    BtlSpawnPickGrid();
                    BtlOpenFormationBoard();
                    g_btl_step = PLACE_RECALL;
                }
                break;
            }
            break;

        case PLACE_OPEN:
            if (g_btl_delay == 0 && g_btl_pick_cursors[0]->motion == 0) {
                g_btl_step++;
            }
            break;

        case PLACE_PICK:
            choice = BtlPlaceGridUpdate(0);
            if (g_btl_place_abort != 0) {
                choice = PLACE_GRID_UNWIND;
            }
            switch (choice) {
            case PLACE_GRID_UNWIND:
            case BTL_PICK_CANCEL:
                BtlSePlay(PLACE_SE_SLOT, PLACE_SE_SHUT);
                for (row = 0, i = 0; row < GRID_H; row++) {
                    for (col = 0; col < GRID_W; col++) {
                        g_btl_formation[i] = g_btl_formation_before[i];
                        if (g_btl_formation[i] != CELL_EMPTY) {
                            BtlPlaceMember(g_btl_formation[i], col, row);
                        }
                        i++;
                    }
                }
                BtlBuildMarkers();
                BtlDespawnPickGrid();
                BtlPartyResetGfx();
                g_btl_step = PLACE_ASK;
                if (choice == PLACE_GRID_UNWIND) {
                    g_btl_place_abort = 1;
                }
                break;

            case PLACE_GRID_DONE:
                BtlSePlay(PICK_SE_BANK, PICK_SE_CHOSE);
                g_btl_step++;
                break;

            case PLACE_GRID_FINISH:
                BtlSePlay(PICK_SE_BANK, PICK_SE_CHOSE);
                BtlRetractMarkers();
                BtlCloseChoice0();
                BtlAfterTalk();
                while (BtlActorsIdle() == 0) {
                    BtlDrawFrame();
                }
                for (i = 0; i < BTL_PARTY; i++) {
                    if (g_btl_actors[i].c.key != 0
                        && ((signed char)g_btl_actors[i].c.status == BTL_STATUS_DOWN
                            || (g_btl_actors[i].flags & BTL_ACTOR_OUT) != 0)) {
                        BtlDespawnPickGrid();
                        BtlPickRefresh();
                        BtlRefreshMarkers();
                        BtlEnemiesResetGfx();
                        BtlPartyResetGfx();
                        if (BtlMarkMovedMembers() != 0) {
                            g_btl_formation_moved = 1;
                            return 1;
                        }
                        return 0;
                    }
                }
                BtlOpenChoice1();
                BtlOpenMessage(0, 0, g_btl_msg_keep_formation, PICK_HELP_X, PLACE_ASK_Y);
                BtlPlacePreset(PRESET_LIVE);
                g_btl_step = PLACE_KEEP;
                g_btl_choice1_row = 0;
                g_btl_delay = PLACE_KEEP_FRAMES;
                break;
            }
            break;

        case PLACE_CARRY:
            choice = BtlPlaceGridUpdate(1);
            switch (choice) {
            case PLACE_GRID_UNWIND:
            case BTL_PICK_CANCEL:
                BtlSePlay(PICK_SE_BANK, PICK_SE_CLOSED);
                g_btl_step--;
                if (choice == PLACE_GRID_UNWIND) {
                    g_btl_place_abort = 1;
                }
                break;

            case PLACE_GRID_DONE:
                BtlSePlay(PLACE_SE_SLOT, PLACE_SE_PUT);
                g_btl_step--;
                break;
            }
            break;

        case PLACE_KEEP:
            if (g_btl_delay != 0) {
                break;
            }
            choice = BtlChoiceUpdate(&g_btl_choice1_row);
            if (g_btl_place_abort != 0) {
                choice = BTL_PICK_ABORT;
            }
            switch (choice) {
            case BTL_PICK_ABORT:
            case BTL_PICK_CANCEL:
                BtlSePlay(PICK_SE_BANK, PICK_SE_CLOSED);
                BtlCloseMessage(0);
                BtlCloseChoice1();
                BtlRefreshMarkers();
                BtlOpenChoice0();
                g_btl_step = PLACE_PICK;
                if (choice == BTL_PICK_ABORT) {
                    g_btl_place_abort = 1;
                }
                break;

            case 0:
                BtlSePlay(PICK_SE_BANK, PICK_SE_CHOSE);
                BtlCloseMessage(0);
                BtlCloseChoice1();
                BtlOpenFormationBoard();
                g_btl_step++;
                break;

            case 1:
                BtlSePlay(PLACE_SE_SLOT, PLACE_SE_SHUT);
                BtlCloseMessage(0);
                BtlCloseChoice1();
                BtlDespawnPickGrid();
                BtlPickRefresh();
                BtlRefreshMarkers();
                BtlEnemiesResetGfx();
                BtlPartyResetGfx();
                if (BtlMarkMovedMembers() != 0) {
                    g_btl_formation_moved = 1;
                    return 1;
                }
                return 0;
            }
            break;

        case PLACE_STORE:
            choice = BtlPresetMenuUpdate(0);
            if (g_btl_place_abort != 0) {
                choice = BTL_PICK_ABORT;
            }
            switch (choice) {
            case BTL_PICK_WAIT:
                break;

            case BTL_PICK_ABORT:
            case BTL_PICK_CANCEL:
                BtlSePlay(PICK_SE_BANK, PICK_SE_CLOSED);
                BtlStandPreset(PRESET_LIVE);
                BtlCloseFormationBoard();
                BtlOpenChoice1();
                BtlOpenMessage(0, 0, g_btl_msg_keep_formation, PICK_HELP_X, PLACE_ASK_Y);
                BtlPartyResetGfx();
                BtlPartyClearAttr(PLACE_PREVIEW_ATTR);
                g_btl_step--;
                if (choice == BTL_PICK_ABORT) {
                    g_btl_place_abort = 1;
                }
                break;

            default:
                BtlSePlay(PICK_SE_BANK, PICK_SE_CHOSE);
                BtlCloseFormationBoard();
                if (BtlFormationPresetEmpty(g_btl_preset_row) != 0) {
                    g_btl_formation_preset[choice] = *(BtlFormation *)g_btl_formation;
                    BtlStandPreset(choice);
                    BtlDespawnPickGrid();
                    BtlRefreshMarkers();
                    BtlPickRefresh();
                    BtlPartyResetGfx();
                    BtlEnemiesResetGfx();
                    BtlPartyClearAttr(PLACE_PREVIEW_ATTR);
                    if (BtlMarkMovedMembers() != 0) {
                        g_btl_formation_moved = 1;
                        return 1;
                    }
                    return 0;
                }
                BtlOpenMessage(0, 0, g_btl_msg_overwrite_layout, PICK_HELP_X, PLACE_ASK_Y);
                slot = choice;
                BtlOpenChoice1();
                g_btl_choice1_row = 0;
                g_btl_step++;
                break;
            }
            break;

        case PLACE_OVERWRITE:
            choice = BtlChoiceUpdate(&g_btl_choice1_row);
            if (choice != BTL_MENU_WAIT) {
                if (choice == 0) {
                    BtlSePlay(PICK_SE_BANK, PICK_SE_CHOSE);
                    BtlSePlay(PLACE_SE_SLOT, PLACE_SE_SHUT);
                    g_btl_formation_preset[slot] = *(BtlFormation *)g_btl_formation;
                    BtlStandPreset(slot);
                    BtlCloseMessage(0);
                    BtlCloseChoice1();
                    BtlDespawnPickGrid();
                    BtlRefreshMarkers();
                    BtlPickRefresh();
                    BtlPartyResetGfx();
                    BtlEnemiesResetGfx();
                    BtlPartyClearAttr(PLACE_PREVIEW_ATTR);
                    if (BtlMarkMovedMembers() != 0) {
                        g_btl_formation_moved = 1;
                        return 1;
                    }
                    return 0;
                }
                BtlSePlay(PICK_SE_BANK, PICK_SE_CLOSED);
                BtlCloseMessage(0);
                BtlCloseChoice1();
                BtlOpenFormationBoard();
                if (choice == BTL_PICK_ABORT) {
                    g_btl_place_abort = 1;
                }
                g_btl_step--;
            }
            break;

        case PLACE_RECALL:
            choice = BtlPresetMenuUpdate(1);
            switch (choice) {
            case BTL_PICK_WAIT:
                break;

            case BTL_PICK_ABORT:
            case BTL_PICK_CANCEL:
                BtlSePlay(PLACE_SE_SLOT, PLACE_SE_SHUT);
                BtlDespawnPickGrid();
                BtlCloseFormationBoard();
                BtlRefreshMarkers();
                BtlOpenChoice0();
                BtlStandPreset(PRESET_LIVE);
                BtlPartyResetGfx();
                BtlPartyClearAttr(PLACE_PREVIEW_ATTR);
                g_btl_step = PLACE_ASK;
                if (choice == BTL_PICK_ABORT) {
                    g_btl_place_abort = 1;
                }
                break;

            default:
                if (BtlFormationPresetEmpty(choice) == 0
                    && BtlFormationPresetFits(choice) != 0) {
                    BtlSePlay(PICK_SE_BANK, PICK_SE_CHOSE);
                    *(BtlFormation *)g_btl_formation = g_btl_formation_preset[choice];
                    BtlStandPreset(choice);
                    BtlCloseFormationBoard();
                    BtlDespawnPickGrid();
                    BtlRefreshMarkers();
                    BtlPickRefresh();
                    BtlEnemiesResetGfx();
                    BtlPartyResetGfx();
                    BtlPartyClearAttr(PLACE_PREVIEW_ATTR);
                    if (BtlMarkMovedMembers() != 0) {
                        g_btl_formation_moved = 1;
                        return 1;
                    }
                    return 0;
                }
                break;
            }
            break;
        }
        BtlDrawFrame();
    }

}
#else
INCLUDE_ASM("btlp/nonmatchings/placemenu", BtlPlaceMenu);
#endif
