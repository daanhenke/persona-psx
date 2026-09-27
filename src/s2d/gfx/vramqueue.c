/* Persona 1 (JP) - S2D's deferred VRAM writes.
 *   0x80098868 VramQueueLoad  0x800988E8 VramQueueClear
 *   0x800989C0 VramFlushQueues
 *
 * BTLP's two queues (src/btlp/vramqueue.c) in S2D's copy: uploads and fills
 * are appended to rings and drained once a frame, both from the back. The
 * upload ring is 64 deep here and stores the source pointer after the
 * rectangle.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>

typedef struct {
    /* 0x0 */ RECT   rect;
    /* 0x8 */ u_char r, g, b;
    /* 0xB */ u_char pad;
} VramClear;

typedef struct {
    /* 0x0 */ RECT    rect;
    /* 0x8 */ u_long *data;
} VramLoad;

#define VRAM_LOADS  64
#define VRAM_CLEARS 32

extern int       g_vram_load_count;
extern int       g_vram_clear_count;
extern VramLoad  g_vram_loads[VRAM_LOADS];
extern VramClear g_vram_clears[VRAM_CLEARS];

void VramQueueLoad(u_long *data, short x, short y, short w, short h)
{
    int i;

    i = g_vram_load_count;
    g_vram_loads[i].rect.x = x;
    g_vram_loads[i].rect.y = y;
    g_vram_loads[i].rect.w = w;
    g_vram_loads[i].rect.h = h;
    g_vram_loads[i].data = data;
    g_vram_load_count = (i + 1) % VRAM_LOADS;
}

/* As BTLP's: the count is read three times over. */
void VramQueueClear(short x, short y, short w, short h,
                    u_char r, u_char g, u_char b)
{
    int i;
    int j;

    i = g_vram_clear_count;
    g_vram_clears[i].r = r;
    j = g_vram_clear_count;
    g_vram_clears[i].rect.x = x;
    g_vram_clears[i].rect.y = y;
    g_vram_clears[i].rect.w = w;
    g_vram_clears[i].rect.h = h;
    g_vram_clears[j].g = g;
    g_vram_clears[g_vram_clear_count].b = b;
    g_vram_clear_count = (g_vram_clear_count + 1) % VRAM_CLEARS;
}

void VramFlushQueues(void)
{
    int n;

    while (g_vram_clear_count != 0) {
        n = g_vram_clear_count - 1;
        g_vram_clear_count = n;
        ClearImage(&g_vram_clears[n].rect, g_vram_clears[n].r,
                   g_vram_clears[n].g, g_vram_clears[n].b);
    }
    while (g_vram_load_count != 0) {
        n = g_vram_load_count - 1;
        g_vram_load_count = n;
        LoadImage(&g_vram_loads[n].rect, g_vram_loads[n].data);
    }
}
