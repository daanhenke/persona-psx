/* Persona 1 (JP) - whether a member may stand on a cell.  BTLP only.
 *   0x800A480C BtlFormationCellFree  0x800A4918 BtlFormationCellFreeOfFallen
 *
 * The battle's copy of the field's rule: a member may only stand where the
 * four cells round it are empty. BtlFormationCellFree asks it of the live grid
 * and leaves the cell itself to its caller.
 *
 * BtlFormationCellFreeOfFallen asks it again of g_btl_formation_fallen, a copy
 * of the grid with every member who is down where it stood put back on the
 * cell it holds - so a member being moved cannot be put beside a fallen one,
 * or on its cell. This time the cell itself has to be empty as well.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/formation.h>

/* A cell of either grid by column and row. Both routines read like these
   were macros: the index is worked out afresh for every test, col and row
   narrowed again at every use, and the neighbours left and right reached off
   the address of the cell itself. Written through a `cell` local instead, the
   index is kept, the addresses are built differently, and the fallen grid's
   copy loses the eight bytes of frame the image reserves. The tests below the
   cell are each an early return with the 1 at the end, as the others are. */
#define CELL(c, r)   (g_btl_formation[(r) * GRID_W + (c)])
#define FALLEN(c, r) (g_btl_formation_fallen.cell[(r) * GRID_W + (c)])

int BtlFormationCellFree(short col, short row)
{
    if (col > 0 && (&CELL(col, row))[-1] != CELL_EMPTY) {
        return 0;
    }
    if (col < GRID_W - 1 && (&CELL(col, row))[1] != CELL_EMPTY) {
        return 0;
    }
    if (row > 0 && CELL(col, row - 1) != CELL_EMPTY) {
        return 0;
    }
    if (row < GRID_H - 1 && CELL(col, row + 1) != CELL_EMPTY) {
        return 0;
    }
    return 1;
}

int BtlFormationCellFreeOfFallen(short col, short row)
{
    BtlActor *a;
    int       i;

    g_btl_formation_fallen = *(BtlFormation *)g_btl_formation;
    a = g_btl_actors;
    for (i = 0; i < BTL_PARTY; i++, a++) {
        if (a->c.key != 0 && (signed char)a->c.status == BTL_STATUS_DOWN
            && a->revive_mark == BTL_REVIVE_STOOD) {
            FALLEN(a->place_col, a->place_row) = i;
        }
    }
    if (FALLEN(col, row) != CELL_EMPTY) {
        return 0;
    }
    if (col > 0 && (&FALLEN(col, row))[-1] != CELL_EMPTY) {
        return 0;
    }
    if (col < GRID_W - 1 && (&FALLEN(col, row))[1] != CELL_EMPTY) {
        return 0;
    }
    if (row > 0 && FALLEN(col, row - 1) != CELL_EMPTY) {
        return 0;
    }
    if (row < GRID_H - 1 && FALLEN(col, row + 1) != CELL_EMPTY) {
        return 0;
    }
    return 1;
}
