/* Persona 1 (JP) - queueing a TIM for upload.  DNG only.
 *   0x80077BFC func_80077BFC  0x80077C04 func_80077C04
 *   0x80077C0C func_80077C0C  0x80077C14 TimQueue
 *   0x80077CA4 TimQueueAt
 *
 * A TIM out of one of the field's archives is put on the deferred upload
 * queue (imagequeue.c): its pixels, and its palette when it has one, as two
 * entries. TimQueue sends them where the TIM itself says; TimQueueAt to a
 * place of the caller's. ADV's copy (src/adv/gfx/timqueue.c) writes the
 * queueing out again; this one calls QueueImageUpload.
 *
 * The three empty routines ahead of them have no callers left.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

extern void QueueImageUpload(RECT *rect, u_long *data);

/* The TIM carries a palette. */
#define TIM_HAS_CLUT(img) (((img).pmode >> 3) & 1)

void func_80077BFC(void)
{
}

void func_80077C04(void)
{
}

void func_80077C0C(void)
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
