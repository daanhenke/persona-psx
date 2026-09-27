/* Persona 1 (JP) - VRAM and memory helpers.  S2D only.
 *   0x80097370 TimLoad    0x80097400 VramStore  0x80097438 VramLoad
 *   0x80097470 VramMove   0x800974B0 IntPow     0x800974E0 CopyWords
 *
 * TimLoad is TimQueue (timqueue.c) sending straight to VRAM rather than
 * through the upload queue. The Vram* wrappers take a rectangle as four
 * numbers so callers need not build a RECT.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <decomp/include_asm.h>

/* The TIM carries a palette. */
#define TIM_HAS_CLUT(img) (((img).pmode >> 3) & 1)

void TimLoad(u_long *tim)
{
    RECT    rect;
    GsIMAGE img;

    GsGetTimInfo(tim + 1, &img);
    rect.x = img.px;
    rect.y = img.py;
    rect.w = img.pw;
    rect.h = img.ph;
    LoadImage(&rect, img.pixel);
    if (TIM_HAS_CLUT(img)) {
        rect.x = img.cx;
        rect.y = img.cy;
        rect.w = img.cw;
        rect.h = img.ch;
        LoadImage(&rect, img.clut);
    }
}

#ifdef NON_MATCHING
void VramStore(x, y, w, h, p)
short   x, y, w, h;
u_long *p;
{
    RECT rect;

    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    StoreImage(&rect, p);
}
#else
/* The image stores ra after the argument load and before the RECT; cc1
   schedules it two slots later (see VramClearRect in render.c). */
INCLUDE_ASM("s2d/nonmatchings/gfx/vram", VramStore);
#endif

#ifdef NON_MATCHING
void VramLoad(x, y, w, h, p)
short   x, y, w, h;
u_long *p;
{
    RECT rect;

    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    LoadImage(&rect, p);
}
#else
/* The image stores ra after the argument load and before the RECT; cc1
   schedules it two slots later (see VramClearRect in render.c). */
INCLUDE_ASM("s2d/nonmatchings/gfx/vram", VramLoad);
#endif

#ifdef NON_MATCHING
void VramMove(x, y, w, h, dx, dy)
short x, y, w, h;
int   dx, dy;
{
    RECT rect;

    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    MoveImage(&rect, dx, dy);
}
#else
/* The image stores ra after the argument load and before the RECT; cc1
   schedules it two slots later (see VramClearRect in render.c). */
INCLUDE_ASM("s2d/nonmatchings/gfx/vram", VramMove);
#endif

/* x to the power n, for n of 1 and up. */
#ifdef NON_MATCHING
int IntPow(int x, int n)
{
    int r;

    r = x;
    while (n >= 2) {
        r *= x;
        n--;
    }
    return r;
}
#else
/* The image's back-branch lands past the decrement, so its loop never
   changes n; this cc1 branches to the decrement. */
INCLUDE_ASM("s2d/nonmatchings/gfx/vram", IntPow);
#endif

void CopyWords(u_long *dst, u_long *src, int n)
{
    while (n != 0) {
        *dst++ = *src++;
        n--;
    }
}
