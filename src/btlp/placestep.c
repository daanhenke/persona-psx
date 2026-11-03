/* Persona 1 (JP) - one frame of moving a member in the placement menu.
 * BTLP only.
 *   0x800A78D8 BtlPlaceMoveStep
 *
 * The cursor BtlPlaceFallenStep walks, taken round the grid for a member who
 * is standing: the same directions wrapping at either edge, and the grid's
 * anchor kept over the cell. A confirm on an empty cell that the four-neighbour
 * rule allows both on the live grid and with the fallen put back stands the
 * member there, and the cell is kept on the actor.
 *
 * The answer is 1 once the member has moved, -1 on a cancel, -2 on the third
 * key, and BTL_PICK_WAIT while nothing has been decided.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/input.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/object.h>
#include <persona/btlp/sound.h>

/* The answers a cancel and the third key give. */
#define PLACE_CANCEL (-1)
#define PLACE_ABORT  (-2)

/* Test the sign of the low halfword while retaining the stepped coordinate
   for both stores. This preserves the original wrap and register lifetimes. */
int BtlPlaceMoveStep(BtlActor *a)
{
    int next;
    int keys;

    g_btl_grid_anchor->attr &= ~BTL_OBJ_HIDDEN;
    keys = BtlMenuKey();
    if (keys & PAD_DIRS) {
        BtlSePlay(1, 0);
    }
    if (keys & PAD_UP) {
        {
            int value = (u_short)g_btl_place_row - 1;
            int coordinate = value;
            value = (u_int)value << 16;
            g_btl_place_row = coordinate;
            if (value < 0) {
                coordinate = GRID_H - 1;
                g_btl_place_row = coordinate;
            }
        }
    }
    if (keys & PAD_DOWN) {
        next = (unsigned short)g_btl_place_row;
        next = next + 1;
        next &= -((short)next < GRID_H);
        g_btl_place_row = next;
    }
    if (keys & PAD_LEFT) {
        {
            int value = (u_short)g_btl_place_col - 1;
            int coordinate = value;
            value = (u_int)value << 16;
            g_btl_place_col = coordinate;
            if (value < 0) {
                coordinate = GRID_W - 1;
                g_btl_place_col = coordinate;
            }
        }
    }
    if (keys & PAD_RIGHT) {
        next = (unsigned short)g_btl_place_col;
        next = next + 1;
        next &= -((short)next < GRID_W);
        g_btl_place_col = next;
    }

    g_btl_grid_anchor->x = (g_btl_place_col * PLACE_XPITCH + PLACE_X) << 16;
    g_btl_grid_anchor->y = (g_btl_place_row * PLACE_YPITCH + PLACE_Y) << 16;
    g_btl_place_cell = g_btl_formation[g_btl_place_row * GRID_W + g_btl_place_col];
    if ((g_btl_pad1_edge & g_btl_key_confirm) && g_btl_place_cell == CELL_EMPTY
        && BtlFormationCellFree(g_btl_place_col, g_btl_place_row)
        && BtlFormationCellFreeOfFallen(g_btl_place_col, g_btl_place_row)) {
        BtlSePlay(PLACE_SE_SLOT, PLACE_SE_PUT);
        BtlPlaceMember(a->obj->mark_num, g_btl_place_col, g_btl_place_row);
        a->place_col = g_btl_place_col;
        a->place_row = g_btl_place_row;
        return 1;
    }
    if (g_btl_pad1_edge & g_btl_key_cancel) {
        return PLACE_CANCEL;
    }
    if (g_btl_pad1_edge & g_btl_key_abort) {
        return PLACE_ABORT;
    }
    return BTL_PICK_WAIT;
}
