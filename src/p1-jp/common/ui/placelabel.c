/* Persona 1 (JP) - the two-line label naming where the player is.
 *
 *   DNG 0x80084398   ADV 0x800757B4   S2D 0x8007476C
 *
 * Eleven cells a row, one row above the other, from a bank the glyph base
 * 0x1AE selects. The labels read "3D ADV" over "2D" and a Japanese line over
 * "IN BATTLE"; see tools/glyphs.py for how the packed bytes decode.
 */
#include <types.h>

#define LABEL_ROW0  ((short *)0x800EF71C)
#define LABEL_ROW1  ((short *)0x800EF736)
#define LABEL_BASE  0x1AE
#define LABEL_CELLS 11

extern const u_char str_place_3d_adv[];
extern const u_char str_place_2d[];
extern const u_char str_place_battle_top[];
extern const u_char str_place_in_battle[];

extern void TileMapWriteRow(const u_char *src, short *dst, int base,
                            u_short count);

void DrawPlaceLabel(short in_battle)
{
    switch (in_battle) {
    case 0:
        TileMapWriteRow(str_place_3d_adv, LABEL_ROW0, LABEL_BASE, LABEL_CELLS);
        TileMapWriteRow(str_place_2d, LABEL_ROW1, LABEL_BASE, LABEL_CELLS);
        break;
    case 1:
        TileMapWriteRow(str_place_battle_top, LABEL_ROW0, LABEL_BASE,
                        LABEL_CELLS);
        TileMapWriteRow(str_place_in_battle, LABEL_ROW1, LABEL_BASE,
                        LABEL_CELLS);
        break;
    }
}
