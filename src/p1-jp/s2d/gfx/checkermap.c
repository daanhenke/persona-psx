/* Persona 1 (JP) - the checkerboard the menus sit on, S2D copy.
 *
 *   S2D @ 0x800834C8
 *
 * A 10x8 character map of four cells laid out two by two, so it tiles as a
 * checkerboard. Only the index array is built here; the cells themselves come
 * from the map's own definition.
 *
 * The index array sits 0x20000 above the DNG and ADV copies; see
 * src/p1-jp/common/gfx/checkermap.c for the shared original.
 */
#include <types.h>

#define g_checker_index ((short *)0x80111D80)

#define CHECKER_W 10
#define CHECKER_H 8

void CheckerMapInit(void)
{
    int i;

    for (i = 0; i < CHECKER_W * CHECKER_H; i++) {
        g_checker_index[i] = ((i / CHECKER_W) & 1) * 2 + (i & 1) + 1;
    }
}
