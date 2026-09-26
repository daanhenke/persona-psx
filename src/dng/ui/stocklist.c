/* Persona 1 (JP) - the Persona stock list, and the menu's top page put back.
 *   DNG 0x8008546C MenuTopRedraw   0x80085564 PersonaStockDraw
 *
 * The field's build of ADV's stock list (src/adv/ui/stocklist.c, which has
 * the row layout), made against a decimal formatter that returns int. ADV
 * carries MenuTopRedraw ahead of it too (0x800768F0), still in asm.
 */
#define SLOT_TAGGED_INTXY
#define TILEMAP_INT_COUNT
#include <decomp/types.h>
#include <persona/common/persona.h>
#include <persona/common/tilemap.h>
#include <persona/common/slot.h>

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
   The field's copy is in the overlay's data block. */
extern u_char g_arcana_labels[];

#ifndef g_tilemap0
#define g_tilemap0 ((short *)0x800EE180)
#endif

extern short   g_cam_y;
extern short   g_map_scroll_y;
extern u_char  g_menu_top_def[];
extern u_char  D_8009AA4C[];
extern u_char  D_8009B074[];

extern void func_80092E5C(int);
extern void MenuScreenDraw(void);

/* The menu's top page put back behind a sub-screen that is closing: the
   three layers blanked, the frame drawn, the two corner sprites and the top
   page's own sprite back in their slots, and the view scrolled home. */
void MenuTopRedraw(void)
{
    func_80092E5C(0);
    SlotClearAll();
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap2, 0, MAP_W, 0x40, MAP_W);
    MenuScreenDraw();
    SlotInitTagged(D_8009AA4C, 0x3C, 0x300, 0xA0, 0x48);
    SlotInitTagged(D_8009B074, 0x2D, 0x2FF, 0x88, 0x41);
    SlotInitTagged(g_menu_top_def, 0x2F, 0x380, 0, 0);
    g_cam_y = 0;
    g_map_scroll_y = 0;
}

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
