/* Persona 1 (JP) - a number drawn as sprite cells.  ADV @ 0x8008F084.
 *
 * The digit form of CellsWriteRow, walking backwards so the least significant
 * digit lands in a fixed column - the same reason TileMapWriteRowRev exists.
 * The row of the atlas is fixed and the bias puts glyph 14 at u = 0.
 */
#include <types.h>
#include <libgs.h>

#define GLYPH_W     8
#define DIGIT_V     0x40
#define DIGIT_FIRST 14
#define CELL_FLAG   0x3C0
#define CELL_TPAGE  0x1A0

void CellsWriteDigitsRev(GsCELL *dst, const u_char *src, u_short count)
{
    int i;

    for (i = 0; i < count; i++) {
        dst->u = *src * GLYPH_W - DIGIT_FIRST * GLYPH_W;
        dst->v = DIGIT_V;
        dst->flag = CELL_FLAG;
        dst->tpage = CELL_TPAGE;
        src++;
        dst--;
    }
}
