/* Persona 1 (JP) - one cell of the tiled background, S2D's copy.
 *
 *   S2D @ 0x8006708C
 *
 * The same routine as the one in src/p1-jp/common/gfx/bgmap.c, which covers
 * DNG and ADV. Only this one differs between the overlays: it reaches the cell
 * definitions and the animation tick by hardcoded address, and S2D's work area
 * sits 0x20000 higher. BgMapClearRow goes through the linker symbol for its
 * index and so the common copy serves all three.
 */
#include <types.h>
#include <libgs.h>

extern u_short g_bg_index[];

#define g_bg_cells ((GsCELL *)0x8010224C)
extern u_int g_bg_state_s2d[];      /* [0] is the animation tick */

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
    state = g_bg_state_s2d[0];
    /* `* 16` and `<< 4` are not interchangeable here: the shift form schedules
       differently and costs the match. */
    g_bg_cells[idx + 1].u = ((idx + 1) & 0xF) * 16;
    cba = g_bg_cells[idx + 1].cba;
    g_bg_cells[idx + 1].v = (idx + 1) & 0xF0;
    row = cba & 0xFFC0;
    col = 0x3C;
    g_bg_cells[idx + 1].cba = row + col + ((state >> 4) & 7);
}
