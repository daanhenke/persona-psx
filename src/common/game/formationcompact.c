/* Persona 1 (JP) - formation row compaction and marker clear.
 *
 *   ADV 0x8008D1FC ..
 *
 * The tail of the member-sprite unit, past the per-member placement that is
 * still taken from asm. See formation.h.
 */
#include <persona/common/formation.h>
#include <persona/common/char.h>

/* Slides the party forward over any empty rows at the front of the grid, so a
   formation loaded from a preset does not leave the party standing at the
   back. Whole rows only: the leading empty cells are counted and rounded down
   to a row. The 25 bytes above the grid are scratch, used by nothing else.
   Each member's cell index is then adjusted by the row count. */
void FormationCompact(void)
{
    u_char *grid;
    u_char *packed;
    u_char *cells;
    u_char  empty;
    u_char  rows;
    u_char  i;
    u_char  j;

    grid = g_formation;
    packed = g_formation_scratch;
    cells = g_formation_cell;
    empty = 0;
    while (grid[empty] == CELL_EMPTY) {
        empty++;
    }
    rows = empty / GRID_W;
    if (rows == 0) {
        return;
    }
    for (i = 0; i < GRID_CELLS; i++) {
        packed[i] = CELL_EMPTY;
    }
    j = 0;
    for (i = rows * GRID_W; i < GRID_CELLS; i++, j++) {
        packed[j] = grid[i];
    }
    for (i = 0; i < GRID_CELLS; i++) {
        grid[i] = packed[i];
    }
    for (i = 0; i < PARTY_MAX; i++) {
        if (cells[i] != CELL_EMPTY) {
            cells[i] = cells[i] - rows;
        }
    }
}

/* Both marker sets the formation screen puts on screen: one sprite per party
   member in slots 2..6, and the five that sit on the grid itself. */
void FormationClearMarkers(void)
{
    SlotClear(2);
    SlotClear(3);
    SlotClear(4);
    SlotClear(5);
    SlotClear(6);
    SlotClear(0x1B);
    SlotClear(0x1C);
    SlotClear(0x1D);
    SlotClear(0x1E);
    SlotClear(0x1F);
}
