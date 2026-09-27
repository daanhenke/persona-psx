/* Persona 1 (JP) - putting the formation grid back in order.
 *   ADV 0x8008C548 FormationRepairGrid  0x8008C630 FormationRepair
 *   DNG 0x8009076C FormationRepairGrid  0x80090854 FormationRepair
 *
 * Drops anyone past the end of the party off the grid, then gives every member
 * left without a cell the first one the placement rule allows. A unit of its
 * own, ahead of the grid unit in formation.c (which starts at the preset
 * loader). See formation.h.
 */
#include <persona/common/formation.h>
#include <persona/common/char.h>

/* Drops anyone past the end of the party off the grid, then gives every member
   left without a cell the first one the placement rule allows. ADV declares
   it inline (FORMATION_REPAIR_INLINE): the routine is still emitted, and
   FormationRepair gets a copy of the body where DNG and S2D call it. */
#ifdef FORMATION_REPAIR_INLINE
inline
#endif
void FormationRepairGrid(void)
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
}

void FormationRepair(void)
{
    FormationRepairGrid();
    FormationSyncCells();
    FormationPlaceMarkers();
    FormationDrawMembers();
}
