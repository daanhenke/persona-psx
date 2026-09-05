/* Persona 1 (JP) - text drawn as sprite cells rather than into a tile layer.
 *
 *   CellsWriteRow  DNG 0x80093188  ADV 0x8008F0E8  S2D 0x8008369C
 *   CellsClear     DNG 0x8009300C  ADV 0x8008EF6C  S2D 0x80083520
 *
 * The font atlas is 31 glyphs across at 8 by 12 pixels, so a character code
 * splits into a column and a row of it. The tile-layer path in tilemap.c does
 * the same job for backgrounds; this one produces GsCELLs.
 */
#include <types.h>
#include <libgs.h>

#define GLYPH_W    8
#define GLYPH_H    12
#define ATLAS_COLS 31
#define CELL_FLAG  0x3C0
#define CELL_TPAGE 0x1A0

/* 0xFF ends the string early, the same terminator TileMapWriteRow uses. */
void CellsWriteRow(GsCELL *dst, const u_char *src, u_char page, u_short count)
{
    while (count != 0) {
        if (*src == 0xFF) {
            return;
        }
        dst->u = (*src % ATLAS_COLS) * GLYPH_W;
        dst->v = (*src / ATLAS_COLS) * GLYPH_H;
        dst->flag = CELL_FLAG;
        dst->tpage = CELL_TPAGE + page;
        count--;
        src++;
        dst++;
    }
}

/* Blanks a run of cells. 0xFF in u and v is the same "no cell here" code the
   row writers stop on, so a bar or a string that shrinks erases its tail. */
void CellsClear(GsCELL *dst, u_char count)
{
    while (count != 0) {
        dst->u = 0xFF;
        dst->v = 0xFF;
        dst++;
        count--;
    }
}
