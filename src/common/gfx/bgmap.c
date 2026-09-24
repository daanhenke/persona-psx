/* Persona 1 (JP) - setting up the tiled background map, and blanking a row.
 *              DNG         ADV         S2D
 *   Init       0x80076138  0x800666B4  0x8006614C
 *   ClearRow   0x800761FC  0x80066778  0x80066210
 *
 * One GsMAP of 16x16-pixel cells, 15 across and 4 down. g_bg_index says which
 * cell goes where and g_bg_cells holds the cell definitions; libgs walks both
 * when it draws the map. This half reaches its index through the linker
 * symbol, so it is the same code everywhere. Setting a cell is a unit of its
 * own much further along, in bgmapcell.c.
 */
#include <decomp/types.h>
#include <persona/common/bg.h>

/* The layer the map is drawn through. */
#define MAP_LAYER 4

/* Points the map at its cells and index, keeps what it is to draw from,
   squares the panel layer's scroll and every counter of the state away, and
   blanks the map's four rows. */
void BgMapInit(void *src, short arg)
{
    g_msg->script = src;
    g_bg_map.cellw = g_bg_map.cellh = BG_MAP_CELL;
    g_bg_map.ncellw = BG_MAP_W;
    g_bg_map.ncellh = BG_MAP_H;
    g_bg_map.base = g_bg_cells;
    g_bg_map.index = g_bg_index;
    g_bg_layers[MAP_LAYER].scrollx = 0;
    g_bg_layers[MAP_LAYER].scrolly = 0;
    g_msg->cursor = 0;
    g_msg->flags = 0;
    g_msg->wait = 0;
    g_msg->delay = 0;
    g_msg->speed = arg;
    BgMapClearRow(0);
    BgMapClearRow(1);
    BgMapClearRow(2);
    BgMapClearRow(3);
}

/* Blanks one row. The row stride is the map's own ncellw, so this and the
   `15` in BgMapInit have to stay in step. */
void BgMapClearRow(u_short row)
{
    int i;

    for (i = 0; i < BG_MAP_W; i++) {
        g_bg_index[row * BG_MAP_W + i] = 0;
    }
}
