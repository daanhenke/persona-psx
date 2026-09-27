/* Persona 1 (JP) - queueing a TIM for upload.  S2D only.
 *   0x80067C24 func_80067C24  0x80067C2C func_80067C2C
 *   0x80067C34 func_80067C34  0x80067C3C TimQueue
 *   0x80067CCC TimQueueAt
 *
 * DNG's unit (src/dng/gfx/timqueue.c), empty routines and all. A TIM goes on
 * the deferred upload queue (imagequeue.c) as its pixels and, when it has
 * one, its palette; TimQueue sends them where the TIM says, TimQueueAt to
 * the caller's place. The three empty routines have no callers.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

extern void QueueImageUpload(RECT *rect, u_long *data);

/* The TIM carries a palette. */
#define TIM_HAS_CLUT(img) (((img).pmode >> 3) & 1)

void func_80067C24(void)
{
}

void func_80067C2C(void)
{
}

void func_80067C34(void)
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
    QueueImageUpload(&rect, img.pixel);
    if (TIM_HAS_CLUT(img)) {
        rect.x = img.cx;
        rect.y = img.cy;
        rect.w = img.cw;
        rect.h = img.ch;
        QueueImageUpload(&rect, img.clut);
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
    QueueImageUpload(&rect, img.pixel);
    if (TIM_HAS_CLUT(img)) {
        rect.x = cx;
        rect.y = cy;
        rect.w = img.cw;
        rect.h = img.ch;
        QueueImageUpload(&rect, img.clut);
    }
}
