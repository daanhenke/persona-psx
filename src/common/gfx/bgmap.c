/* Persona 1 (JP) - blanking one row of the tiled background map.
 *   DNG 0x800761FC   ADV 0x80066778   S2D 0x80066210
 *
 * One GsMAP of 16x16-pixel cells, 15 across and 4 down. g_bg_index says which
 * cell goes where and g_bg_cells holds the cell definitions; libgs walks both
 * when it draws the map. This half reaches its index through the linker
 * symbol, so it is the same code everywhere. Setting a cell is a unit of its
 * own much further along, in bgmapcell.c.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

extern u_short g_bg_index[];


/* Blanks one row. The row stride is the map's own ncellw, so this and the
   `15` in BgMapInit have to stay in step. */
void BgMapClearRow(u_short row)
{
    int i;

    for (i = 0; i < 15; i++) {
        g_bg_index[row * 15 + i] = 0;
    }
}
