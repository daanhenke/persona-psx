/* Persona 1 (JP) - the list of saved formations.  DNG only.
 *   0x80090DC0 FormationDrawPresets
 *
 * ADV's routine (src/adv/ui/formationpresets.c) with the preset check
 * called rather than written out in place: eight rows, one per saved
 * layout - "saved" and its number for one that fits the party, the same in
 * the dim bank for one saved for another size, dashes for none.
 *
 * The blanking at the top is off, as in ADV's: it indexes the character map
 * with the row counter before the loop has given it a value.
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

/* 97.75%: the number's sum. The image adds GLYPH_ONE to the bank value in
   place (it stays in s0 from the call) and then the row; every spelling here
   adds the row first. ADV's draft has the same residual. */
#ifdef NON_MATCHING
void FormationDrawPresets(void)
{
    u_char i;
    u_char dim;
    short  fits;

    TileMapFillRect(&g_tilemap1[i * MAP_W], 0, 10, PRESETS, MAP_W);
    i = 0;
    dim = 0;
    for (; i < PRESETS; i++) {
        fits = FormationPresetFits(i);
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
INCLUDE_ASM("dng/nonmatchings/ui/formationpresets", FormationDrawPresets);
#endif
