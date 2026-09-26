/* Persona 1 (JP) - the item counters' buy and sell pages.  ADV only.
 *   0x800A9978 ShopBuyOpen        0x800A9EF0 ShopSellOpen
 *
 * Each lays out its page: eight rows of stock or of the bag, the count
 * window with the price and the money, the scroll marks, and the count
 * cursor opened on the row under the list cursor.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/menuctx.h>
#include <persona/adv/personapage.h>

#define PAGE_MARK_SLOT 32
#define SHOP_ROWS      8

#define g_money       (*(u_int *)0x801F2674)
#define g_shop_items  ((u_short *)0x800EB590)
#define g_item_list   ((u_short *)0x800EAE4C)

extern short    g_item_top;
extern short    D_800BB7F4;
extern short    D_800B8458;
extern short    D_800BBB24;
extern u_char   g_facility_count;
extern MenuList D_800BB838;
extern int      D_800EB5D0[];    /* the prices, as the buy page reaches them */
extern u_char   D_800BA640[];
extern u_char   D_800BA664[];
extern u_char   D_800B1D08[];
extern u_char   D_800B2330[];
extern u_char   D_800B1EB8[];

extern void   TextItemStatRow(short item, short x, short y);
extern void   ShopCountInit(u_char row);
extern u_char ShopHave(short item);
extern void   ShopBuyListDraw(void);
extern void   DrawItemRow(short n, short *dst);
extern short  func_800AB040(void);

/* 99.8%: the row loop's counter and row pointer take s1/s2 the other way
   round in the image. */
#ifdef NON_MATCHING
void ShopBuyOpen(void)
{
    int      i;
    short    row;
    u_short *item;
    int      k;

    row = g_item_top + g_menu->unk100.cur;
    func_8008EDBC(0x14);
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap2, 0, MAP_W, 0x40, MAP_W);
    TileMapDrawWindow(g_tilemap0, 0x20, 0x11, MAP_W);
    TileMapDrawBox(g_tilemap0 + MAP_W + 1, 0x1E, 0xF, MAP_W);
    for (i = 0; i < SHOP_ROWS; i++) {
        TileMapWriteBar(AT(g_tilemap0, i + 2, 3), 0xA);
        TileMapWriteBar(AT(g_tilemap0, i + 2, 15), 0xA);
        TileMapWriteBar(AT(g_tilemap0, i + 2, 26), 3);
    }
    TileMapFillRect(AT(g_tilemap0, 2, 13), 0x17, 2, 8, MAP_W);
    TileMapFillRect(AT(g_tilemap0, 2, 25), 0x17, 1, 8, MAP_W);
    TileMapFillRect(AT(g_tilemap0, 11, 2), 0x17, 0x1C, 4, MAP_W);
    TileMapWriteRow(str_cell_run, AT(g_tilemap2, 0, 14), 0x46E, 4);
    TileMapWriteRow(str_cell_run, AT(g_tilemap2, 10, 13), 0x38D, 5);
    TileMapWriteRow(str_cell_run, AT(g_tilemap2, 12, 13), 0x368, 5);
    TileMapWriteBar(AT(g_tilemap0, 12, 18), 0xA);
    TileMapWriteBar(AT(g_tilemap0, 14, 18), 0xA);
    TileMapWriteBar(AT(g_tilemap0, 14, 11), 3);
    k = row;
    item = &g_shop_items[k];
    TextItemStatRow(*item & 0x1FF, 0x38, 0xE);
    ShopCountInit(row);
    TileMapFillRect(AT(g_tilemap2, 11, 16), 0, 9, 1, MAP_W);
    TileMapFillRect(AT(g_tilemap2, 13, 16), 0, 9, 1, MAP_W);
    *AT(g_tilemap2, 11, 15) = 0xD0;
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap2, 11, 24), GLYPH_DIGIT0,
                       FormatDecimal(D_800EB5D0[k], g_hud_digits, 9));
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap2, 13, 10), GLYPH_DIGIT0,
                       FormatDecimal(ShopHave(*item), g_hud_digits, 2));
    *AT(g_tilemap2, 13, 8) = 0xCE;
    *AT(g_tilemap2, 13, 15) = 0xD0;
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap2, 13, 24), GLYPH_DIGIT0,
                       FormatDecimal(g_money, g_hud_digits, 9));
    TileMapWriteRow(D_800BA640, AT(g_tilemap2, 13, 0), 0, 7);
    SlotClearAll();
    SlotClear(0x2F);
    SlotInitTagged(D_800B1EB8, 0x2E, 0x24, 0x36, 0xC);
    SlotInitTagged(D_800B1D08, 0x3C, 0x300, 0x18, 0x18);
    SlotInitTagged(D_800B2330, 0x2D, 0x2FF, 0, 0x10);
    SlotSetAnim(0x2D, 0, 0, 0, 0x90, 0x24, 0, 0);
    SlotInitTagged(g_pdata_cursor_def, 4, 0x42, 0x48, g_menu->unk100.cur * 12 + 0x30);
    SlotSetFlicker(4, 1);
    SlotInitTagged(g_pdata_mark_up_def, PAGE_MARK_SLOT, 0x42, 0x98, 0x30);
    SlotInitTagged(g_pdata_mark_down_def, PAGE_MARK_SLOT + 1, 0x42, 0x98, 0x84);
    g_slot_cur = &g_slots[PAGE_MARK_SLOT];
    if (g_item_top == 0) {
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
    } else {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }
    g_slot_cur = &g_slots[PAGE_MARK_SLOT + 1];
    if (g_item_top == g_facility_count - SHOP_ROWS) {
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
    } else {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }
    MenuListInit(&D_800BB838, 0, -1, 0xA, 0x90);
    ShopCountInit(row);
    ShopBuyListDraw();
    g_map_scroll_y = g_item_top * 12;
}
#else
INCLUDE_ASM("adv/nonmatchings/ui/shoppages", ShopBuyOpen);
#endif

void ShopSellOpen(void)
{
    int   i;
    short n;

    n = D_800BB7F4 * 2 + g_menu->stock.cur + g_menu->page.cur * 2;
    func_8008EDBC(0x14);
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap2, 0, MAP_W, 0x40, MAP_W);
    TileMapDrawWindow(g_tilemap0, 0x20, 0x11, MAP_W);
    TileMapDrawBox(g_tilemap0 + MAP_W + 1, 0x1E, 0xF, MAP_W);
    for (i = 0; i < SHOP_ROWS; i++) {
        TileMapWriteBar(AT(g_tilemap0, i + 2, 3), 0xA);
        TileMapWriteBar(AT(g_tilemap0, i + 2, 13), 2);
        TileMapWriteBar(AT(g_tilemap0, i + 2, 17), 0xA);
        TileMapWriteBar(AT(g_tilemap0, i + 2, 27), 2);
    }
    TileMapFillRect(AT(g_tilemap0, 2, 15), 0x17, 2, 8, MAP_W);
    TileMapFillRect(AT(g_tilemap0, 11, 2), 0x17, 0x1C, 4, MAP_W);
    TileMapWriteRow(str_cell_run, AT(g_tilemap2, 10, 13), 0x38D, 5);
    TileMapWriteRow(str_cell_run, AT(g_tilemap2, 12, 13), 0x368, 5);
    TileMapWriteBar(AT(g_tilemap0, 12, 18), 0xA);
    TileMapWriteBar(AT(g_tilemap0, 14, 18), 0xA);
    TileMapWriteBar(AT(g_tilemap0, 12, 11), 3);
    TextItemStatRow(g_item_list[n] & 0x1FF, 0x38, 0xE);
    ShopCountInit(n);
    *AT(g_tilemap2, 11, 15) = 0xD0;
    *AT(g_tilemap2, 11, 8) = 0xCE;
    *AT(g_tilemap2, 13, 15) = 0xD0;
    *AT(g_tilemap2, 11, 15) = 0xD0;
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap2, 13, 24), GLYPH_DIGIT0,
                       FormatDecimal(g_money, g_hud_digits, 9));
    TileMapWriteRow(D_800BA664, AT(g_tilemap2, 11, 2), 0, 4);
    *AT(g_tilemap2, 11, 10) = GLYPH_DIGIT0;
    *AT(g_tilemap2, 11, 24) = GLYPH_DIGIT0;
    SlotClearAll();
    SlotClear(0x2F);
    SlotInitTagged(D_800B1EB8, 0x2E, 0x24, 0x36, 0xC);
    SlotInitTagged(D_800B1D08, 0x3C, 0x300, 0x18, 0x18);
    SlotInitTagged(D_800B2330, 0x2D, 0x2FF, 0, 0x10);
    SlotSetAnim(0x2D, 0, 0, 0, 0xC0, 0x24, 0, 0);
    SlotInitTagged(g_pdata_cursor_def, 4, 0x42, 0x38, g_menu->unk100.cur * 12 + 0x30);
    SlotSetFlicker(4, 1);
    SlotInitTagged(g_pdata_mark_up_def, PAGE_MARK_SLOT, 0x42, 0xA8, 0x30);
    SlotInitTagged(g_pdata_mark_down_def, PAGE_MARK_SLOT + 1, 0x42, 0xA8, 0x84);
    g_slot_cur = &g_slots[PAGE_MARK_SLOT];
    if (D_800BB7F4 == 0) {
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
    } else {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }
    g_slot_cur = &g_slots[PAGE_MARK_SLOT + 1];
    if (D_800BB7F4 == g_facility_count - SHOP_ROWS) {
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
    } else {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }
    MenuListInit(&D_800BB838, 0, -1, 0xA, 0x90);
    ShopCountInit(n);
    D_800BBB24 = func_800AB040() + 1;
    if (D_800BBB24 < 0x10) {
        D_800BBB24 = 0x10;
    }
    for (i = 0; i < SHOP_ROWS; i++) {
        DrawItemRow((D_800BB7F4 + i) * 2, AT(g_tilemap1, (D_800BB7F4 + i) & 0x1F, 0));
        DrawItemRow((D_800BB7F4 + i) * 2 + 1,
                    AT(g_tilemap1, (D_800BB7F4 + i) & 0x1F, 14));
    }
    D_800B8458 = 0;
    g_map_scroll_y = D_800BB7F4 * 12;
}
