/* Persona 1 (JP) - the list of saved formations.  DNG and S2D.
 *   DNG 0x80090DC0 / S2D 0x800812CC FormationDrawPresets
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
#include <persona/common/formation.h>
#include <persona/common/tilemap.h>

#define PRESETS    8
#define LABEL_COL  2
#define NUMBER_COL 9
#define GLYPH_BANK 0xD7
#ifndef PRESET_GLYPH_BASE
#define PRESET_GLYPH_BASE(bank) (bank)
#endif

#define GLYPH_ONE  0xA6    /* the first preset's number */

extern const u_char str_preset_saved[];
extern const u_char str_empty[];

/* Keep the glyph bank in a signed halfword local across the row call, then
 * add the first-digit base before the row index. This preserves gcc 2.6's
 * original sum order. The incoming, uninitialized row used by the blanking
 * call is retained from the original binary. */
void FormationDrawPresets(void)
{
    u_char i;
    u_char dim;
    short  fits;
    short  bank;

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
            bank = dim * GLYPH_BANK;
            TileMapWriteRow(str_preset_saved, &g_tilemap1[i * MAP_W],
                            PRESET_GLYPH_BASE(bank), 8);
            bank += GLYPH_ONE;
            g_tilemap1[i * MAP_W + NUMBER_COL] = i + bank;
            break;
        }
        dim = 0;
    }
}
