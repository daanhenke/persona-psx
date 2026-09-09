/* Persona 1 (JP) - does a saved formation preset fit the party we have?
 *   DNG 0x80090D1C   ADV 0x8008CBB8   S2D 0x80081228
 *
 * A unit of its own: DNG's marker placer ahead of it is not the one ADV and
 * S2D carry. See formation.h.
 */
#include <persona/common/formation.h>
#include <persona/common/char.h>

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
