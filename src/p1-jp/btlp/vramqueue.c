/* Persona 1 (JP) - BTLP overlay @ 0x80066760, 0x800665F4
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

/* The upload ring wraps here, though nothing ever fills it. */
#define BTL_VRAM_LOADS 0x48

/* The clear ring is shallower; the contact box is the only thing that uses it
   more than once a frame. */
#define BTL_VRAM_CLEARS 0x20

/* Appends one rectangle and the colour to fill it with. The red goes in before
   the rectangle and the other two after it, and the count is read three times
   over rather than once - the second and third reads are what put the index
   arithmetic where the original has it, so the repetition is load-bearing. */
void BtlQueueVramClear(short x, short y, short w, short h,
                       u_char r, u_char g, u_char b)
{
    int i;
    int j;

    i = g_btl_vram_clear_count;
    g_btl_vram_clears[i].r = r;
    j = g_btl_vram_clear_count;
    g_btl_vram_clears[i].rect.x = x;
    g_btl_vram_clears[i].rect.y = y;
    g_btl_vram_clears[i].rect.w = w;
    g_btl_vram_clears[i].rect.h = h;
    g_btl_vram_clears[j].g = g;
    g_btl_vram_clears[g_btl_vram_clear_count].b = b;
    g_btl_vram_clear_count = (g_btl_vram_clear_count + 1) % BTL_VRAM_CLEARS;
}

/* Appends one rectangle. The source pointer is stored between the width and
   the height, not after them. */
void BtlQueueVramLoad(u_long *data, short x, short y, short w, short h)
{
    int i;

    i = g_btl_vram_load_count;
    g_btl_vram_loads[i].rect.x = x;
    g_btl_vram_loads[i].rect.y = y;
    g_btl_vram_loads[i].rect.w = w;
    g_btl_vram_loads[i].data = data;
    g_btl_vram_loads[i].rect.h = h;
    g_btl_vram_load_count = (g_btl_vram_load_count + 1) % BTL_VRAM_LOADS;
}

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
