/* Persona 1 (JP) - one frame of the placement menu's grid cursor.  BTLP only.
 *   0x800A4020 BtlPlaceGridUpdate
 *
 * The placement menu runs this twice over, once in each of its two states:
 * with nothing in hand, where the directions walk the members who have not
 * moved yet and the confirm lifts one off the grid, and with a member in hand,
 * where the directions move the cursor and the confirm puts it down.
 *
 * The walk and the cursor move together either way - each direction both steps
 * a row or a column and steps the member the cursor would pick up - so the
 * member offered is always the one under the cursor.
 *
 * A member put down on an empty cell has to pass the same four-neighbour rule
 * the field keeps; put down on an occupied one it swaps with whoever is there,
 * which is refused if that member's marker is down.
 *
 * Answers PLACE_GRID_DONE as a member is lifted or put down, BTL_PICK_CANCEL
 * on a cancel, PLACE_GRID_FINISH once the player is done moving,
 * PLACE_GRID_UNWIND on the third key, and BTL_PICK_WAIT otherwise.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/input.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/sound.h>

/* The grid's four sides, and how far round the cursor wraps. */
#define PLACE_GRID_SIDE 5

/* The motion a member is lifted onto, and the cursor's two: one under the
   member being offered, nought under the rest. */
#define PLACE_LIFT_MOTION   0xA
#define PLACE_CURSOR_ON     1
#define PLACE_CURSOR_OFF    0

/* What a cursor is walked to and how fast: full grey for a member that can
   still be moved, an eighth for one that cannot. */
#define PLACE_CURSOR_LIT  0x80
#define PLACE_CURSOR_DIM  0x20
#define PLACE_CURSOR_FADE 0xFF

/* A member whose marker has come down has moved already. */
#define PLACE_MARKER_UP 2

/* The sound a direction makes. */
#define PLACE_SE_MOVE_SLOT 1
#define PLACE_SE_MOVE      0

/* Where the member the cursor lifted came from, and the slot the walk is on
   before the pick is settled. */
extern short   g_btl_place_from_col;
extern short   g_btl_place_from_row;
extern int     g_btl_place_walk;

/* 95.45%. Every branch, call and store is the image's. What is left is the
   two wraps: where a direction takes a row or a column past the last, the
   image works the new value out with a mask off the comparison and stores it
   once, and gcc branches round a second store. The same wrap written as
   cursor.c writes its own - `n = n + 1; n = n < 5 ? n : 0;` - does produce the
   mask, but only with `n` an int, and here the comparison is on a short: the
   image loads the field unsigned, adds one and sign-extends the sum, which no
   typing of the temporary has reproduced. */
#ifdef NON_MATCHING
int BtlPlaceGridUpdate(int carrying)
{
    BtlObj **cursor;
    int      key;
    int      i;
    short    cell;
    short    at;

    g_btl_place_walk = g_btl_place_member;
    key = BtlMenuKey();
    key &= 0xFFFF;
    if ((key & PAD_DIRS) != 0) {
        BtlSePlay(PLACE_SE_MOVE_SLOT, PLACE_SE_MOVE);
    }
    if ((key & PAD_UP) != 0) {
        g_btl_place_walk = BtlUnreadyMemberPrev(g_btl_place_walk);
        g_btl_place_row--;
        if (g_btl_place_row < 0) {
            g_btl_place_row = PLACE_GRID_SIDE - 1;
        }
    }
    if ((key & PAD_DOWN) != 0) {
        g_btl_place_walk = BtlUnreadyMemberNext(g_btl_place_walk);
        g_btl_place_row++;
        if (g_btl_place_row >= PLACE_GRID_SIDE) {
            g_btl_place_row = 0;
        }
    }
    if ((key & PAD_LEFT) != 0) {
        g_btl_place_walk = BtlUnreadyMemberPrev(g_btl_place_walk);
        g_btl_place_col--;
        if (g_btl_place_col < 0) {
            g_btl_place_col = PLACE_GRID_SIDE - 1;
        }
    }
    if ((key & PAD_RIGHT) != 0) {
        g_btl_place_walk = BtlUnreadyMemberNext(g_btl_place_walk);
        g_btl_place_col++;
        if (g_btl_place_col >= PLACE_GRID_SIDE) {
            g_btl_place_col = 0;
        }
    }

    at = g_btl_place_row * PLACE_GRID_SIDE + g_btl_place_col;
    cell = g_btl_formation[at];
    g_btl_place_cell = cell;
    if (carrying == 0) {
        g_btl_place_member = g_btl_place_walk;
        g_btl_grid_anchor->attr |= BTL_OBJ_HIDDEN;
        if (g_btl_place_member < 0) {
            g_btl_grid_back->attr &= ~BTL_OBJ_HIDDEN;
            BtlPartyResetGfx();
        } else {
            g_btl_grid_back->attr |= BTL_OBJ_HIDDEN;
            g_btl_place_col = g_btl_actors[g_btl_place_member].obj->col2 >> 1;
            g_btl_place_row = g_btl_actors[g_btl_place_member].obj->row;
        }
    } else {
        g_btl_grid_anchor->attr &= ~BTL_OBJ_HIDDEN;
        if (cell != CELL_EMPTY
            && g_btl_actors[cell].marker < PLACE_MARKER_UP) {
            g_btl_actors[cell].obj->motion = PLACE_LIFT_MOTION;
        }
    }

    cursor = g_btl_pick_cursors;
    for (i = 0; i < BTL_PARTY; i++, cursor++) {
        if (g_btl_place_member == i) {
            (*cursor)->motion = PLACE_CURSOR_ON;
        } else {
            (*cursor)->motion = PLACE_CURSOR_OFF;
            if (g_btl_actors[i].marker < PLACE_MARKER_UP) {
                (*cursor)->rgb_to[0] = PLACE_CURSOR_LIT;
                (*cursor)->rgb_to[1] = PLACE_CURSOR_LIT;
                (*cursor)->rgb_to[2] = PLACE_CURSOR_LIT;
            } else {
                (*cursor)->rgb_to[0] = PLACE_CURSOR_DIM;
                (*cursor)->rgb_to[1] = PLACE_CURSOR_DIM;
                (*cursor)->rgb_to[2] = PLACE_CURSOR_DIM;
            }
            (*cursor)->fade = PLACE_CURSOR_FADE;
        }
    }

    if (g_btl_place_member >= 0) {
        for (i = 0; i < BTL_PARTY; i++) {
            if (g_btl_actors[i].c.key != 0
                && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
                && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
                if (i == g_btl_place_member) {
                    g_btl_actors[i].obj->motion = PLACE_LIFT_MOTION;
                } else {
                    BtlMemberResetGfx(i);
                }
            }
        }
    }

    g_btl_grid_anchor->x = (g_btl_place_col * PLACE_XPITCH + PLACE_X)
                           * PLACE_FIXED;
    g_btl_grid_anchor->y = (g_btl_place_row * PLACE_YPITCH + PLACE_Y)
                           * PLACE_FIXED;
    at = g_btl_place_row * PLACE_GRID_SIDE + g_btl_place_col;
    if ((g_btl_pad1_edge & g_btl_key_confirm) != 0) {
        if (carrying == 0) {
            if (g_btl_place_member < 0) {
                g_btl_grid_back->attr |= BTL_OBJ_HIDDEN;
                return PLACE_GRID_FINISH;
            }
            g_btl_formation[at] = CELL_EMPTY;
            g_btl_place_from_col = g_btl_place_col;
            g_btl_place_from_row = g_btl_place_row;
            return PLACE_GRID_DONE;
        }
        if (g_btl_place_cell == CELL_EMPTY) {
            if (BtlFormationCellFree(g_btl_place_col, g_btl_place_row) == 0) {
                return BTL_PICK_WAIT;
            }
            g_btl_formation[at] = g_btl_place_member;
            BtlPlaceMember(g_btl_place_member, g_btl_place_col,
                           g_btl_place_row);
        } else {
            if (g_btl_actors[g_btl_place_cell].marker >= PLACE_MARKER_UP) {
                return BTL_PICK_WAIT;
            }
            g_btl_formation[at] = g_btl_place_member;
            g_btl_formation[g_btl_place_from_row * PLACE_GRID_SIDE
                            + g_btl_place_from_col] = g_btl_place_cell;
            BtlPlaceMember(g_btl_place_member, g_btl_place_col,
                           g_btl_place_row);
            BtlPlaceMember(g_btl_place_cell, g_btl_place_from_col,
                           g_btl_place_from_row);
        }
        BtlBuildMarkers();
        BtlRefreshPickCursors();
        return PLACE_GRID_DONE;
    }
    if ((g_btl_pad1_edge & g_btl_key_cancel) != 0) {
        if (carrying == 0) {
            if (g_btl_place_member >= 0) {
                g_btl_place_member = -1;
                return BTL_PICK_WAIT;
            }
        } else {
            g_btl_formation[g_btl_place_from_row * PLACE_GRID_SIDE
                            + g_btl_place_from_col] = g_btl_place_member;
        }
        BtlBuildMarkers();
        BtlRefreshPickCursors();
        return BTL_PICK_CANCEL;
    }
    if ((g_btl_pad1_edge & g_btl_key_abort) != 0) {
        return PLACE_GRID_UNWIND;
    }
    return BTL_PICK_WAIT;
}
#else
INCLUDE_ASM("btlp/nonmatchings/placegrid", BtlPlaceGridUpdate);
#endif
