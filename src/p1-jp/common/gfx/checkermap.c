/* Persona 1 (JP) - the checkerboard the menus sit on.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   DNG @ 0x80092FB4   ADV @ 0x8008EF14   S2D @ 0x800834C8
 *
 * A 10x8 character map of four cells laid out two by two, so it tiles as a
 * checkerboard. Only the index array is built here; the cells themselves come
 * from the map's own definition.
 *
 * S2D builds this same source against an index array 0x20000 higher, which is
 * what WORK_BIAS says.
 */
#include <types.h>

/* Reached by hardcoded address; the GsMAP at 0x800B8350 points at it. */
#define g_checker_index ((short *)(0x800F1D80 + WORK_BIAS))

#define CHECKER_W 10
#define CHECKER_H 8

void CheckerMapInit(void)
{
    int i;

    for (i = 0; i < CHECKER_W * CHECKER_H; i++) {
        g_checker_index[i] = ((i / CHECKER_W) & 1) * 2 + (i & 1) + 1;
    }
}
