/* Persona 1 (JP) - the fusion screens' pages.  ADV only.
 *   0x800A0E60 FuseResultsOpen    0x800A11EC FusionOpen
 *   0x800A14C4 FuseResultRowDraw  0x800A1690 FuseResultLineDraw
 *   0x800A1824 FusionStockDraw
 *
 * The list of results a search can give (fusesearch.c), and the stock page
 * two Personas are picked from with the would-be result on its bottom line
 * (fusion.c).
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/menuctx.h>
#include <persona/common/persona.h>
#include <persona/adv/personapage.h>

#define PAGE_MARK_SLOT 32
#define GLYPH_DIGIT0   0xC0
#define RESULT_ROWS    6
#define ROW_H          24
#define STOCK_SLOTS    12

typedef struct {
    u_short arcana;
    u_short unk2;
    u_short unk4;
    u_short persona;
    u_short flag;
} FuseResult;

#define g_fuse         (*(FuseResult *)0x801F1B8C)
#define g_fuse_results ((u_short *)0x80130000)

extern short   g_swap_top;
extern short   g_header_scroll_y;
extern short   D_800BB950;
extern u_char  g_kind_labels[];
extern u_char  D_800B1D08[];
extern u_char  D_800B2330[];
extern u_char  D_800B130C[];

extern short PersonaStockCompact(void);
extern void  func_800A1990(u_char a, u_char b, short mode, FuseResult *out,
                           short special);
extern void  func_800A275C(void);

void FuseResultRowDraw(short row);
void FuseResultLineDraw(short id);
void FusionStockDraw(void);

void FuseResultsOpen(void)
{
    int i;

    func_8008EDBC(0x1E);
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap2, 0, MAP_W, 0x40, MAP_W);
    TileMapDrawWindow(g_tilemap0, 0x20, 0x11, MAP_W);
    TileMapDrawBox(g_tilemap0 + MAP_W + 1, 0x1E, 0xF, MAP_W);
    for (i = 0; i < RESULT_ROWS; i++) {
        TileMapWriteBar(AT(g_tilemap0, i * 2 + 4, 2), 0xC);
        TileMapWriteBar(AT(g_tilemap0, i * 2 + 4, 14), 4);
        TileMapWriteBar(AT(g_tilemap0, i * 2 + 4, 18), 0xC);
        TileMapFillRect(AT(g_tilemap0, i * 2 + 3, 2), 0x17, 0x1C, 1, MAP_W);
        FuseResultRowDraw(g_swap_top + i);
    }
    TileMapWriteRow(str_cell_run, g_tilemap2, 0x476, 6);
    TileMapWriteRow(str_cell_run, AT(g_tilemap2, 0, 7), 0x457, 6);
    g_map_scroll_y = g_swap_top * ROW_H;
    SlotClearAll();
    SlotInitTagged(D_800B1D08, 0x3C, 8, 0x18, 0x18);
    SlotInitTagged(D_800B2330, 0x2D, 7, 0, 0x10);
    SlotSetAnim(0x2D, 0, 0, 0, 0x30, 0x24, 0, 0);
    if (D_800BB950 >= RESULT_ROWS) {
        SlotInitTagged(g_pdata_mark_up_def, PAGE_MARK_SLOT, 0x42, 0xB0, 0x24);
        SlotInitTagged(g_pdata_mark_down_def, PAGE_MARK_SLOT + 1, 0x42, 0xB0,
                       0xC0);
        g_slot_cur = &g_slots[PAGE_MARK_SLOT];
        if (g_swap_top == 0) {
            g_slot_cur->attr |= SLOT_ATTR_HIDE;
        } else {
            g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
        }
        g_slot_cur = &g_slots[PAGE_MARK_SLOT + 1];
        if (g_swap_top != D_800BB950 - RESULT_ROWS) {
            g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
        } else {
            g_slot_cur->attr |= SLOT_ATTR_HIDE;
        }
    }
    SlotInitTagged(D_800B130C, 1, 0x42, 0x48, g_menu->status_who.cur * ROW_H + 0x30);
    SlotSetFlicker(1, 1);
    g_cam_y = 0;
    g_header_scroll_y = 0;
    g_map_scroll_y = g_swap_top * ROW_H;
}

/* The stock page: twelve rows, with what the two picked would make below. */
void FusionOpen(void)
{
    int i;

    func_8008EDBC(0x19);
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap2, 0, MAP_W, 0x40, MAP_W);
    TileMapDrawWindow(g_tilemap0, 0x26, 0x13, MAP_W);
    TileMapDrawBox(g_tilemap0 + MAP_W + 1, 0x24, 0x11, MAP_W);
    for (i = 0; i < STOCK_SLOTS; i++) {
        TileMapWriteBar(AT(g_tilemap0, i + 3, 2), 2);
        TileMapWriteBar(AT(g_tilemap0, i + 3, 4), 6);
        TileMapWriteBar(AT(g_tilemap0, i + 3, 10), 4);
        TileMapWriteBar(AT(g_tilemap0, i + 3, 14), 0xA);
    }
    TileMapFillRect(AT(g_tilemap0, 3, 24), 0x17, 0xC, 0xC, MAP_W);
    TileMapWriteBar(AT(g_tilemap0, 2, 2), 0x16);
    TileMapWriteBar(AT(g_tilemap0, 2, 24), 0xC);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 0, 3), 0x46A, 4);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 0, 9), 0x383, 2);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 0, 15), 0x472, 4);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 13, 2), 0x476, 6);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 14, 13), 0x383, 2);
    FusionStockDraw();
    func_800A275C();
    TileMapWriteBar(AT(g_tilemap0, 16, 3), 0x1C);
    PersonaStockCompact();
    func_800A1990(g_persona_stock[g_menu->status_page.cur],
                  g_persona_stock[g_menu->top.cur], 0, &g_fuse, 0);
    FuseResultLineDraw(g_fuse.persona);
    SlotClearAll();
    SlotInitTagged(D_800B1D08, 0x3C, 8, 0x18, 0x18);
    SlotInitTagged(D_800B2330, 0x2D, 7, 0, 0x10);
    SlotSetAnim(0x2D, 0, 0, 0, 0x30, 0x24, 0, 0);
    g_cam_y = 0;
    g_map_scroll_y = 0;
}

void FuseResultRowDraw(short row)
{
    int      r;
    short   *line;
    u_short  id;

    r = row & 0xF;
    line = AT(g_tilemap1, r * 2 + 1, 2);
    TileMapFillRect(line, 0, 0x1A, 1, MAP_W);
    id = g_fuse_results[row];
    if (id != 0xFF) {
        TileMapWriteRow(&g_kind_labels[(g_persona_defs[id].kind - 1) * 10],
                        AT(g_tilemap1, r * 2 + 1, 1), 0, 10);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, r * 2 + 1, 12), 0x383, 2);
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, r * 2 + 1, 15),
                           GLYPH_DIGIT0,
                           FormatDecimal(g_persona_defs[id].level, g_hud_digits, 2));
        TileMapWriteRow(g_persona_defs[id].name, AT(g_tilemap1, r * 2 + 1, 17),
                        0, 10);
        return;
    }
    TileMapFillRect(line, 0x1A3, 10, 1, MAP_W);
    TileMapFillRect(AT(g_tilemap1, r * 2 + 1, 12), 0x1A3, 4, 1, MAP_W);
    TileMapFillRect(AT(g_tilemap1, r * 2 + 1, 17), 0x1A3, 10, 1, MAP_W);
}

void FuseResultLineDraw(short id)
{
    TileMapFillRect(AT(g_tilemap1, 14, 2), 0, 10, 1, MAP_W);
    TileMapFillRect(AT(g_tilemap1, 14, 15), 0, 2, 1, MAP_W);
    TileMapFillRect(AT(g_tilemap1, 14, 18), 0, 10, 1, MAP_W);
    if (id != 0) {
        TileMapWriteRow(&g_kind_labels[(g_persona_defs[id].kind - 1) * 10],
                        AT(g_tilemap1, 14, 2), 0, 10);
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 14, 16), GLYPH_DIGIT0,
                           FormatDecimal(g_persona_defs[id].level, g_hud_digits, 2));
        TileMapWriteRow(g_persona_defs[id].name, AT(g_tilemap1, 14, 18), 0, 10);
        return;
    }
    TileMapFillRect(AT(g_tilemap1, 14, 2), 0x1A3, 10, 1, MAP_W);
    TileMapFillRect(AT(g_tilemap1, 14, 15), 0x1A3, 2, 1, MAP_W);
    TileMapFillRect(AT(g_tilemap1, 14, 18), 0x1A3, 10, 1, MAP_W);
}

/* The stock, one row a slot: its number, arcana, level and name. */
void FusionStockDraw(void)
{
    int i;
    int p;

    TileMapFillRect(AT(g_tilemap1, 1, 0), 0, 0x14, STOCK_SLOTS, MAP_W);
    for (i = 0; i < STOCK_SLOTS; i++) {
        p = g_persona_stock[i];
        if (p != 0) {
            *AT(g_tilemap1, i + 1, 0) = i + 0x418;
            *AT(g_tilemap1, 0, 22 + i) = i + 0x418;
            DrawPersonaName(p, AT(g_tilemap1, i + 1, 12), 0);
            TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, i + 1, 10),
                               GLYPH_DIGIT0,
                               FormatDecimal(g_persona_data[p].level,
                                             g_hud_digits, 3));
            TileMapWriteRow(&g_arcana_labels[(g_persona_data[p].arcana - 1) * 6],
                            AT(g_tilemap1, i + 1, 2), 0, 6);
        }
    }
}
