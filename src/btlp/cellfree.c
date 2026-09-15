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
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/formation.h>

/* 90.97%: every test is the image's but the one above the cell, whose return
   the image lays out in line - a beq on to the test below, then the jump to
   the return - where gcc here shares it with the two before. The last test
   written return-1-first is what puts the 1 in the delay slot of its branch;
   an ok local, the up test guarding the rest, an else chain, a ternary and an
   explicit else all leave the up test's return shared. The two names at a
   negative offset from g_btl_formation are D_ labels in the image and cannot
   be spelt as a symbol here. */
#ifdef NON_MATCHING
int BtlFormationCellFree(short col, short row)
{
    if (col > 0 && g_btl_formation[row * GRID_W + col - 1] != CELL_EMPTY) {
        return 0;
    }
    if (col < GRID_W - 1
        && g_btl_formation[row * GRID_W + col + 1] != CELL_EMPTY) {
        return 0;
    }
    if (row > 0 && g_btl_formation[(row - 1) * GRID_W + col] != CELL_EMPTY) {
        return 0;
    }
    if (row >= GRID_H - 1) {
        return 1;
    }
    return g_btl_formation[(row + 1) * GRID_W + col] == CELL_EMPTY;
}
#else
INCLUDE_ASM("btlp/nonmatchings/cellfree", BtlFormationCellFree);
#endif

/* 85.54%: the same up test, and more. The image keeps an eight-byte frame in
   this leaf - a byte local nothing here has been found for (neither the
   counter as u_char nor as short) - takes col and row into t0 and a3 the
   other way round, and builds the fallen grid's address into a register ahead
   of the cell index, reaching the left neighbour off that sum. */
#ifdef NON_MATCHING
int BtlFormationCellFreeOfFallen(short col, short row)
{
    BtlActor *a;
    int       cell;
    int       i;

    g_btl_formation_fallen = *(BtlFormation *)g_btl_formation;
    a = g_btl_actors;
    for (i = 0; i < BTL_PARTY; i++, a++) {
        if (a->c.key != 0 && (signed char)a->c.status == BTL_STATUS_DOWN
            && a->revive_mark == BTL_REVIVE_STOOD) {
            g_btl_formation_fallen.cell[a->place_row * GRID_W + a->place_col] =
                i;
        }
    }
    cell = row * GRID_W + col;
    if (g_btl_formation_fallen.cell[cell] != CELL_EMPTY) {
        return 0;
    }
    if (col > 0 && (g_btl_formation_fallen.cell + cell)[-1] != CELL_EMPTY) {
        return 0;
    }
    if (col < GRID_W - 1
        && g_btl_formation_fallen.cell[cell + 1] != CELL_EMPTY) {
        return 0;
    }
    if (row > 0
        && g_btl_formation_fallen.cell[(row - 1) * GRID_W + col] != CELL_EMPTY) {
        return 0;
    }
    if (row >= GRID_H - 1) {
        return 1;
    }
    return g_btl_formation_fallen.cell[(row + 1) * GRID_W + col] == CELL_EMPTY;
}
#else
INCLUDE_ASM("btlp/nonmatchings/cellfree", BtlFormationCellFreeOfFallen);
#endif
