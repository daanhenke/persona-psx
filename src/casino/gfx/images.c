/* Persona 1 (JP) - CASINO's VRAM uploads.
 *   0x8006A2A8 CasinoUploadTim
 *   0x8006A344 CasinoQueueImage
 *   0x8006A3CC CasinoFlushImages
 *
 * Nothing goes to VRAM straight away: a TIM's pixels and palette are queued,
 * and the frame hands the queue to LoadImage once the GPU has finished
 * drawing.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/casino/casino.h>

/* The image last handed to LoadImage. */
extern u_long *D_800A8338;

void CasinoQueueImage(u_long *data, RECT *r);

void CasinoUploadTim(u_long *tim, int px, int py, int cx, u_short cy)
{
    RECT    r;
    GsIMAGE img;

    GsGetTimInfo(tim + 1, &img);
    r.x = px;
    r.y = py;
    r.w = img.pw;
    r.h = img.ph;
    CasinoQueueImage(img.pixel, &r);
    r.x = cx;
    r.y = cy;
    r.w = img.cw;
    r.h = img.ch;
    CasinoQueueImage(img.clut, &r);
}

void CasinoQueueImage(u_long *data, RECT *r)
{
    int n;

    n = g_casino_load_queue.n;
    g_casino_load_queue.rect[n].x = r->x;
    g_casino_load_queue.rect[n].y = r->y;
    g_casino_load_queue.rect[n].w = r->w;
    g_casino_load_queue.rect[n].h = r->h;
    g_casino_load_queue.data[n] = data;
    g_casino_load_queue.n++;
}

void CasinoFlushImages(void)
{
    RECT r;

    while (g_casino_load_queue.n) {
        r = g_casino_load_queue.rect[--g_casino_load_queue.n];
        D_800A8338 = g_casino_load_queue.data[g_casino_load_queue.n];
        LoadImage(&r, D_800A8338);
    }
}
