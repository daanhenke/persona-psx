/* Persona 1 (JP) - the Persona stock screen.  ADV @ 0x800769E8.
 *
 * The twelve stock slots drawn one row each into the second character-map
 * layer. A row starts with its slot number - a glyph of its own per row - and
 * then holds the Persona's name, its level right-aligned in two cells, the
 * "LV" label ahead of the level and the six-cell arcana label after it:
 *
 *      col  0     slot number
 *      col  1-10  name
 *      col 12-13  label
 *      col 14-15  level
 *      col 17-22  arcana
 *
 * An empty slot keeps its number's cell and the three fields, but filled with
 * the blank-slot glyph instead.
 */
#include <decomp/types.h>
#include <persona/common/persona.h>
#include <persona/common/tilemap.h>

/* The three blocks cleared before the rows go down. */
#define NAME_W   10
#define LEVEL_AT 11
#define LEVEL_W  4
#define ARCANA_W_AT 16
#define ARCANA_BLOCK_W 10

/* Where each field sits in a row. */
#define NAME_AT     1
#define LABEL_AT    12
#define LABEL_W     2
#define DIGITS_AT   15
#define ARCANA_AT   17

/* The glyphs. Slot numbers run on from SLOT_GLYPH0, one a row. */
#define SLOT_GLYPH0  0x418
#define LABEL_BASE   0x383
#define GLYPH_EMPTY  0x1A3
#define EMPTY_NAME_AT 2
#define EMPTY_NAME_W  8

/* The six-cell label of each arcana, flat, by the 1-based PersonaData.arcana.
   The field's own copy; the status pages read it too. */
u_char g_arcana_labels[ARCANA_LABELS * ARCANA_LABEL_W] = {
    0x75, 0x70, 0x00, 0x00, 0x00, 0x00,
    0x77, 0x54, 0x70, 0x00, 0x00, 0x00,
    0x77, 0x54, 0x5F, 0x53, 0x00, 0x00,
    0x64, 0x7F, 0x5D, 0x00, 0x00, 0x00,
    0x8B, 0x64, 0x7F, 0x5D, 0x00, 0x00,
    0x79, 0xA3, 0x54, 0x56, 0x54, 0x00,
    0x87, 0xA2, 0x79, 0xA3, 0x54, 0x00,
    0x77, 0x54, 0x62, 0xA4, 0x54, 0x00,
    0x58, 0xA4, 0x54, 0x62, 0xA4, 0x54,
    0x70, 0x87, 0xA3, 0x54, 0x00, 0x00,
    0x77, 0x54, 0x87, 0xA3, 0x54, 0x00,
    0x58, 0x87, 0xA4, 0x00, 0x00, 0x00,
    0x77, 0x54, 0x58, 0x00, 0x00, 0x00,
    0x62, 0x7B, 0x53, 0x00, 0x00, 0x00,
    0x87, 0xA2, 0x58, 0x00, 0x00, 0x00,
    0x76, 0x54, 0x58, 0x00, 0x00, 0x00,
    0x52, 0x59, 0x79, 0xA4, 0x54, 0x00,
    0x5D, 0x58, 0x00, 0x00, 0x00, 0x00,
    0x84, 0x8F, 0x54, 0x00, 0x00, 0x00,
    0x85, 0x54, 0x70, 0x00, 0x00, 0x00,
    0x62, 0xA4, 0x54, 0x87, 0x7F, 0x00,
    0x70, 0x87, 0x7F, 0x00, 0x00, 0x00,
    0x5D, 0x67, 0x81, 0x71, 0x00, 0x00,
    0x70, 0x56, 0x54, 0x00, 0x00, 0x00,
    0x70, 0x87, 0x7F, 0x6A, 0x54, 0x00,
    0x70, 0x87, 0x7F, 0x00, 0x00, 0x00,
};

void PersonaStockDraw(void)
{
    int    i;
    int    id;

    TileMapFillRect(g_tilemap1, 0, NAME_W, STOCK_ROWS, MAP_W);
    TileMapFillRect(&g_tilemap1[LEVEL_AT], 0, LEVEL_W, STOCK_ROWS, MAP_W);
    TileMapFillRect(&g_tilemap1[ARCANA_W_AT], 0, ARCANA_BLOCK_W, STOCK_ROWS,
                    MAP_W);

    for (i = 0; i < STOCK_ROWS; i++) {
        id = g_persona_stock[i];
        if (id != STOCK_FREE) {
            g_tilemap1[i * MAP_W] = SLOT_GLYPH0 + i;
            DrawPersonaName(id, &g_tilemap1[i * MAP_W + NAME_AT], 0);
            TileMapWriteRowRev(g_hud_digits,
                               &g_tilemap1[i * MAP_W + DIGITS_AT],
                               GLYPH_DIGIT0,
                               FormatDecimal(g_persona_data[id].level,
                                             g_hud_digits, 2));
            TileMapWriteRow(&g_arcana_labels[(g_persona_data[id].arcana - 1) * ARCANA_LABEL_W],
                            &g_tilemap1[i * MAP_W + ARCANA_AT], 0,
                            ARCANA_LABEL_W);
            TileMapWriteRow(str_cell_run, &g_tilemap1[i * MAP_W + LABEL_AT],
                            LABEL_BASE, LABEL_W);
        } else {
            TileMapFillRect(&g_tilemap1[i * MAP_W], GLYPH_EMPTY, 1, 1, MAP_W);
            TileMapFillRect(&g_tilemap1[i * MAP_W + EMPTY_NAME_AT],
                            GLYPH_EMPTY, EMPTY_NAME_W, 1, MAP_W);
            TileMapFillRect(&g_tilemap1[i * MAP_W + LABEL_AT], GLYPH_EMPTY,
                            LEVEL_W, 1, MAP_W);
            TileMapFillRect(&g_tilemap1[i * MAP_W + ARCANA_AT], GLYPH_EMPTY,
                            ARCANA_LABEL_W, 1, MAP_W);
        }
    }
}
