/* Persona 1 (JP) - closing the party's formation up.  BTLP only.
 *   0x8009BD44 BtlFormationCloseUp
 *
 * The battle inherits the field's 5x5 formation grid, one byte a cell holding
 * a party index or 0xFF for an empty one, and it keeps its copy in the
 * resident EXE's work area so the overlay loading over the field code does not
 * take it away.
 *
 * Whichever way the party was arranged, it fights from the front: while the
 * whole front row is empty the grid is shifted up a row and the back row
 * filled in as empty. The overlay's entry point runs this straight after
 * mirroring the grid end for end, which it does when the encounter comes in on
 * mode 2 - the mirror turns the party around and this pulls it forward again.
 */
#include <types.h>

#define GRID_W     5
#define GRID_CELLS 25
#define CELL_EMPTY 0xFF

extern u_char g_btl_formation[];

void BtlFormationCloseUp(void)
{
    /* The empty cell is held in a variable of its own: the walk compares
       against it and the back row is filled from it, and the original keeps
       one register for both. */
    u_char  empty;
    u_char *back;
    int     i;
    int     j;

    empty = CELL_EMPTY;
    i = 0;
    /* Two ways back to the top - one per cell tested, one after a row has
       been shifted away - which is why this is spelt with a label. */
top:
    if (g_btl_formation[i] != empty) {
        return;
    }
    i++;
    if (i < GRID_W) {
        goto top;
    }
    for (j = 0; j < GRID_CELLS - GRID_W; j++) {
        g_btl_formation[j] = g_btl_formation[j + GRID_W];
    }
    j = GRID_W - 1;
    back = &g_btl_formation[GRID_CELLS - 1];
    for (; j >= 0; j--) {
        *back-- = empty;
    }
    i = 0;
    goto top;
}
