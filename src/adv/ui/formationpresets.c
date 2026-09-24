/* Persona 1 (JP) - the list of saved formations.  ADV only.
 *   0x8008CC5C FormationDrawPresets
 *
 * Eight rows on the character map, one per saved layout: a layout that fits
 * the party as it is now reads "saved" with its number, one saved for a party
 * of another size the same in the dim bank, and one never saved a row of
 * dashes. The test is FormationPresetFits, which the original has in this
 * file and gcc writes out in place.
 *
 * The blanking at the top is off: it indexes the character map with the row
 * counter before the loop has given it a value.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/common/formation.h>
#include <persona/common/tilemap.h>

#define PRESETS    8
#define LABEL_COL  2
#define NUMBER_COL 9
#define GLYPH_BANK 0xD7
#define GLYPH_ONE  0xA6    /* the first preset's number */

extern const u_char str_preset_saved[];
extern const u_char str_empty[];

/* 77.22%: the preset check's loop keeps its element pointer and its -1 in
   other registers than the original's. */
#ifdef NON_MATCHING
void FormationDrawPresets(void)
{
    u_char  i;
    u_char  dim;
    short   fits;
    u_char *row;
    short   cell;
    short   n;
    short   highest;

    TileMapFillRect(&g_tilemap1[i * MAP_W], 0, 10, PRESETS, MAP_W);
    dim = 0;
    for (i = 0; i < PRESETS; i++) {
        highest = 0;
        n = -1;
        row = &g_formation_preset[i * GRID_CELLS];
        for (cell = 0; cell < GRID_CELLS; cell++) {
            if (row[cell] == CELL_EMPTY) {
                continue;
            }
            n++;
            if (highest < row[cell]) {
                highest = row[cell];
            }
        }
        if (n == -1) {
            fits = -1;
        } else {
            fits = n == g_party_last;
        }
        switch (fits) {
        case -1:
            TileMapWriteRow(str_empty, &g_tilemap1[i * MAP_W + LABEL_COL],
                            GLYPH_BANK, 5);
            break;
        case 0:
            dim = 1;
        case 1:
            TileMapWriteRow(str_preset_saved, &g_tilemap1[i * MAP_W],
                            dim * GLYPH_BANK, 8);
            g_tilemap1[i * MAP_W + NUMBER_COL] = i + dim * GLYPH_BANK + GLYPH_ONE;
            break;
        }
        dim = 0;
    }
}
#else
INCLUDE_ASM("adv/nonmatchings/ui/formationpresets", FormationDrawPresets);
#endif
