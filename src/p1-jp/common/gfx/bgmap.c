/* Persona 1 (JP) - the tiled background map.
 *
 * Compiled into three overlays rather than called across the boundary:
 *              DNG         ADV         S2D
 *   Init       0x80076138  0x800666B4  0x8006614C
 *   ClearRow   0x800761FC  0x80066778  0x80066210
 *   SetCell    0x80077064  0x8006789C  0x8006708C
 * DNG and ADV share the work-area half of it; S2D's sits 0x20000 higher and
 * keeps src/p1-jp/s2d/bgmap.c.
 *
 * One GsMAP of 16x16-pixel cells, 15 across and 4 down. g_bg_index says which
 * cell goes where and g_bg_cells holds the cell definitions; libgs walks both
 * when it draws the map.
 */
#include <types.h>
#include <libgs.h>

extern GsMAP   g_bg_map;
extern u_short g_bg_index[];

/* The cell definitions the map points at, and the background tick whose low
   bits pick a palette. Both are work-area addresses. */
#define g_bg_cells ((GsCELL *)0x800E224C)
extern u_int   g_bg_state[];      /* [0] is the animation tick */

/* Blanks one row. The row stride is the map's own ncellw, so this and the
   `15` in BgMapInit have to stay in step. */
void BgMapClearRow(u_short row)
{
    int i;

    for (i = 0; i < 15; i++) {
        g_bg_index[row * 15 + i] = 0;
    }
}

/* Points one map cell at its own tile and fills in that tile's GsCELL.
 *
 * The tile number splits into nibbles to address a 16x16 atlas: the low
 * nibble scaled by 16 gives u, the high nibble is already v. The CLUT keeps
 * its top bits and takes a palette from the background tick, which is what
 * animates the water and fire tiles. */
void BgMapSetCell(u_short idx)
{
    GsCELL  *cell;
    u_short  tile;

    tile = idx + 1;
    g_bg_index[idx] = tile;
    cell = g_bg_cells + 1;
    cell[idx].u = (tile & 0xF) << 4;
    cell[idx].v = tile & 0xF0;
    cell[idx].cba = (cell[idx].cba & 0xFFC0) + 0x3C + ((g_bg_state[0] >> 4) & 7);
}
