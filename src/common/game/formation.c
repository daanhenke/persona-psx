/* Persona 1 (JP) - the battle formation grid.
 *   DNG 0x8009088C   ADV 0x8008C730   S2D as ADV, work area 0x20000 up
 *
 * The grid and the placement rule. The markers, the preset fit test, the
 * member sprites and the compaction are units of their own beside this one;
 * ADV alone carries the repair pass ahead of it, in formationrepair.c. See
 * formation.h.
 */
#include <persona/common/formation.h>
#include <persona/common/char.h>

void FormationLoadPreset(u_char preset)
{
    u_char *cells;
    u_char  member;

    cells = g_formation_cell;
    for (member = 0; member < PARTY_MAX; member++) {
        cells[member] = FormationPresetCellOf(member, preset);
    }
    FormationPlaceMarkers();
}

/* Whoever other than `member` is standing on `cell`. The editor uses this to
   find the member to swap with when a move lands on an occupied cell. */
u_char FormationOtherAt(u_char member, u_char cell)
{
    u_char *cells;
    u_char  i;

    cells = g_formation_cell;
    for (i = 0; i < PARTY_MAX; i++) {
        if (cells[i] == cell && i != member) {
            return i;
        }
    }
    return CELL_EMPTY;
}

/* Rebuilds the member-to-cell map from the grid. */
void FormationSyncCells(void)
{
    u_char *cells;
    u_char  member;

    cells = g_formation_cell;
    for (member = 0; member < PARTY_MAX; member++) {
        cells[member] = FormationCellOf(member);
    }
}

/* Which cell `member` stands on, or 0xFF if it is off the grid. */
u_char FormationCellOf(u_char member)
{
    u_char *grid;
    u_char  cell;

    grid = g_formation;
    for (cell = 0; cell < GRID_CELLS; cell++) {
        if (grid[cell] == member) {
            return cell;
        }
    }
    return CELL_EMPTY;
}

u_char FormationPresetCellOf(u_char member, u_char preset)
{
    u_char *row;
    u_char  cell;

    row = &g_formation_preset[preset * GRID_CELLS];
    for (cell = 0; cell < GRID_CELLS; cell++) {
        if (row[cell] == member) {
            return cell;
        }
    }
    return CELL_EMPTY;
}

/* First cell a member may stand on. Runs off the end when the grid has no room
   left, which the callers do not check for. */
u_char FormationFirstFree(void)
{
    u_char cell;

    for (cell = 0; cell < GRID_CELLS; cell++) {
        if (FormationCellFree(cell)) {
            return cell;
        }
    }
}

/* The placement rule does not come out of the C yet; the overlays take it
   from asm, so it is kept here for the progress build only. */
#ifdef NON_MATCHING
/* A member may only stand on an empty cell whose four orthogonal neighbours are
   also empty. Each edge test carries both bounds even though one half of it is
   always true; `last` holds a constant, and the casts on the neighbour indices
   decide which way round the address addition comes out. Leave all three. */
u_char FormationCellFree(u_char cell)
{
    u_char *grid;
    int     ok;
    int     last;

    ok = 1;
    grid = g_formation;
    if (grid[cell] != CELL_EMPTY) {
        return 0;
    }
    {
        int row = (cell / GRID_W) & 0xFF;
        if (row < GRID_H && row != 0 && grid[cell - GRID_W] != CELL_EMPTY) {
            return 0;
        }
    }
    last = GRID_H - 1;
    {
        int row = (cell / GRID_W) & 0xFF;
        if (row < last && row >= 0 && grid[(long)(cell + GRID_W)] != CELL_EMPTY) {
            return 0;
        }
    }
    {
        int col = (cell % GRID_W) & 0xFF;
        if (col < GRID_W && col != 0 && grid[(long)(cell - 1)] != CELL_EMPTY) {
            return 0;
        }
    }
    {
        int col = (cell % GRID_W) & 0xFF;
        if (col < last && col >= 0 && grid[(long)(cell + 1)] != CELL_EMPTY) {
            return 0;
        }
    }
    return ok;
}
#endif
