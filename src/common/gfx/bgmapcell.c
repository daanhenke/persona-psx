/* Persona 1 (JP) - pointing one background map cell at its tile.
 *   DNG 0x80077064   ADV 0x8006789C   S2D 0x8006708C
 *
 * Reaches the cell definitions and the tick by hardcoded address, and S2D's
 * work area sits 0x20000 higher - which is what WORK_BIAS says. Blanking a row
 * is a unit of its own well ahead of this, in bgmap.c.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

extern GsMAP   g_bg_map;
extern u_short g_bg_index[];

/* The cell definitions the map points at, and the background tick whose low
   bits pick a palette. Both are work-area addresses. */
#define g_bg_cells ((GsCELL *)(0x800E224C + WORK_BIAS))

/* [0] is the animation tick, reached by address like the cells beside it, so
   S2D's copy comes out 0x20000 higher on the same WORK_BIAS. */
#define g_bg_state ((u_int *)(0x800E1E4C + WORK_BIAS))

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
