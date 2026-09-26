/* Persona 1 (JP) - the field's main menu.  ADV only.
 *   0x80068774 MainMenuOpen   0x800689D8 MainMenu
 *
 * The button that opens the menu from the field lands here: the screen is
 * cleared and the graphics system set up again, the archive the preload left
 * unpacked at 0x80118000 hands its eight images to VRAM, and the double
 * ordering table is rebuilt before the menu itself takes over. The persona
 * data screen opens the same way.
 *
 * The menu resets every cursor it owns - the config rows start on the saved
 * options - and runs one of its six pages a frame until the top page closes
 * it, then waits for the closing animation and fades out.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/main/state.h>
#include <persona/common/imageanim.h>
#include <persona/adv/ot.h>
#include <persona/common/menuctx.h>
#include <persona/common/formation.h>
#include <persona/common/bg.h>
#include <persona/adv/personapage.h>

#define PACK_AT        ((u_long *)0x80118000)
#define g_seq_handle   ((short *)0x801F537C)
#define g_party_tactic ((u_char *)0x801F2AE6)

/* The page g_menu_sel names once the menu has been closed. */
#define MENU_CLOSED 0xFF

extern short   g_menu_sel;
extern u_char  g_menu_blink;
extern u_short g_key_menu_close;
extern int     g_pad_held[];
extern int     g_pad_pressed[];
extern u_char  g_BC5C8;
extern u_char  g_BC204;
extern int     g_BB94C;
extern int     g_BB998;
extern int     D_800BB7F8;
extern short   g_stock_last;
extern GsMAP   D_800B8370;
extern GsCELL  g_panel_cells[];
extern short   D_800BB7F4, g_item_top, g_swap_top, D_800BC224, D_800B8458,
               g_item_scroll_step;
extern short   g_arcana_top;
extern short   g_header_scroll_y;
extern short   g_cam_x, g_map_scroll_x, D_800BBC04;
extern short   g_view_dx, g_view_dy, g_view2_dx, g_view2_dy;
extern MenuList D_800BC584;
extern MenuList D_800BC594;
extern MenuList D_800BC5A4;
extern MenuList D_800BC1A4;
extern MenuList D_800BC1B4;
extern MenuList D_800BC1C4;
extern MenuList D_800BC1D4;
extern MenuList D_800BC1E4;
extern MenuList D_800BC1F4;
extern u_char  D_801F2AC4, D_801F2AC5, D_801F2AC9, D_801F2ACA, D_801F2ACB,
               D_801F2ACC, D_801F2ACD;
extern u_char  g_options;
extern u_char  g_pad_config;

extern void VramClearRect(int x, int y, int w, int h);
extern void func_80033A50(int a, int b, int c, int d);
extern void func_80034850(u_long *base);
extern void TimQueueAt(u_long *tim, short x, short y, short cx, short cy);
extern void BgReset(void);
extern void SoundOpenSeq(u_short slot, u_short seq, short vab);
extern void SoundPlaySeq(u_short slot, u_short seq, short vab);
extern void CellsInit(void);
extern void FontCellsInit(void);
extern void PadLoadBindings(u_char config);
extern void PadSetPageButtons(u_char config);
extern void FadeBlackout(void);
extern void ImageAnimStopAll(void);
extern void CheckerMapInit(void);
extern void MenuBuild(void);
extern void FadeUpBlocking(short step, short limit);
extern void FadeDownBlocking(short step, short floor);
extern void SsSetNck(short handle);
extern u_char PartyLastSlot(void);
extern short PersonaStockCompact(void);
extern void RunFrame(void);
extern u_char func_80076F78(void);
extern u_char func_80077614(void);
extern void MenuTick(void);
extern void func_800699C8(void);
extern void StatusMenuStep(void);
extern void ConfigMenuStep(void);
extern void FormationMenuStep(void);
extern void MenuStatusTick(void);

void MainMenu(void);

void MainMenuOpen(void)
{
    u_long *pack = PACK_AT;

    VramClearRect(0, 0, 0x140, 0x1DF);
    SetDispMask(0);
    VSync(0);
    DrawSync(0);
    if (g_state_next == 3) {
        GsInitGraph2(0x140, 0xF0, 0x100, 0, 0);
    } else {
        GsInitGraph(0x140, 0xF0, 0x100, 0, 0);
    }
    func_80033A50(0, 0, 0, 0xF0);

    g_image_queue_count = 0;
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[1]), 0x380, 0x1C8, 0x100, 0x1F8);
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[0]), 0x380, 0x100, 0x3C0, 0x1A0);
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[2]), 0x300, 0x100, 0x3C0, 0x1B0);
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[3]), 0x200, 0x100, 0, 0x1F8);
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[4]), 0x280, 0x100, 0, 0x1F0);
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[5]), 0x300, 0x140, 0x3D0, 0x180);
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[6]), 0x300, 0x1A0, 0x3E0, 0x180);
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[7]), 0x300, 0x130, 0x3F0, 0x1B0);
    BgReset();

    g_ot[0].length = g_ot[1].length = 11;
    g_ot[0].org = (GsOT_TAG *)0x800D6000;
    g_ot[1].org = (GsOT_TAG *)0x800D9000;
    g_ot_index = GsGetActiveBuff();
    func_80034850((u_long *)(0x800C0000 + g_ot_index * 0xB000));
    GsClearOt(0, 0, &g_ot[g_ot_index]);
    MainMenu();
}

void MainMenu(void)
{
    g_menu = (MenuCtx *)0x800ECC80;
    g_BC5C8 = 1;
    g_BB998 = 0;
    g_BC204 = 0;
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
    g_item_top = 0;
    g_swap_top = 0;
    D_800BC224 = 0;
    g_arcana_top = 0;
    D_800B8458 = 0;
    g_item_scroll_step = 0;
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
    MenuListInit(&g_menu->status_page, 0, 0, 2, 0x14);
    MenuListInit(&g_menu->status_who, 0, 0, g_party_last, 0x1A);
    MenuListInit(&g_menu->unk030, 0, 0, 2, 0x2E);
    MenuListInit(&g_menu->unk040, 1, 0, 1, 0x1E);
    MenuListInit(&g_menu->unk050, 0, 0, g_party_last, 0x1A);
    MenuListInit(&g_menu->status_cmd, 0, 0, 2, 0x2E);
    MenuListInit(&g_menu->skill_member, 0, 0, g_party_last, 0x1A);
    MenuListInit(&g_menu->skill_persona, 0, 0, 2, 0x16);
    MenuListInit(&g_menu->skill_spell, 0, 0, 6, 0x1E);
    MenuListInit(&g_menu->status_member, 0, 0, g_party_last, 0x1A);
    MenuListInit(&g_menu->persona_cmd, 0, 0, 1, 0x16);
    MenuListInit(&g_menu->persona_slot, 0, 0, 2, 0x16);
    MenuListInit(&g_menu->page, 0, 0, 1, 0x14);
    MenuListInit(&g_menu->stock, 0, 0, g_stock_last + 1, 0x1E);
    MenuListInit(&g_menu->stock_release, 0, 0, g_stock_last, 0x1E);
    MenuListInit(&g_menu->unk100, 0, 0, 2, 0x2E);
    MenuListInit(&g_menu->arcana_row, 0, 0, 6, 0x14);
    MenuListInit(&g_menu->arcana_col, 0, 0, 1, 0x1A);
    MenuListInit(&g_menu->unk130, 0, 0, 0xA, 0x16);
    MenuListInit(&g_menu->unk140, 0, 0, 1, 0x14);
    MenuListInit(&g_menu->unk150, 0, 0, 1, 0x1E);
    MenuListInit(&g_menu->unk160, 0, 0, 1, 0x1E);
    MenuListInit(&g_menu->unk170, 0, 0, 4, 0x16);
    MenuListInit(&g_menu->unk180, 0, 0, 1, 0x1A);
    MenuListInit(&g_menu->unk190, 0, 0, 1, 0x1A);
    MenuListInit(&g_menu->unk1A0, 0, 0, 1, 0x1E);
    MenuListInit(&g_menu->unk1B0, 0, 0, 4, 0x16);
    MenuListInit(&g_menu->cfg_list, 0, 0, 4, 0x16);
    MenuListInit(&g_menu->cfg_row, 0, 0, 3, 0x16);
    MenuListInit(&g_menu->member_list, 0, 0, g_party_last, 0x16);
    /* Twice over; unk200 onwards follow. */
    MenuListInit(&g_menu->unk1F0, 0, 0, 2, 0x1E);
    MenuListInit(&g_menu->unk1F0, 0, 0, 2, 0x1E);
    MenuListInit(&g_menu->unk200, 0, 0, 5, 0x14);
    MenuListInit(&g_menu->unk210, 0, 0, 1, 0x1A);
    MenuListInit(&g_menu->unk220, 0, 0, 8, 0x16);
    MenuListInit(&g_menu->unk230, 0, 0, 3, 0x14);
    MenuListInit(&g_menu->unk240, 0, 0, 1, 0x1A);
    MenuListInit(&g_menu->unk250, 0, 0, 1, 0x1A);
    MenuListInit(&g_menu->item_row, 0, 0, 0xA, 0x14);
    MenuListInit(&g_menu->item_col, 0, 0, 1, 0x1A);
    MenuListInit(&g_menu->unk280, D_801F2AC4, 0, 1, 0x1A);
    MenuListInit(&g_menu->unk290, D_801F2ACC, 0, 2, 0x1A);
    MenuListInit(&g_menu->unk2A0, D_801F2AC5, 0, 1, 0x1A);
    MenuListInit(&g_menu->unk2B0, D_801F2ACB, 0, 1, 0x1A);
    MenuListInit(&g_menu->unk2C0, g_options, 0, 1, 0x1A);
    MenuListInit(&g_menu->unk2D0, D_801F2AC9, 0, 1, 0x1A);
    MenuListInit(&g_menu->unk2E0, D_801F2ACA, 0, 2, 0x1A);
    MenuListInit(&g_menu->unk2F0, g_party_tactic[5], 0, 1, 0x1A);
    MenuListInit(&g_menu->unk300, g_party_tactic[6], 0, 1, 0x1A);
    MenuListInit(&g_menu->unk310, g_pad_config, 0, 1, 0x1E);
    MenuListInit(&g_menu->formation_cmd, 0, 0, 1, 0x1E);
    MenuListInit(&g_menu->list[2], 0, 0, 0, 6);
    MenuListInit(&g_menu->list[3], 0, 0, 0, 0xA);
    MenuListInit(&D_800BC584, D_801F2ACD, 0, 1, 0x1E);
    MenuListInit(&D_800BC594, 0, 0, 0xB, 0x16);
    MenuListInit(&D_800BC5A4, 0, 0, 1, 0x1A);
    MenuListInit(&D_800BC1A4, 0, 0, 5, 0x16);
    MenuListInit(&D_800BC1B4, g_party_tactic[0], 0, 2, 0x1A);
    MenuListInit(&D_800BC1C4, g_party_tactic[1], 0, 2, 0x1A);
    MenuListInit(&D_800BC1D4, g_party_tactic[2], 0, 2, 0x1A);
    MenuListInit(&D_800BC1E4, g_party_tactic[3], 0, 2, 0x1A);
    MenuListInit(&D_800BC1F4, g_party_tactic[4], 0, 2, 0x1A);
    PadLoadBindings(g_pad_config);
    PadSetPageButtons(g_pad_config);
    SlotClearAll();
    FadeBlackout();
    ImageAnimStopAll();
    CheckerMapInit();
    g_menu_blink = 0x1F;
    MenuBuild();
    SetDispMask(1);
    FadeUpBlocking(8, 0x80);
    SoundPlaySeq(0x18, 5, 1);
opening:
    if (!func_80076F78()) {
        RunFrame();
        goto opening;
    }
    g_menu_blink = 0x20;

    while (g_menu_sel != MENU_CLOSED) {
        RunFrame();
        switch (g_menu_sel) {
        case 0:
            MenuTick();
            break;
        case 1:
            func_800699C8();
            break;
        case 2:
            StatusMenuStep();
            break;
        case 3:
            ConfigMenuStep();
            break;
        case 4:
            FormationMenuStep();
            break;
        case 5:
            MenuStatusTick();
            break;
        }
        if (g_menu_sel == MENU_CLOSED) {
            g_BB94C++;
        }
        if (g_pad_pressed[0] & 0x100) {
            g_BB998 ^= 1;
        }
        if (!g_menu_allow_hold && (g_key_menu_close & g_pad_pressed[0])) {
            g_menu_allow_hold = 1;
            SoundPlaySeq(0x18, 0, 1);
        }
        if (g_BB94C) {
            g_menu_blink = MENU_CLOSED;
        closing:
            if (!func_80077614()) {
                RunFrame();
                goto closing;
            }
            FadeDownBlocking(8, 0);
            g_BC5C8 = 0;
        }
    }
    SsSetNck(g_seq_handle[0x18]);
    SsSetNck(g_seq_handle[0x19]);
    SsSetNck(g_seq_handle[0x1A]);
    SsSetNck(g_seq_handle[0x1B]);
}
