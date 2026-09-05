/* Persona 1 (JP) - BTLP overlay @ 0x80066760
 *
 * VRAM writes cannot happen while the GPU is drawing, so the battle code does
 * not call ClearImage/LoadImage where it wants them: it appends a rectangle to
 * one of two queues and the queues are drained here, once per frame, from
 * BtlDrawFrame while the display is between fields.
 *
 * The clear queue is walked backwards and its count decremented in place, so a
 * rectangle appended while the queue is draining still gets serviced. The
 * upload queue is walked forwards and cleared in one go at the end.
 */
#include <types.h>
#include <libgpu.h>

typedef struct {
    /* 0x0 */ RECT   rect;
    /* 0x8 */ u_char r, g, b;
    /* 0xB */ u_char pad;
} VramClear;                        /* 0xC bytes */

typedef struct {
    /* 0x0 */ RECT    rect;
    /* 0x8 */ u_long *data;
} VramLoad;                         /* 0xC bytes */

extern int       g_btl_vram_load_count;
extern int       g_btl_vram_clear_count;
extern VramLoad  g_btl_vram_loads[];
extern VramClear g_btl_vram_clears[];
extern DRAWENV   g_btl_drawenv;

void BtlFlushVramQueues(void)
{
    VramLoad *q;
    int n;
    int i;

    GetDrawEnv(&g_btl_drawenv);

    while (g_btl_vram_clear_count > 0) {
        n = g_btl_vram_clear_count - 1;
        g_btl_vram_clear_count = n;
        ClearImage(&g_btl_vram_clears[n].rect, g_btl_vram_clears[n].r,
                   g_btl_vram_clears[n].g, g_btl_vram_clears[n].b);
    }

    i = 0;
    if (g_btl_vram_load_count != 0) {
        q = g_btl_vram_loads;
        do {
            LoadImage(&q->rect, g_btl_vram_loads[i].data);
            q++;
            i++;
        } while (g_btl_vram_load_count != i);
    }
    g_btl_vram_load_count = 0;
}
