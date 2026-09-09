/* Persona 1 (JP) - putting the formation grid back in order.  ADV only.
 *   ADV 0x8008C630
 *
 * Drops anyone past the end of the party off the grid, then gives every member
 * left without a cell the first one the placement rule allows. A unit of its
 * own: DNG and S2D do not carry it, and their copy of the grid unit in
 * formation.c starts at the preset loader instead. See formation.h.
 */
#include <persona/common/formation.h>
#include <persona/common/char.h>

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
