/* Persona 1 (JP) - the formation menu.  ADV only.
 *   0x800711F4 FormationMenuStep   0x80071324 FormationMenuOpen
 *   0x800715EC FormationMenu
 *
 * The menu is entered from the field: the archive the preload left unpacked
 * at 0x80118000 carries its eight images, which go to VRAM, and every menu
 * cursor is reset. Each frame runs one step of the formation screen. The live
 * row of the saved layouts mirrors the grid being edited: the screen copies
 * it into the grid when it opens and back out when it is left.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/menuctx.h>
#include <persona/common/formation.h>
#include <persona/common/bg.h>
#include <persona/adv/personapage.h>

#define LIVE_ROW 8

#define g_seq_handle ((short *)0x801F537C)
#define PACK_AT      ((u_long *)0x80118000)
#define MEMBER(n)    ((u_long *)((u_char *)PACK_AT + PACK_AT[n]))

extern short   g_menu_subsel;
extern short   g_menu_sel;
extern u_char  g_menu_allow_hold;
extern u_short g_key_menu_close;
extern int     g_pad_held[];
extern int     g_pad_pressed[];
extern u_char  g_BC5C8;
extern int     g_BB94C;
extern int     D_800BB7F8;
extern short   g_stock_last;
extern GsMAP   g_bg_map0, g_bg_map1, g_bg_map2, D_800B8370;
extern GsCELL  g_panel_cells[];
extern short   D_800BB7F4, D_800BB954, D_800BB958, D_800BC224, D_800B8458,
               D_800BC048;
extern short   g_arcana_top;
extern short   g_header_scroll_y;
extern short   g_cam_x, g_map_scroll_x, D_800BBC04;
extern short   g_view_dx, g_view_dy, g_view2_dx, g_view2_dy;
extern MenuList D_800BC584;
extern MenuList D_800BC594;
extern MenuList D_800BC5A4;
extern MenuList D_800BC1A4;
extern u_char  D_801F2ACD;
extern u_char  g_pad_config;
extern u_char  D_800B1D08[];
extern u_char  D_800B2330[];
extern u_char  D_800B25E8[];
extern u_char  g_fm_prompt_cur_def[];
extern u_char  g_fm_hint_def[];
extern u_char  g_fm_hint2_def[];

extern void  MenuScreenDraw(void);
extern void  DrawStatusHud(void);
extern void  SoundOpenSeq(u_short slot, u_short seq, short vab);
extern void  SoundPlaySeq(u_short slot, u_short seq, short vab);
extern void  CellsInit(void);
extern void  FontCellsInit(void);
extern void  PadLoadBindings(u_char config);
extern void  PadSetPageButtons(u_char config);
extern void  BgReset(void);
extern void  FadeBlackout(void);
extern void  ImageAnimStopAll(void);
extern void  CheckerMapInit(void);
extern void  FadeUpBlocking(short step, short limit);
extern void  FadeDownBlocking(short step, short floor);
extern void  SsSetNck(short handle);
extern u_char PartyLastSlot(void);
extern short PersonaStockCompact(void);

extern void FormationCmdStep(void);
extern void FormationPickMember(void);
extern void FormationPlaceMember(void);
extern void FormationDonePrompt(void);
extern void FormationPresetPick(void);
extern void FormationSavePrompt(void);
extern void func_80074318(void);
extern void FormationLoadPrompt(void);

void FormationMenuOpen(void);

/* One step of the formation screen. */
inline void FormationMenuStep(void)
{
    u_char *live = &g_formation_preset[LIVE_ROW * GRID_CELLS];
    u_char *grid = g_formation;
    u_char  i;

    switch (g_menu_subsel) {
    case 0:
        FormationMenuOpen();
        for (i = 0; i < GRID_CELLS; i++) {
            live[i] = grid[i];
        }
        g_menu_subsel++;
        break;
    case 1:
        FormationCmdStep();
        break;
    case 2:
        FormationPickMember();
        break;
    case 3:
        FormationPlaceMember();
        break;
    case 4:
        FormationDonePrompt();
        break;
    case 5:
        FormationPresetPick();
        break;
    case 6:
        FormationSavePrompt();
        break;
    case 7:
        func_80074318();
        break;
    case 8:
        FormationLoadPrompt();
        break;
    }
}

/* The formation screen: the command prompt, arrange or presets, over the
   status screen, with the grid taken from the live layout. */
void FormationMenuOpen(void)
{
    u_char *live = &g_formation_preset[LIVE_ROW * GRID_CELLS];
    u_char *grid = g_formation;
    u_char  i;

    func_8008EDBC(0);
    SlotClearAll();
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap2, 0, MAP_W, 0x20, MAP_W);
    MenuScreenDraw();
    SlotClear(0x2F);
    SlotInitTagged(D_800B1D08, 0x3C, 0x300, 0x18, 0x18);
    SlotSetAnim(0x3C, 0, 0, 0, 0x80, 0, 0, 0);
    SlotInitTagged(D_800B2330, 0x2D, 0x2FF, 0, 0x10);
    SlotSetAnim(0x2D, 0, 0, 0, 0, 0x18, 0, 0);
    SlotInitTagged(g_fm_prompt_cur_def, 1, 0x23, 6,
                   g_menu->formation_cmd.cur * 16 + 0x32);
    SlotInitTagged(g_fm_hint_def, 0x20, 0x24, 6, 0x30);
    SlotInitTagged(g_fm_hint_def, 0x21, 0x24, 6, 0x40);
    SlotInitTagged(g_fm_hint2_def, 0x22, 0x22, 6, 0x30);
    SlotInitTagged(g_fm_hint2_def, 0x23, 0x22, 6, 0x40);
    SlotSetAnim(0x22, 0, 0, 0, 0x60, 0x10, 0, 0);
    SlotSetAnim(0x23, 0, 0, 0, 0x90, 0x10, 0, 0);
    SlotSetFlicker(1, 1);
    g_cam_y = 0;
    g_map_scroll_y = 0;
    SlotInitTagged(D_800B25E8, 7, 0x20, 0xD8, 0x10);
    for (i = 0; i < GRID_CELLS; i++) {
        grid[i] = live[i];
    }
    FormationRepair();
    DrawStatusHud();
}

void FormationMenu(void)
{
    u_char *live = &g_formation_preset[LIVE_ROW * GRID_CELLS];
    u_char *grid = g_formation;
    u_char  i;

    TimQueueAt(MEMBER(1), 0x380, 0x1C8, 0, 0x1FC);
    TimQueueAt(MEMBER(0), 0x380, 0x100, 0x3C0, 0x1A0);
    TimQueueAt(MEMBER(2), 0x300, 0x100, 0x3C0, 0x1B0);
    TimQueueAt(MEMBER(3), 0x200, 0x100, 0, 0x1F8);
    TimQueueAt(MEMBER(4), 0x280, 0x100, 0, 0x1F0);
    TimQueueAt(MEMBER(5), 0x300, 0x140, 0x3D0, 0x180);
    TimQueueAt(MEMBER(6), 0x300, 0x1A0, 0x3E0, 0x180);
    TimQueueAt(MEMBER(7), 0x300, 0x130, 0x3F0, 0x1B0);
    g_menu = (MenuCtx *)0x800ECC80;
    g_BC5C8 = 1;
    g_menu_allow_hold = 0;
    D_800BB7F8 = 0x8CA0;
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
    g_menu_subsel = 1;
    g_bg_map0.base = (GsCELL *)0x800E864C;
    g_bg_layers[0].attribute = 0x9000000;
    g_menu_sel = 0;
    g_BB94C = 0;
    D_800BB7F4 = 0;
    D_800BB954 = 0;
    D_800BB958 = 0;
    D_800BC224 = 0;
    g_arcana_top = 0;
    D_800B8458 = 0;
    D_800BC048 = 0;
    g_pad_held[0] = 0;
    g_pad_pressed[0] = 0;
    g_bg_map0.cellw = 8;
    g_bg_map0.cellh = 12;
    g_bg_map0.ncellw = MAP_W;
    g_bg_map0.ncellh = 0x40;
    g_bg_map0.index = (u_short *)g_tilemap0;
    g_bg_layers[1].attribute = 0x8000000;
    g_bg_layers[1].x = 0;
    g_bg_layers[1].y = 0;
    g_bg_map1.base = (GsCELL *)0x800E3E4C;
    g_bg_map1.cellw = 8;
    g_bg_map1.cellh = 12;
    g_bg_map1.ncellw = MAP_W;
    g_bg_map1.ncellh = 0x40;
    g_bg_map1.index = (u_short *)g_tilemap1;
    g_bg_layers[2].attribute = 0x8000000;
    g_bg_layers[5].attribute = 0x8000000;
    D_800B8370.ncellw = 0xC;
    D_800B8370.ncellh = 3;
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
    MenuListInit(&g_menu->top, 0, 0, 4, 0x2E);
    MenuListInit(&g_menu->unk030, 0, 0, 2, 0x2E);
    MenuListInit(&g_menu->unk040, 1, 0, 1, 0x1E);
    MenuListInit(&g_menu->unk050, 0, 0, g_party_last, 0x1A);
    MenuListInit(&g_menu->unk060, 0, 0, 2, 0x2E);
    MenuListInit(&g_menu->unk070, 0, 0, g_party_last, 0x1A);
    MenuListInit(&g_menu->unk100, 0, 0, 2, 0x2E);
    MenuListInit(&g_menu->unk220, 0, 0, 8, 0x16);
    MenuListInit(&g_menu->unk230, 0, 0, 3, 0x14);
    MenuListInit(&g_menu->unk240, 0, 0, 1, 0x1A);
    MenuListInit(&g_menu->unk250, 0, 0, 1, 0x1A);
    MenuListInit(&g_menu->formation_cmd, 0, 0, 1, 0x1E);
    MenuListInit(&g_menu->list[2], 0, 0, 0, 6);
    MenuListInit(&g_menu->list[3], 0, 0, 0, 0xA);
    MenuListInit(&D_800BC584, D_801F2ACD, 0, 1, 0x1E);
    MenuListInit(&D_800BC594, 0, 0, 0xB, 0x16);
    MenuListInit(&D_800BC5A4, 0, 0, 1, 0x1A);
    MenuListInit(&D_800BC1A4, 0, 0, 5, 0x16);
    PadLoadBindings(g_pad_config);
    PadSetPageButtons(g_pad_config);
    BgReset();
    SlotClearAll();
    FadeBlackout();
    ImageAnimStopAll();
    CheckerMapInit();
    FormationMenuOpen();
    for (i = 0; i < GRID_CELLS; i++) {
        live[i] = grid[i];
    }
    SetDispMask(1);
    FadeUpBlocking(8, 0x80);

    g_menu_subsel = 1;
    g_menu_sel = 1;
    do {
        RunFrame();
        FormationMenuStep();
        if (!g_menu_allow_hold && (g_key_menu_close & g_pad_pressed[0])) {
            g_menu_allow_hold = 1;
            SoundPlaySeq(0x18, 0, 1);
        }
    } while (g_menu_sel != 0);
    FadeDownBlocking(8, 0);
    g_BC5C8 = 0;
    SsSetNck(g_seq_handle[0x18]);
    SsSetNck(g_seq_handle[0x19]);
    SsSetNck(g_seq_handle[0x1A]);
    SsSetNck(g_seq_handle[0x1B]);
}
