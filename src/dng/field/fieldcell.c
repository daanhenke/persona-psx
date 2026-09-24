/* Persona 1 (JP) - the minimap window's cells, and restarting the sound.
 * DNG only.
 *   0x8006E988 FieldSetCell
 *   0x8006EA5C FieldResetSound
 */
#include <decomp/types.h>
#include <libsnd.h>
#include <persona/dng/field.h>

/* The minimap shows an 11 by 11 window that wraps round the floor, one
   halfword per cell; tile (x, y) always lands in cell (x % 11, y % 11). */
#define WINDOW 11

/* Copies tile (x, y)'s minimap icon into its window cell. */
void FieldSetCell(int x, int y)
{
    u_short *cell;

    cell = &((u_short (*)[WINDOW])(PACK_BASE + 8 + *g_pack_cell_tab))[y % WINDOW][x % WINDOW];
    *cell = g_tile_defs[g_floor_grid[y][x]].icon;
}

/* Restarts libsnd and forgets every open sequence. */
void FieldResetSound(void)
{
    int i;

    SsEnd();
    SsQuit();
    SsInit();
    for (i = 0; i < SEQ_HANDLES; i++) {
        g_seq_handles[i] = -1;
    }
}
