/* Persona 1 (JP) - queueing a TIM for upload.  ADV only.
 *   0x8006849C func_8006849C  0x800684A4 func_800684A4
 *   0x800684AC func_800684AC  0x800684B4 TimQueue
 *   0x80068604 TimQueueAt
 *
 * A TIM out of one of ADV's archives is put on the deferred upload queue
 * (imagequeue.c): its pixels, and its palette when it has one, as two
 * entries. TimQueue sends them where the TIM itself says; TimQueueAt to a
 * place of the caller's. The queueing is QueueImageUpload written out again.
 *
 * The three empty routines ahead of them have no callers left.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

typedef struct {
    /* 0x00 */ u_long *data;
    /* 0x04 */ RECT    rect;
} ImageUpload;                  /* 0x0C bytes */

extern int         g_image_queue_count;
extern ImageUpload g_image_queue[];

/* The TIM carries a palette. */
#define TIM_HAS_CLUT(img) (((img).pmode >> 3) & 1)

static inline void TimQueueUpload(RECT *rect, u_long *data)
{
    int n;

    n = g_image_queue_count;
    g_image_queue[n].data = data;
    g_image_queue[n].rect.x = rect->x;
    g_image_queue[n].rect.y = rect->y;
    g_image_queue[n].rect.w = rect->w;
    g_image_queue_count = n + 1;
    g_image_queue[n].rect.h = rect->h;
}

void func_8006849C(void)
{
}

void func_800684A4(void)
{
}

void func_800684AC(void)
{
}

void TimQueue(u_long *tim)
{
    RECT    rect;
    GsIMAGE img;

    GsGetTimInfo(tim + 1, &img);
    rect.x = img.px;
    rect.y = img.py;
    rect.w = img.pw;
    rect.h = img.ph;
    TimQueueUpload(&rect, img.pixel);
    if (TIM_HAS_CLUT(img)) {
        rect.x = img.cx;
        rect.y = img.cy;
        rect.w = img.cw;
        rect.h = img.ch;
        TimQueueUpload(&rect, img.clut);
    }
}

void TimQueueAt(u_long *tim, short x, short y, short cx, short cy)
{
    RECT    rect;
    GsIMAGE img;

    GsGetTimInfo(tim + 1, &img);
    rect.x = x;
    rect.y = y;
    rect.w = img.pw;
    rect.h = img.ph;
    TimQueueUpload(&rect, img.pixel);
    if (TIM_HAS_CLUT(img)) {
        rect.x = cx;
        rect.y = cy;
        rect.w = img.cw;
        rect.h = img.ch;
        TimQueueUpload(&rect, img.clut);
    }
}
