/* Persona 1 (JP) - the battle formation grid.
 *
 * The party stands on a 5x5 grid. g_formation is the grid itself, one byte per
 * cell holding a party index or 0xFF for empty; g_formation_cell is the inverse
 * map, one cell index per party member. FormationCellFree is where the placement
 * rule lives: a cell is only usable when it and all four of its orthogonal
 * neighbours are empty, which is why a five-member party cannot fill the middle.
 *
 * Eight saved layouts sit in the save-game work area at g_formation_preset,
 * 25 bytes each, in the same cell-to-member form as the grid.
 *
 *                          DNG         ADV
 *   FormationLoadPreset    0x8009088C  0x8008C730
 *   FormationOtherAt       0x80090908  0x8008C7AC
 *   FormationSyncCells     0x8009095C  0x8008C800
 *   FormationCellOf        0x800909C0  0x8008C864
 *   FormationPresetCellOf  0x80090A08  0x8008C8AC
 *   FormationCellFree      0x80090AC0  0x8008C964
 *   FormationPresetFits    0x80090D1C  0x8008CBB8
 *
 * S2D carries its own copy against a work area 0x20000 higher; see
 * src/p1-jp/s2d/game/formation.c.
 */
#include <types.h>
#include <persona/common/slot.h>

#define g_formation        ((u_char *)0x800EB34C)
#define g_formation_cell   ((u_char *)0x800EB380)
#define g_formation_preset ((u_char *)0x801F2584)

#define GRID_W     5
#define GRID_H     5
#define GRID_CELLS 25
#define PARTY_MAX  5
#define CELL_EMPTY 0xFF

/* Highest occupied party index, so the party holds g_party_last + 1 members. */
extern u_char g_party_last;

/* Moves each member's marker sprite onto its cell, hiding the unplaced ones. */
extern void FormationPlaceMarkers(void);

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

/* -1 when the preset was never saved, 1 when it places exactly the party we
   have, 0 when it holds a different number of members. */
int FormationPresetFits(u_char preset)
{
    u_char *row;
    short   cell;
    short   n;
    short   highest;

    highest = 0;
    n = -1;
    row = &g_formation_preset[preset * GRID_CELLS];
    for (cell = 0; cell < GRID_CELLS; cell++) {
        if (row[cell] == CELL_EMPTY) {
            continue;
        }
        n++;
        if (highest < row[cell]) {
            highest = row[cell];
        }
    }
    if (n == -1) {
        return -1;
    }
    return n == g_party_last;
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
