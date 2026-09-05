/* Persona 1 (JP) - the tiled background map.
 *
 * Compiled into three overlays rather than called across the boundary:
 *              DNG         ADV         S2D
 *   Init       0x80076138  0x800666B4  0x8006614C
 *   ClearRow   0x800761FC  0x80066778  0x80066210
 *   SetCell    0x80077064  0x8006789C  0x8006708C
 * ClearRow reaches its index through the linker symbol, so all three overlays
 * use the copy here. SetCell reaches the cell definitions and the tick by
 * hardcoded address, and S2D's work area sits 0x20000 higher, so that one has
 * its own copy in src/p1-jp/s2d/gfx/bgmap.c.
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
    u_int state;
    u_int row;
    u_int col;
    u_short cba;

    g_bg_index[idx] = idx + 1;
    /* Every one of these four locals is load-bearing: the tick, the old CLUT,
       the half of it that survives, and even the constant column. Folding any
       of them back into the expression that uses it costs the match. */
    state = g_bg_state[0];
    /* `* 16` and `<< 4` are not interchangeable here: the shift form schedules
       differently and costs the match. */
    g_bg_cells[idx + 1].u = ((idx + 1) & 0xF) * 16;
    cba = g_bg_cells[idx + 1].cba;
    g_bg_cells[idx + 1].v = (idx + 1) & 0xF0;
    row = cba & 0xFFC0;
    col = 0x3C;
    g_bg_cells[idx + 1].cba = row + col + ((state >> 4) & 7);
}
