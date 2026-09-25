/* Persona 1 (JP) - the facility screens.  ADV only.
 *   0x80098B8C FacilityScreen
 *
 * Shops and the other counters a scene opens (script commands 2A and 4B)
 * share one host. `id` picks a (kind, shop) pair out of g_facility_defs; the
 * host sets up the menu screens the way FormationMenu does, then each kind
 * lays out its own page and runs its own step every frame until the step
 * counter reads 0xFF. Kinds 0 to 2 and 5 are item counters, 4 the money
 * page, 9 the Persona stock. It answers g_facility_leave, which a counter
 * sets when the scene should be left.
 */
#include <decomp/types.h>
/* FormatDecimal's count comes back as an int in this unit. */
#define TILEMAP_INT_COUNT
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/menuctx.h>
#include <persona/common/bg.h>
#include <persona/common/item.h>
#include <persona/adv/personapage.h>

#define g_seq_handle ((short *)0x801F537C)
#define g_money2     (*(u_int *)0x801F2678)

/* ADVCMD.BIN, unpacked, and its members. */
#define PACK_AT   ((u_long *)0x80118000)
#define MEMBER(n) ((u_long *)((u_char *)PACK_AT + PACK_AT[n]))

/* The open counter: which shop, and the kind of counter it is. */
#define g_facility ((u_char *)0x800EB580)

/* The shop's stock, as item ids; reached by address. */
#define g_shop_items ((short *)0x800EB590)

#define STEP_DONE  0xFF
#define PAD_TOGGLE 0x100

typedef struct {
    u_char kind;
    u_char shop;
} FacilityDef;

extern FacilityDef g_facility_defs[];
extern u_char  g_facility_kind;
extern u_char  g_facility_count;
extern u_char  g_facility_leave;
extern u_char  D_800BA0C0[];
extern u_char  D_800BA0E4[];
extern u_char  g_money_msg[];   /* the money page's message script */
extern u_char  g_money_label[]; /* its four-cell heading           */
extern int     D_800BBB1C;
extern short   D_800BB950;
extern short   D_800BB820;
extern short   D_800BB7F4;
extern MenuList D_800BB848;
extern MenuList D_800BB838;
extern MenuList D_800BC604;
extern short   g_persona_data_step;
extern short   g_cutscene_alt;
extern int     g_image_queue_count;
extern int     g_BB998;
extern u_char  g_BC5C8;
extern u_char  g_menu_allow_hold;
extern u_short g_key_menu_close;
extern int     g_pad_pressed[];
extern u_char  g_pad_config;
extern u_char  g_party_last;
extern short   g_stock_last;
extern short   g_item_top;
extern short   g_swap_top;
extern short   g_cam_x;
extern short   g_map_scroll_x;
extern short   g_header_scroll_y;
extern short   D_800BBC04;
extern short   g_view_dx, g_view_dy, g_view2_dx, g_view2_dy;
extern GsMAP   D_800B8370;
extern GsCELL  g_panel_cells[];
extern u_char  g_hud_digits[];
extern u_char  D_800B1D08[];
extern u_char  D_800B2330[];
extern u_char  D_800B110C[];
extern u_char  D_800B148C[];
extern u_char  D_800B1528[];

extern void  PadLoadBindings(u_char config);
extern void  PadSetPageButtons(u_char config);
extern void  VramClearRect(int x, int y, int w, int h);
extern void  SoundOpenSeq(u_short slot, u_short seq, short vab);
extern void  SoundPlaySeq(u_short slot, u_short seq, short vab);
extern void  CellsInit(void);
extern void  FontCellsInit(void);
extern u_char PartyLastSlot(void);
extern short PersonaStockCompact(void);
extern void  BgReset(void);
extern void  FadeBlackout(void);
extern void  ImageAnimStopAll(void);
extern void  CheckerMapInit(void);
extern void  FadeUpBlocking(short step, short limit);
extern void  FadeDownBlocking(short step, short floor);
extern void  SsSetNck(short handle);
extern void  CopyShorts(u_short *src, u_short *dst, u_short count);
extern void  TextItemStatRow(short item, short x, short y);
extern void  StatusStockOpen();
extern void  StatusStockScreen(void);
extern void  func_80012FF8(void);
extern void  func_80013218(void);
extern void  func_80013160(void);
extern void  func_800AA660(u_char shop);
extern void  func_800AA6E0(u_char shop);
extern void  func_800AA760(u_char shop);
extern void  func_800AA7E0(u_char shop);
extern void  func_800AAD10(u_char n);
extern void  func_800AAD50(u_char n);
extern void  func_800A81C8(void);
extern void  func_800A6728(void);
extern void  func_800A5560(void);
extern int   func_800A5B34(void);
extern void  func_800A03C8(void);
extern void  func_800A2A48(void);
extern void  func_800A11EC(void);
extern short func_8009BCE4(void);
extern void  func_800A0E60(void);
extern void  func_80099E9C(void);
extern void  func_800A8104(void);
extern void  func_800A6674(void);
extern void  func_800A5B7C(void);
extern void  func_800A4E2C(void);
extern void  func_800A2994(void);
extern void  func_8009D5A8(void);
extern void  func_8009BE48(void);
extern void  func_80099DE8(void);

u_char FacilityScreen(u_char id)
{
    u_char *fac;
    short   n;
    int     i;
    int     unused[2];  /* eight bytes of frame the original has */

    g_facility_leave = 0;
    g_image_queue_count = 0;
    PadLoadBindings(g_pad_config);
    fac = g_facility;
    PadSetPageButtons(g_pad_config);
    g_menu = (MenuCtx *)0x800ECC80;
    TimQueueAt(MEMBER(1), 0x380, 0x1C8, 0, 0x1FC);
    TimQueueAt(MEMBER(0), 0x380, 0x100, 0x3C0, 0x1A0);
    TimQueueAt(MEMBER(2), 0x300, 0x100, 0x3C0, 0x1B0);
    TimQueueAt(MEMBER(3), 0x200, 0x100, 0, 0x1F8);
    TimQueueAt(MEMBER(4), 0x280, 0x100, 0, 0x1F0);
    TimQueueAt(MEMBER(5), 0x300, 0x140, 0x3D0, 0x180);
    TimQueueAt(MEMBER(6), 0x300, 0x1A0, 0x3E0, 0x180);
    TimQueueAt(MEMBER(7), 0x300, 0x130, 0x3F0, 0x1B0);
    VramClearRect(0, 0, 0x140, 0x1DF);
    g_menu_allow_hold = 0;
    g_facility_kind = g_facility_defs[id].kind;
    fac[0] = g_facility_defs[id].shop;
    g_cutscene_alt = 0;
    func_80012FF8();
    func_80013218();
    SoundOpenSeq(0x18, 0, 0);
    SoundOpenSeq(0x19, 0, 0);
    SoundOpenSeq(0x1A, 0, 0);
    SoundOpenSeq(0x1B, 0, 0);
    CellsInit();
    FontCellsInit();

    g_bg_layer_otz[0] = 0x60;
    g_bg_layer_otz[1] = 0x40;
    g_bg_layer_otz[2] = 0x40;
    g_bg_layer_otz[3] = 0xA0;
    g_bg_layer_otz[4] = 0x20;
    g_bg_map0.base = (GsCELL *)0x800E864C;
    g_bg_layers[0].attribute = 0x9000000;
    g_bg_map0.cellw = 8;
    g_bg_map0.cellh = 12;
    g_bg_map0.ncellw = MAP_W;
    g_bg_map0.ncellh = 0x40;
    g_bg_map0.index = (u_short *)g_tilemap0;
    g_bg_layers[1].attribute = 0x8000000;
    g_bg_layers[2].attribute = 0x8000000;
    g_bg_layers[5].attribute = 0x8000000;
    g_bg_layers[1].x = 0;
    g_bg_layers[1].y = 0;
    g_bg_map1.base = (GsCELL *)0x800E3E4C;
    g_bg_map1.cellw = 8;
    g_bg_map1.cellh = 12;
    g_bg_map1.ncellw = MAP_W;
    g_bg_map1.ncellh = 0x20;
    g_bg_map1.index = (u_short *)g_tilemap1;
    g_bg_layers[2].x = 0;
    g_bg_layers[2].y = 0;
    g_bg_map2.base = (GsCELL *)0x800E3E4C;
    g_bg_map2.cellw = 8;
    g_bg_map2.cellh = 12;
    g_bg_map2.ncellw = MAP_W;
    g_bg_map2.ncellh = 0x20;
    g_bg_map2.index = (u_short *)g_tilemap2;
    D_800B8370.base = (GsCELL *)0x800E3E4C;
    D_800B8370.cellw = 8;
    D_800B8370.cellh = 12;
    D_800B8370.ncellw = 0xC;
    D_800B8370.ncellh = 3;
    D_800B8370.index = (u_short *)g_panel_cells;
    g_cam_y = 0;
    g_cam_x = 0;
    g_map_scroll_y = 0;
    g_map_scroll_x = 0;
    g_header_scroll_y = 0;
    D_800BBC04 = 0;
    g_view_dy = 0;
    g_view_dx = 0;
    g_view2_dy = 0;
    g_view2_dx = 0;

    g_party_last = PartyLastSlot();
    g_stock_last = PersonaStockCompact();
    MenuListInit(&g_menu->status_who, 0, 0, g_party_last, 0x1A);
    MenuListInit(&g_menu->page, 0, 0, 7, 0x14);
    MenuListInit(&g_menu->stock, 0, 0, 1, 0x1A);
    MenuListInit(&D_800BB848, 0, 0, g_facility_count - 1, 0x1E);
    MenuListInit(&D_800BB838, 0, -1, 0xA, 0x90);
    MenuListInit(&g_menu->unk100, 0, 0, 7, 0x14);
    MenuListInit(&g_menu->list[2], 0, 0, 0, 6);
    MenuListInit(&g_menu->list[3], 0, 0, 0, 0xA);
    MenuListInit(&g_menu->unk050, 0, 0, g_party_last, 0x1A);
    MenuListInit(&g_menu->unk220, 0, 0, 8, 0x16);
    MenuListInit(&g_menu->unk230, 0, 0, 3, 0x14);
    MenuListInit(&g_menu->unk240, 0, 0, 1, 0x1A);
    MenuListInit(&g_menu->unk250, 0, 0, 1, 0x1A);
    g_item_top = 0;
    g_swap_top = 0;
    D_800BB7F4 = 0;
    BgReset();
    SlotClearAll();
    FadeBlackout();
    ImageAnimStopAll();
    CheckerMapInit();
    g_persona_data_step = 0;

    switch (g_facility_kind) {
    case 0:
    case 1:
    case 2:
        g_facility_count = D_800BA0C0[fac[0] * 2];
        CopyShorts(g_items, g_items_pending, ITEM_SLOTS);
        MenuListInit(&g_menu->unk050, 0, 0, g_party_last, 0x1A);
        MenuListInit(&g_menu->page, 0, 0, 7, 0x14);
        MenuListInit(&g_menu->skill_persona, 0, 0, 2, 0x2E);
        MenuListInit(&D_800BC604, 0, 0, 4, 0x1E);
        func_800AA660(fac[0]);
        func_800AA6E0(fac[0]);
        func_800AAD10(D_800BB848.cur);
        func_800A81C8();
        SlotSetFlicker(0, 1);
        goto next;
    case 3:
        MenuListInit(&g_menu->unk180, 0, 0, 5, 0x14);
        MenuListInit(&g_menu->unk190, 0, 0, 1, 0x1A);
        MenuListInit(&g_menu->skill_persona, 0, 0, 1, 0x2E);
        MenuListInit(&g_menu->unk1A0, 0, 0, 6, 0x16);
        func_800A6728();
        goto next;
    case 4:
        func_8008EDBC(0x16);
        TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
        TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
        TileMapFillRect(g_tilemap2, 0, MAP_W, 0x40, MAP_W);
        TileMapDrawWindow(g_tilemap0, 0x18, 0xE, MAP_W);
        TileMapDrawBox(AT(g_tilemap0, 1, 1), 0x16, 6, MAP_W);
        TileMapDrawBox(AT(g_tilemap0, 8, 1), 0x16, 5, MAP_W);
        TileMapWriteBar(AT(g_tilemap0, 3, 10), 0xB);
        TileMapWriteBar(AT(g_tilemap0, 5, 10), 0xB);
        TileMapWriteBar(AT(g_tilemap0, 9, 10), 0xB);
        TileMapWriteBar(AT(g_tilemap0, 11, 10), 0xB);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 1, 1), 0x461, 6);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 1, 8), 0x389, 4);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 8, 1), 0x389, 4);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 10, 1), 0x368, 5);
        *AT(g_tilemap1, 3, 8) = 0x37F;
        *AT(g_tilemap1, 9, 8) = 0x37F;
        *AT(g_tilemap1, 5, 8) = 0xD0;
        *AT(g_tilemap1, 5, 18) = 0xC0;
        *AT(g_tilemap1, 11, 8) = 0xD0;
        TileMapWriteRow(g_money_label, g_tilemap2, 0, 4);
        g_tilemap2[9] = 0xC1;
        g_tilemap2[7] = 0x37F;
        g_tilemap2[11] = 0xCF;
        g_tilemap2[13] = 0xD0;
        n = FormatDecimal(100, g_hud_digits, 3);
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap2, 0, 16), 0xC0,
                           (u_short)n);
        D_800BBB1C = func_800A5B34();
        i = (short)FormatDecimal(D_800BBB1C, g_hud_digits, 8);
        TileMapFillRect(AT(g_tilemap1, 3, 19) - i, 0xC0, i, 1, MAP_W);
        MenuListInit(&g_menu->arcana_row, i, 0, i, 0x18);
        n = FormatDecimal(g_money2, g_hud_digits, 8);
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 9, 18), 0xC0,
                           (u_short)n);
        n = FormatDecimal(G_MONEY, g_hud_digits, 9);
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 11, 18), 0xC0,
                           (u_short)n);
        SlotClear(0x2F);
        SlotInitTagged(D_800B1D08, 0x3C, 0x300, 0x18, 0x18);
        SlotInitTagged(D_800B2330, 0x2D, 0x2FF, 0, 0x10);
        SlotSetAnim(0x2D, 0, 0, 0, 0, 0x48, 0, 0);
        MenuListInit(&g_menu->unk300, 0, -1, D_800BBB1C, 0x50);
        MenuListInit(&g_menu->unk100, 0, 0, D_800BBB1C, 0x50);
        SlotInitTagged(D_800B110C, 1, 0x42, 0xE0, 0x54);
        SlotInitTagged(D_800B1E98, 0x2E, 0x200, 0x46, 0xA);
        SlotSetFlicker(1, 1);
        BgMapInit(g_money_msg, 0);
        g_bg_layers[4].x = 0x48;
        g_bg_layers[4].y = 0xC;
        g_bg_layers[4].w = 0xB0;
        g_bg_layers[4].h = 0x10;
        g_bg_shown |= 0x10;
        break;
    case 5:
        g_facility_count = D_800BA0E4[fac[0] * 2];
        CopyShorts(g_items, g_items_pending, ITEM_SLOTS);
        MenuListInit(&g_menu->unk050, 0, 0, g_party_last, 0x1A);
        MenuListInit(&g_menu->page, 0, 0, 7, 0x14);
        MenuListInit(&g_menu->skill_persona, 0, 0, 2, 0x1E);
        MenuListInit(&D_800BC604, 0, 0, 4, 0x1E);
        func_800AA760(fac[0]);
        func_800AA7E0(fac[0]);
        func_800AAD50(D_800BB848.cur);
        func_800A5560();
        SlotSetFlicker(0, 1);
        i = g_item_top + g_menu->unk100.cur;
        TextItemStatRow(g_shop_items[i], 0x38, 0xE);
        break;
    case 6:
        func_800A03C8();
        MenuListInit(&g_menu->unk2E0, 0, 0, 9, 0x14);
        D_800BB7F4 = 0;
        func_800A2A48();
    next:
        g_persona_data_step++;
        break;
    case 7:
        func_800A03C8();
        /* Called without a prototype here: the count comes back as an
           int, not extended from a short. */
        i = ((int (*)())PersonaStockCompact)();
        MenuListInit(&g_menu->top, 0, 0, i, 0x1A);
        MenuListInit(&g_menu->status_page, 0, 0, i, 0x16);
        MenuListInit(&g_menu->unk2D0, 0, 0, 4, 0x14);
        MenuListInit(&g_menu->unk2E0, 0, 0, 1, 0x1A);
        D_800BB7F4 = 0;
        D_800BB820 = 0;
        func_800A11EC();
        if (i != 0) {
            SlotInitTagged(D_800B148C, 1, 0x42, 0xC8, 0x30);
            SlotInitTagged(D_800B1528, 2, 0x42, 0x20, 0x30);
            SlotSetFlicker(1, 1);
            SlotSetFlicker(2, 1);
            g_slot_cur = &g_slots[1];
            g_slots[1].flicker = 0;
            g_slot_cur = &g_slots[2];
            g_slots[2].flicker = 0;
        }
        break;
    case 8:
        func_800A03C8();
        PersonaStockCompact();
        D_800BB950 = n = func_8009BCE4();
        if (n > 6) {
            MenuListInit(&g_menu->status_who, 0, 0, 5, 0x14);
        } else {
            MenuListInit(&g_menu->status_who, 0, 0, n - 1, 0x16);
        }
        MenuListInit(&g_menu->unk2D0, 0, 0, 4, 0x14);
        MenuListInit(&g_menu->unk2E0, 0, 0, 1, 0x1A);
        D_800BB7F4 = 0;
        g_item_top = 0;
        g_swap_top = 0;
        D_800BB820 = 1;
        func_800A0E60();
        break;
    case 9:
        MenuListInit(&g_menu->stock, 0, 0, g_stock_last + 1, 0x1E);
        MenuListInit(&g_menu->stock_release, 0, 0, g_stock_last, 0x1E);
        StatusStockOpen(0);
        break;
    case 10:
        MenuListInit(&g_menu->unk2E0, 0, 0, 1, 0x1E);
        func_80099E9C();
        break;
    }
    FadeUpBlocking(4, 0x80);

    while (g_persona_data_step != STEP_DONE) {
        RunFrame();
        switch (fac[1]) {
        case 0:
            func_800A8104();
            break;
        case 1:
            func_800A8104();
            break;
        case 2:
            func_800A8104();
            break;
        case 3:
            func_800A6674();
            break;
        case 4:
            func_800A5B7C();
            break;
        case 5:
            func_800A4E2C();
            break;
        case 6:
            func_800A2994();
            break;
        case 7:
            func_8009D5A8();
            break;
        case 8:
            func_8009BE48();
            break;
        case 9:
            StatusStockScreen();
            g_persona_data_step = STEP_DONE;
            break;
        case 10:
            func_80099DE8();
            break;
        }
        if (g_pad_pressed[0] & PAD_TOGGLE) {
            g_BB998 ^= 1;
        }
        if (!g_menu_allow_hold && (g_key_menu_close & g_pad_pressed[0])) {
            g_menu_allow_hold = 1;
            SoundPlaySeq(0x18, 0, 1);
        }
    }
    FadeDownBlocking(8, 0);
    for (i = 0; i < 16; i++) {
        RunFrame();
    }
    func_80013160();
    SsSetNck(g_seq_handle[0x18]);
    SsSetNck(g_seq_handle[0x19]);
    SsSetNck(g_seq_handle[0x1A]);
    SsSetNck(g_seq_handle[0x1B]);
    return g_facility_leave;
}
