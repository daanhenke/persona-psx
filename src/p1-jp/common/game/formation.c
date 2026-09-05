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
 * S2D builds this same source against a work area 0x20000 higher, which is
 * what WORK_BIAS says.
 */
#include <types.h>
#include <persona/common/char.h>
#include <persona/common/slot.h>

#define g_formation        ((u_char *)(0x800EB34C + WORK_BIAS))
#define g_formation_cell   ((u_char *)(0x800EB380 + WORK_BIAS))
#define g_formation_scratch ((u_char *)(0x800EB365 + WORK_BIAS))
#define g_formation_preset ((u_char *)0x801F2584)

#define GRID_W     5
#define GRID_H     5
#define GRID_CELLS 25
#define PARTY_MAX  5
#define CELL_EMPTY 0xFF

/* The five grid markers live in slots 27..31, laid out sixteen pixels apart
   across and eight down from the grid's top left. The slot record is reached
   from the first marker's address rather than through g_slots, which is what
   the rematerialised base in the loop says the original did. */
#define g_marker_slot ((Slot *)(0x800DC838 + WORK_BIAS))
#define MARKER_SLOT   0x1B
#define MARKER_Z      6
#define MARKER_X0     0xD8
#define MARKER_Y0     0x10
#define MARKER_XPITCH 16
#define MARKER_YPITCH 8

/* Highest occupied party index, so the party holds g_party_last + 1 members. */
extern u_char g_party_last;

extern Slot *g_slot_cur;
/* One sprite definition per party member. */
extern void *g_formation_marker_def[];

/* The party's own markers, in slots 2..6, are drawn on the grid isometrically
   rather than on the flat layout the position markers use: eleven pixels of x
   per column against nine per row, four of y per column against three per row.
   The sort depth counts down from 0x18 so a nearer row draws in front. */
#define g_member_slot ((Slot *)(0x800DC194 + WORK_BIAS))
#define MEMBER_SLOT   2
#define MEMBER_X0     0x84
#define MEMBER_Y0     0x35
#define MEMBER_Z0     0x18
#define MEMBER_COL_X  11
#define MEMBER_ROW_X  9
#define MEMBER_COL_Y  4
#define MEMBER_ROW_Y  3

/* One texture holds every face; the slot's u offset picks one. */
#define MEMBER_PORTRAIT_W 16

extern void g_formation_member_def;

/* Reached by hardcoded address here rather than through the linker symbol. */
#define g_party_at ((u_char *)0x801F256C)

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

/* Moves each member's marker sprite onto its cell. A member who is not on the
   grid still gets a sprite - placed from cell 0xFF, so off the right-hand side
   - and is hidden instead. */
void FormationPlaceMarkers(void)
{
    u_char *cells;
    u_char  member;
    u_char  cell;
    u_char  col;
    u_char  row;

    cells = g_formation_cell;
    for (member = 0; member < PARTY_MAX; member++) {
        g_slot_cur = &g_marker_slot[member];
        cell = cells[member];
        row = cell / GRID_W;
        col = cell % GRID_W;
        SlotInitTagged(g_formation_marker_def[member], MARKER_SLOT + member,
                       MARKER_Z, col * MARKER_XPITCH + MARKER_X0,
                       row * MARKER_YPITCH + MARKER_Y0);
        if (cells[member] == CELL_EMPTY) {
            g_slot_cur->attr |= SLOT_ATTR_HIDE;
        }
    }
}

/* Places one member's marker on its cell, or clears the slot when the member
   is off the grid. */
void FormationSetMemberSprite(u_char member, u_char portrait)
{
    u_char cell;
    u_char col;
    u_char row;
    int    dx;

    cell = g_formation_cell[member];
    if (cell != CELL_EMPTY) {
        row = cell / GRID_W;
        col = cell % GRID_W;
        g_slot_cur = &g_member_slot[member];
        dx = row * MEMBER_ROW_X - MEMBER_X0;
        SlotInitTagged(&g_formation_member_def, MEMBER_SLOT + member,
                       MEMBER_Z0 - (row * GRID_W + col),
                       col * MEMBER_COL_X - dx,
                       row * MEMBER_ROW_Y + col * MEMBER_COL_Y + MEMBER_Y0);
        g_slot_cur->u_add = portrait * MEMBER_PORTRAIT_W;
    } else {
        SlotClear(MEMBER_SLOT + member);
    }
}

/* The character's key doubles as its portrait number, counting from one.

   The member counter is an int: the party bound is compared signed, which is
   what puts the always-false `g_party_last < 0` guard ahead of the loop. */
void FormationDrawMembers(void)
{
    Char *chars;
    int   member;

    chars = g_chars;
    for (member = 0; member <= g_party_last; member++) {
        FormationSetMemberSprite(member, chars[g_party_at[member]].key - 1);
    }
}

/* Drops anyone past the end of the party off the grid, then gives every member
   left without a cell the first one the placement rule allows. */
void FormationRepair(void)
{
    u_char *grid;
    u_char  i;
    u_char  free;

    /* One counter serves both loops - it is the same register in the original,
       which only happens if it is the same variable. */
    grid = g_formation;
    for (i = 0; i < GRID_CELLS; i++) {
        if (grid[i] != CELL_EMPTY && g_party_last < grid[i]) {
            grid[i] = CELL_EMPTY;
        }
    }
    for (i = 0; i <= g_party_last; i++) {
        if (FormationCellOf(i) == CELL_EMPTY) {
            for (free = 0; FormationCellFree(free) == 0; free++) {
            }
            grid[free] = i;
        }
    }
    FormationSyncCells();
    FormationPlaceMarkers();
    FormationDrawMembers();
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
