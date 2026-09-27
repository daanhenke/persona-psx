/* Persona 1 (JP) - numbers drawn as sprite cells.
 *
 * Compiled into two overlays rather than called across the boundary:
 *                          ADV         DNG
 *   CellsWriteNumberRev    0x8008EF9C  0x8009303C
 *   CellsWriteGlyphsRev    0x8008F000  0x800930A0
 *   CellsWriteDigitsRev    0x8008F084  0x80093124
 *
 * The digit forms of CellsWriteRow, walking backwards so the least
 * significant digit lands in a fixed column - the same reason
 * TileMapWriteRowRev exists. Each takes its glyphs from a fixed row of the
 * atlas, biased so digit 0 lands on the row's first numeral.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

#define GLYPH_W     8
#define CELL_FLAG   0x3C0
#define CELL_TPAGE  0x1A0

/* The large numerals: the row at v 0x48, digit 0 six glyphs in. */
#define NUMBER_V     0x48
#define NUMBER_FIRST 6

/* Digits from the same row, anything from 10 up from the row at v 0x3C,
   one glyph in. */
#define LETTER_V     0x3C
#define LETTER_FIRST 1

/* The small numerals: the row at v 0x40, glyph 14 at u 0. */
#define DIGIT_V     0x40
#define DIGIT_FIRST 14

void CellsWriteNumberRev(GsCELL *dst, const u_char *src, u_short count)
{
    int i;

    for (i = 0; i < count; i++) {
        dst->u = *src * GLYPH_W + NUMBER_FIRST * GLYPH_W;
        dst->v = NUMBER_V;
        dst->flag = CELL_FLAG;
        dst->tpage = CELL_TPAGE;
        src++;
        dst--;
    }
}

void CellsWriteGlyphsRev(GsCELL *dst, const u_char *src, u_short count)
{
    int i;

    for (i = 0; i < count; i++) {
        if (*src < 10) {
            dst->u = *src * GLYPH_W + NUMBER_FIRST * GLYPH_W;
            dst->v = NUMBER_V;
        } else {
            dst->u = *src * GLYPH_W + LETTER_FIRST * GLYPH_W;
            dst->v = LETTER_V;
        }
        dst->flag = CELL_FLAG;
        dst->tpage = CELL_TPAGE;
        src++;
        dst--;
    }
}

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
