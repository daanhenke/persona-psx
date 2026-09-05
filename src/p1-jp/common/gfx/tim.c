/* Persona 1 (JP) - getting TIMs and packed maps into VRAM.
 *
 *              DNG         NAME
 *   TimLoad    0x8006FB08  0x80067928
 *   TimLoadAt  0x8006FBAC  0x800679CC
 *   BgFromPack 0x8006FD84  0x80067AD8
 *
 * A TIM is {id, flag, then one or two blocks of {size, x, y, w, h, data}}.
 * TimLoad goes through GsGetTimInfo and uploads both blocks; TimLoadAt reads
 * the pixel block's header itself so the caller can put it somewhere other
 * than where the file says.
 */
#include <types.h>
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

/* Builds a GsMAP and the GsBG that draws it out of a packed map file: four
   bytes of dimensions, the cell definitions eight bytes into the pack, and the
   cell index straight after the header. */
void BgFromPack(u_long *pack, u_char *hdr, GsMAP *map, GsBG *bg,
                short x, short y)
{
    hdr += 4;
    map->ncellw = *hdr++;
    map->ncellh = *hdr++;
    map->cellw = *hdr++;
    map->cellh = *hdr++;
    map->base = (GsCELL *)(pack + 2);
    map->index = (u_short *)hdr;

    bg->attribute = 0x1000000;
    bg->b = 0x80;
    bg->g = 0x80;
    bg->r = 0x80;
    bg->scrolly = 0;
    bg->scrollx = 0;
    bg->map = map;
    bg->scaley = 0x1000;
    bg->scalex = 0x1000;
    bg->rotate = 0;
    bg->w = map->cellw * map->ncellw;
    bg->h = map->cellh * map->ncellh;
    bg->mx = bg->w / 2;
    bg->my = bg->h / 2;
    bg->x = x + bg->mx;
    bg->y = y + bg->my;
}
