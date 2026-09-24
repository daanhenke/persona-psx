/* Persona 1 (JP) - putting the tiled background map back to nothing.  ADV only.
 *   ADV 0x800667B8
 *
 * Wipes the map's texture page in VRAM, empties the 64-word table beside the
 * map, points every cell at the blank corner of the page, queues the map's
 * palette row for upload and blanks the four rows of the map index - the
 * last written out a row at a time rather than through BgMapClearRow.
 */
#include <decomp/types.h>
#include <persona/common/bg.h>
#include <persona/common/imageanim.h>
#include <persona/common/vram.h>

/* The texture page the cells are cut from, and the palette row under it. */
#define PAGE_X   0x3C0
#define PAGE_Y   0x100
#define PAGE_W   0x40
#define PAGE_H   0x100
#define CLUT_Y   0x1FF

/* Every cell the map can point at, plus the blank one ahead of them. */
#define BG_CELLS (BG_MAP_W * BG_MAP_H + 5)

#define CELL_CBA   0x7FFC
#define CELL_TPAGE 0x1F

/* Beside the cells in the work area, reached by hardcoded address. */
#define BG_SLOTS 64
#define g_bg_slots ((u_int *)(0x800EB14C + WORK_BIAS))

/* The palette row the map's cells draw with. */
extern u_long g_bg_clut[];

static inline void ClearRow(u_short row)
{
    int i;

    for (i = 0; i < BG_MAP_W; i++) {
        g_bg_index[row * BG_MAP_W + i] = 0;
    }
}

void BgReset(void)
{
    u_int  *slots;
    GsCELL *cell;
    u_char  n;
    int     q;
    /* Eight bytes of frame nothing reads; neither the inline's parameter nor
       any local's type accounts for them. */
    int     unused[2];

    slots = g_bg_slots;
    cell = g_bg_cells;
    VramClearRect(PAGE_X, PAGE_Y, PAGE_W, PAGE_H);
    for (n = 0; n < BG_SLOTS; n++) {
        slots[n] = 0;
    }
    for (n = 0; n < BG_CELLS; n++) {
        cell->u = 0;
        cell->v = 0;
        cell->cba = CELL_CBA;
        cell->flag = 0;
        cell->tpage = CELL_TPAGE;
        cell++;
    }

    q = g_image_queue_count;
    g_image_queue[q].data = g_bg_clut;
    g_image_queue[q].rect.x = PAGE_X;
    g_image_queue[q].rect.y = CLUT_Y;
    g_image_queue[q].rect.w = PAGE_W;
    g_image_queue[q].rect.h = 1;
    g_image_queue_count = q + 1;

    ClearRow(0);
    ClearRow(1);
    ClearRow(2);
    ClearRow(3);
}
