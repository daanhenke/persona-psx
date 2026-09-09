/* Persona 1 (JP) - a packed map file into a background.
 *
 *              DNG         NAME
 *   BgFromPack 0x8006FD84  0x80067AD8
 *
 * A unit of its own: in NAME the two upload helpers of image.c sit between
 * this and TimLoad, which shares its source.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

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
