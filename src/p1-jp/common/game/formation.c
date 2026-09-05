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

#define ROW(cell) ((u_char)((cell) / GRID_W))
#define COL(cell) ((u_char)((cell) % GRID_W))

/* A member may only stand on an empty cell whose four orthogonal neighbours are
   also empty. Each edge test is written with both bounds even though one half is
   always true, which is why the row and column come out recomputed per test. */
u_char FormationCellFree(u_char cell)
{
    u_char *grid;

    grid = g_formation;
    if (grid[cell] != CELL_EMPTY) {
        return 0;
    }
    if (ROW(cell) < GRID_H && ROW(cell) != 0 &&
        grid[cell - GRID_W] != CELL_EMPTY) {
        return 0;
    }
    if (ROW(cell) < GRID_H - 1 && ROW(cell) >= 0 &&
        grid[cell + GRID_W] != CELL_EMPTY) {
        return 0;
    }
    if (COL(cell) < GRID_W && COL(cell) != 0 &&
        grid[cell - 1] != CELL_EMPTY) {
        return 0;
    }
    if (COL(cell) < GRID_W - 1 && COL(cell) >= 0 &&
        grid[cell + 1] != CELL_EMPTY) {
        return 0;
    }
    return 1;
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
