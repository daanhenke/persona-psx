/* Persona 1 (JP) - a whole image descriptor uploaded in one go.
 *   NAME @ 0x800679CC   DNG @ 0x8006FBAC
 *
 * The row-at-a-time form both NAME and DNG carry is a unit of its own, in
 * image.c, and in both it follows this one. DNG's field hands it a TIM out of
 * the floor's archive, calling it unprototyped with int coordinates.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>

/* The same descriptor, uploaded once rather than a row at a time. The caller's
   pointer is what walks it, which is why there is no separate cursor. */
void UploadImageOne(int *p, u_short x, u_short y)
{
    RECT rect;
    int  packed;

    p += 3;
    rect.x = x;
    rect.y = y;
    packed = *p++;
    if ((short)x < 0) {
        rect.x = (short)packed;
    }
    if ((short)y < 0) {
        rect.y = (short)((u_int)packed >> 16);
    }
    packed = *p++;
    rect.w = (short)packed;
    rect.h = (short)((u_int)packed >> 16);
    LoadImage(&rect, (u_long *)p);
}
