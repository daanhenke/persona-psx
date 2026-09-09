/* Persona 1 (JP) - a TIM into VRAM.
 *
 *              DNG         NAME
 *   TimLoad    0x8006FB08  0x80067928
 *
 * A TIM is {id, flag, then one or two blocks of {size, x, y, w, h, data}}.
 * TimLoad goes through GsGetTimInfo and uploads both blocks. The variant that
 * reads the pixel block's header itself, so the caller can put it somewhere
 * other than where the file says, is a unit of its own; so is the packed-map
 * reader that follows it in the image, in bgpack.c.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

extern void GsGetTimInfo(u_long *tim, GsIMAGE *img);

/* `nopal` suppresses the CLUT even when the TIM carries one. */
void TimLoad(u_long *tim, int nopal)
{
    RECT    r;
    GsIMAGE img;

    GsGetTimInfo(tim + 1, &img);
    r.x = img.px;
    r.y = img.py;
    r.w = img.pw;
    r.h = img.ph;
    LoadImage(&r, img.pixel);
    if (nopal == 0 && ((img.pmode >> 3) & 1)) {
        r.x = img.cx;
        r.y = img.cy;
        r.w = img.cw;
        r.h = img.ch;
        LoadImage(&r, img.clut);
    }
}
