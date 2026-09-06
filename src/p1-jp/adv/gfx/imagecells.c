/* Persona 1 (JP) - cells for a full-screen picture.  ADV @ 0x80083568.
 *
 * An event picture is drawn as a character map of 16x16 cells rather than one
 * sprite, so it can be paged into VRAM a strip at a time. This lays the cell
 * run out for the width the picture happens to be: sixteen rows of 24, 32 or
 * 48 cells, which is 384, 512 or 768 pixels across.
 *
 * The texture page steps every sixteenth cell because a page is 256 pixels
 * wide, so a picture wider than that spans several of them.
 */
#include <types.h>
#include <libgs.h>

/* Reached by hardcoded address; filled just before the picture is loaded. */
#define g_adv_image_cells ((GsCELL *)0x800E964C)

#define CELL      16
#define IMG_CLUT  0x7800
#define IMG_ROWS  16

void ImageCellsInit(short kind)
{
    GsCELL *c;
    short   i;

    switch (kind) {
    case 0:
    case 1:
        for (i = 0; i < 24 * IMG_ROWS; i++) {
            c = &g_adv_image_cells[i];
            c->u = (i % 24 & 0xF) * CELL;
            c->v = (i / 24) * CELL;
            c->cba = IMG_CLUT;
            c->flag = 0;
            c->tpage = (i % 24) / CELL * 2 + 6;
        }
        break;
    case 2:
        for (i = 0; i < 32 * IMG_ROWS; i++) {
            c = &g_adv_image_cells[i];
            c->u = (i & 0xF) * CELL;
            c->v = (i / 32) * CELL;
            c->cba = IMG_CLUT;
            c->flag = 0;
            c->tpage = (i & 0x1F) / CELL * 2 + 6;
        }
        break;
    case 3:
    case 4:
        for (i = 0; i < 48 * IMG_ROWS; i++) {
            c = &g_adv_image_cells[i];
            c->u = (i & 0xF) * CELL;
            c->v = (i / 48) * CELL;
            c->cba = IMG_CLUT;
            c->flag = 0;
            c->tpage = (i % 48) / CELL * 2 + 6;
        }
        break;
    }
}
