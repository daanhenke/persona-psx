/* Persona 1 (JP) - the persona data screen.  ADV only.
 *   0x80096B30 PersonaDataOpen     0x8009715C PersonaDataScreen
 *   0x800972AC ArcanaGridOpen      0x8009772C ArcanaGridStep
 *   0x80097AE4 PersonaDataPick     0x80097D20 PersonaDataView
 *   0x80098074 PersonaDataLayout   0x80098480 PersonaDataDraw
 *
 * Picking a persona off the list opens its data page: the persona's portrait
 * is read off the disc (AdvResolveSceneLoc kind 4 narrows g_adv_scene_file
 * down to it) into a staging buffer and queued into VRAM, and the page's two
 * halves scroll into view one at a time. Moving the cursor on the open page
 * reads the next persona's portrait the same way.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libcd.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/main/cd.h>
#include <persona/common/menuctx.h>
#include <persona/common/slot.h>
#include <persona/common/tilemap.h>
#include <persona/common/persona.h>
#include <persona/common/bg.h>
#include <persona/adv/personapage.h>

/* The persona ids the list shows, reached by hardcoded address. */
#define g_persona_list ((u_char *)0x800EAE4C)

/* This screen's portraits: kind 4, nine sectors each. */
#define PORTRAIT_KIND    4
#define PORTRAIT_SECTORS 9

/* The arcana grid's arrows hide at the ends of its scroll. */
#define ARCANA_MARKS()                                                        \
    g_slot_cur = &g_slots[PAGE_MARK_SLOT];                                    \
    if (g_arcana_top == 0) {                                                  \
        g_slot_cur->attr |= SLOT_ATTR_HIDE;                                   \
    } else {                                                                  \
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;                                  \
    }                                                                         \
    g_slot_cur = &g_slots[PAGE_MARK_SLOT + 1];                                \
    if (g_arcana_top == ARCANA_TOP_MAX) {                                     \
        g_slot_cur->attr |= SLOT_ATTR_HIDE;                                   \
    } else {                                                                  \
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;                                  \
    }

/* Two arcana a row; the grid scrolls by rows of 12 lines, at most 4 down. */
#define ARCANA_ROW_H   12
#define ARCANA_TOP_MAX 4
#define ARCANA_AT_CURSOR()                                                    \
    (g_arcana_top * 2 + g_menu->arcana_col.cur + g_menu->arcana_row.cur * 2)

extern u_char FlagsCollectGroup(u_char group);
extern void DrawPersonaList(void);

/* The save-game option bytes; the fourth is the pad layout. */
#define g_cfg        ((u_char *)0x801F2AC4)
#define g_seq_handle ((short *)0x801F537C)
#define PACK_AT      ((u_long *)0x80118000)

extern void VramClearRect(int x, int y, int w, int h);
extern void func_80033A50(int a, int b, int c, int d);
extern void func_80034850(u_long *base);
extern void BgReset(void);
extern void SoundOpenSeq(u_short slot, u_short seq, short vab);
extern void CellsInit(void);
extern void FontCellsInit(void);
extern void PadLoadBindings(u_char config);
extern void PadSetPageButtons(u_char config);
extern void FadeBlackout(void);
extern void ImageAnimStopAll(void);
extern void FadeUpBlocking(short step, short limit);
extern void FadeDownBlocking(short step, short floor);
extern void SsSetNck(short handle);

extern int    g_state_next;
extern int    g_image_queue_count;
extern int    g_ot_index;
extern GsOT   g_ot[];
extern int    g_pad_held[];
extern u_char g_BC204, g_BC5C8;
extern int    g_BB94C;
extern GsMAP  g_bg_map0, g_bg_map1, g_bg_map2;
extern short  D_800BB7F4, D_800BB954, D_800BB958, D_800BC224, D_800B8458,
              D_800BC048;
extern short  g_cam_x, g_map_scroll_x;
extern short  g_view_dx, g_view_dy, g_view2_dx, g_view2_dy;
extern u_char PageScrollValue(short *value, short lo, short hi, short step);
extern short MenuScrollCursor(MenuList *m, short *row, short first, short last,
                              u_short *offset);
extern void MenuResetRepeat(MenuList *m);

extern short   g_arcana_top;
extern short   g_arcana_scroll_step;
extern short   g_header_scroll_y;
extern u_char *g_arcana_help_msgs[];
extern u_char  g_fm_prompt_cur_def[];
extern u_char  D_800B1D08[];
extern u_char  D_800B2330[];
extern u_char  D_800B1EF8[];
extern short func_80098B0C(short kind);

extern u_char  D_800B9628[];

extern void   SoundPlaySeq(u_short slot, u_short seq, short vab);

extern u_char  g_persona_list_count;
extern short   g_persona_data_step;
extern int     g_pad_pressed[];
extern int     g_BB998;
extern u_short g_key_menu_close;

void ArcanaGridOpen(void);
void ArcanaGridStep(void);
void PersonaDataPick(void);
void PersonaDataView(void);
void PersonaDataLayout(void);
void PersonaDataDraw(short id);

/* The screen, from the field: the archive the preload left unpacked at
   0x80118000 carries the screen's eight images, which go to VRAM; the three
   character-map layers are set up over the work area's tile maps, the menu
   cursors reset, and the arcana grid opened. The screen runs between a fade
   up and a fade down, and its four sound sequences are closed after it. */
void PersonaDataOpen(void)
{
    u_char  *cfg = g_cfg;
    u_long  *pack = PACK_AT;

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

    SoundOpenSeq(0x18, 0, 0);
    SoundOpenSeq(0x19, 0, 0);
    SoundOpenSeq(0x1A, 0, 0);
    SoundOpenSeq(0x1B, 0, 0);
    g_menu = (MenuCtx *)0x800ECC80;
    g_BB998 = 0;
    g_BC204 = 0;
    g_menu_allow_hold = 0;
    g_BC5C8 = 1;
    CellsInit();
    FontCellsInit();

    g_bg_layer_otz[0] = 0x60;
    g_bg_layer_otz[1] = 0x40;
    g_bg_layer_otz[2] = 0x40;
    g_bg_layer_otz[3] = 0xA0;
    g_bg_layer_otz[4] = 0x20;
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
    g_bg_layers[0].attribute = 0x9000000;
    g_bg_map0.base = (GsCELL *)0x800E864C;
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
    g_bg_layers[2].x = 0;
    g_bg_layers[2].y = 0;
    g_bg_map2.base = (GsCELL *)0x800E3E4C;
    g_bg_map2.cellw = 8;
    g_bg_map2.cellh = 12;
    g_bg_map2.ncellw = MAP_W;
    g_bg_map2.ncellh = 0x20;
    g_bg_map2.index = (u_short *)g_tilemap2;
    g_cam_y = 0;
    g_cam_x = 0;
    g_map_scroll_y = 0;
    g_map_scroll_x = 0;
    g_view_dy = 0;
    g_view_dx = 0;
    g_view2_dy = 0;
    g_view2_dx = 0;

    MenuListInit(&g_menu->arcana_row, 0, 0, 6, 0x14);
    MenuListInit(&g_menu->arcana_col, 0, 0, 1, 0x1A);
    MenuListInit(&g_menu->unk130, 0, 0, 0xA, 0x16);
    MenuListInit(&g_menu->unk140, 0, 0, 1, 0x14);
    MenuListInit(&g_menu->list[2], 0, 0, 0, 6);
    MenuListInit(&g_menu->list[3], 0, 0, 0, 0xA);
    PadLoadBindings(cfg[3]);
    PadSetPageButtons(cfg[3]);
    SlotClearAll();
    FadeBlackout();
    ImageAnimStopAll();
    ArcanaGridOpen();
    FadeUpBlocking(8, 0x80);
    PersonaDataScreen();
    FadeDownBlocking(8, 0);
    SsSetNck(g_seq_handle[0x18]);
    SsSetNck(g_seq_handle[0x19]);
    SsSetNck(g_seq_handle[0x1A]);
    SsSetNck(g_seq_handle[0x1B]);
}

void PersonaDataScreen(void)
{
    g_persona_data_step = 0;
    do {
        RunFrame();
        switch (g_persona_data_step) {
        case 0:
            ArcanaGridOpen();
            g_persona_data_step++;
            break;
        case 1:
            ArcanaGridStep();
            break;
        case 2:
            PersonaDataPick();
            break;
        case 3:
            PersonaDataView();
            break;
        }
        if (g_pad_pressed[0] & 0x100) {
            g_BB998 ^= 1;
        }
        if (!g_menu_allow_hold && (g_key_menu_close & g_pad_pressed[0])) {
            g_menu_allow_hold = 1;
            SoundPlaySeq(0x18, 0, 1);
        }
    } while (g_persona_data_step != 0xFF);
}

/* Opens the arcana grid: two columns of arcana labels in the header layer,
   thirteen rows of them, scrolled so that the cursor's row is in view. The
   persona list beside it is the one arcana the cursor is on, collected from
   the story flags; the help line under it is that arcana's message. */
void ArcanaGridOpen(void)
{
    int i;   /* the row, then the arcana under the cursor */

    func_8008EDBC(0xB);
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap2, 0, MAP_W, 0x20, MAP_W);
    TileMapDrawWindow(AT(g_tilemap0, 2, 1), 0x14, 0xB, MAP_W);
    TileMapDrawBox(AT(g_tilemap0, 3, 2), 0x12, 9, MAP_W);
    TileMapDrawWindow(AT(g_tilemap0, 0, 24), 0xE, 0xD, MAP_W);
    TileMapDrawBox(AT(g_tilemap0, 1, 25), 0xC, 0xB, MAP_W);
    for (i = 0; i < 7; i++) {
        TileMapWriteBar(AT(g_tilemap0, 4 + i, 4), 6);
        TileMapWriteBar(AT(g_tilemap0, 4 + i, 12), 6);
        *AT(g_tilemap0, 4 + i, 10) = 0x17;
        *AT(g_tilemap0, 4 + i, 11) = 0x17;
    }
    for (i = 0; i < 9; i++) {
        TileMapWriteBar(AT(g_tilemap0, 2 + i, 26), 10);
    }
    for (i = 0; i < 13; i++) {
        TileMapWriteRow(&g_arcana_labels[i * 12], AT(g_tilemap2, i, 0), 0, 6);
        TileMapWriteRow(&g_arcana_labels[(i * 2 + 1) * 6], AT(g_tilemap2, i, 8),
                        0, 6);
    }

    i = ARCANA_AT_CURSOR();
    g_persona_list_count = FlagsCollectGroup(i);
    DrawPersonaList();
    SlotClearAll();
    SlotInitTagged(g_fm_prompt_cur_def, 1, 0x42,
                   g_menu->arcana_col.cur * 64 + 0x28,
                   g_menu->arcana_row.cur * 12 + 0x48);
    SlotSetFlicker(1, 1);
    SlotInitTagged(D_800B1D08, 0x3C, 0x300, 0x18, 0x18);
    SlotInitTagged(D_800B2330, 0x2D, 0x2FF, 0, 0x10);
    SlotSetAnim(0x2D, 0, 0, 0, 0x90, 0x18, 0, 0);
    SlotInitTagged(D_800B1EF8, 0x2C, 0x24, 0xE7, 0xE);
    SlotInitTagged(g_pdata_mark_up_def, PAGE_MARK_SLOT, 0x42, 0x58, 0x48);
    SlotInitTagged(g_pdata_mark_down_def, PAGE_MARK_SLOT + 1, 0x42, 0x58, 0x90);

    ARCANA_MARKS();

    g_bg_layers[4].x = 0xE9;
    g_bg_layers[4].y = 0x10;
    g_bg_layers[4].w = 0x30;
    g_bg_layers[4].h = 0x10;
    g_arcana_scroll_step = 0;
    g_header_scroll_y = g_arcana_top * 12;
    g_bg_shown |= 0x10;
    BgMapInit(g_arcana_help_msgs[i], 0);
    MsgStep();
    g_cam_y = 0;
    g_map_scroll_y = 0;
}

/* The arcana grid, a frame: the cursor walks two columns by seven rows, and
   the grid scrolls a row (12 lines) at a time past its seventh. Landing on
   another arcana redraws the persona list and its help line; accepting one
   with any personas in it moves on to the list. */
void ArcanaGridStep(void)
{
    short prev;
    int   arcana;

    prev = ARCANA_AT_CURSOR();
    if ((short)(g_header_scroll_y % ARCANA_ROW_H) == 0) {
        if (g_arcana_scroll_step != 0) {
            if (g_menu->arcana_row.delay < 3) {
                g_menu->arcana_row.delay = 0;
            }
            g_arcana_scroll_step = 0;
        }
        if (PageScrollValue(&g_arcana_top, 0, ARCANA_TOP_MAX, 7)) {
            g_header_scroll_y = g_arcana_top * ARCANA_ROW_H;
        } else if (!MenuScrollCursor(&g_menu->arcana_row, &g_arcana_top, 0,
                                     ARCANA_TOP_MAX,
                                     (u_short *)&g_arcana_scroll_step)) {
            MenuStepCursor(&g_menu->arcana_col);
        }
    } else {
        MenuResetRepeat(&g_menu->arcana_col);
    }
    g_header_scroll_y += g_arcana_scroll_step;
    SlotSetPos(1, 0x42, g_menu->arcana_col.cur * 64 + 0x28,
               g_menu->arcana_row.cur * 12 + 0x48);
    ARCANA_MARKS();

    if (prev != (arcana = ARCANA_AT_CURSOR())) {
        BgMapInit(g_arcana_help_msgs[arcana], 0);
        g_persona_list_count = FlagsCollectGroup(arcana);
        DrawPersonaList();
    }
    MsgStep();
    if (InputCheckAcceptA(1)) {
        if (g_persona_list_count != 0) {
            MenuListInit(&g_menu->list[1], 0, 0, g_persona_list_count - 1,
                         0x16);
            SlotInitTagged(g_pdata_cursor_def, PICK_CURSOR_SLOT, 0x42, 0xD8,
                           g_menu->list[1].cur * 12 + 0x30);
            SlotSetFlicker(1, 0);
            SlotSetFlicker(PICK_CURSOR_SLOT, 1);
            g_persona_data_step++;
        }
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        SlotClear(1);
        g_persona_data_step = 0xFF;
    }
}

void PersonaDataPick(void)
{
    u_char id;

    MenuStepCursor(&g_menu->list[1]);
    SlotSetPos(PICK_CURSOR_SLOT, 0x42, 0xD8, g_menu->list[1].cur * 12 + 0x30);
    MsgStep();
    if (InputCheckAcceptA(1)) {
        MenuListInit(&g_menu->list[0], g_menu->list[1].cur, 0,
                     g_persona_list_count - 1, 0x1A);
        id = g_persona_list[g_menu->list[1].cur];
        AdvResolveSceneLoc(PORTRAIT_KIND, id, 0);
        CdReadFileToAddrAsync(&g_adv_scene_file, PORTRAIT_SECTORS,
                              PORTRAIT_READ);
        while (g_cd_busy != -1) {
            RunFrame();
        }
        TimQueueAt(PORTRAIT_TIM, 0x140, 0x168, 0, 0x1E6);
        MenuListInit(&g_menu->page, 0, 0, 1, 0x14);
        PersonaDataLayout();
        if (g_persona_list_count >= 2) {
            SlotInitTagged(g_pdata_arrow_l_def, ARROW_L_SLOT, 0x23, 0x56, 0xD8);
            SlotInitTagged(g_pdata_arrow_r_def, ARROW_R_SLOT, 0x23, 0x102, 0xD8);
        }
        SlotClear(HINT_SLOT);
        PersonaDataDraw(id);
        DrawPersonaDataStatBars(id);
        g_persona_data_step++;
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        SlotSetFlicker(1, 1);
        SlotClear(PICK_CURSOR_SLOT);
        g_persona_data_step--;
    }
}

void PersonaDataView(void)
{
    int id;   /* the persona, then the page's scroll stop */

    if (!MenuStepCursor(&g_menu->page) && MenuStepCursor(&g_menu->list[0])) {
        id = g_persona_list[g_menu->list[0].cur];
        AdvResolveSceneLoc(PORTRAIT_KIND, id, 0);
        CdReadFileToAddrAsync(&g_adv_scene_file, PORTRAIT_SECTORS,
                              PORTRAIT_READ);
        while (g_cd_busy != -1) {
            RunFrame();
        }
        TimQueueAt(PORTRAIT_TIM, 0x140, 0x168, 0, 0x1E6);
        PersonaDataDraw(id);
        DrawPersonaDataStatBars(id);
    }

    switch (g_menu->page.cur) {
    case 0:
        id = 0;
        break;
    case 1:
        id = PAGE_LOW;
        break;
    }
    if (id < g_cam_y) {
        g_cam_y -= PAGE_STEP;
    }
    if (g_cam_y < id) {
        g_cam_y += PAGE_STEP;
    }
    if (id < g_map_scroll_y) {
        g_map_scroll_y -= PAGE_STEP;
    }
    if (g_map_scroll_y < id) {
        g_map_scroll_y += PAGE_STEP;
    }
    SlotSetPos(PAGE_TOP_SLOT, 0x50, 0x44, 0x40 - g_cam_y);
    SlotSetPos(PAGE_BOTTOM_SLOT, 0x50, 0x48, 0x120 - g_cam_y);

    PAGE_MARKS();

    if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        g_menu->list[1].cur = g_menu->list[0].cur;
        ArcanaGridOpen();
        SlotInitTagged(g_pdata_cursor_def, PICK_CURSOR_SLOT, 2, 0xD8,
                       g_menu->list[1].cur * 12 + 0x30);
        SlotSetFlicker(1, 0);
        SlotSetFlicker(PICK_CURSOR_SLOT, 1);
        g_persona_data_step--;
    }
}

void PersonaDataLayout(void)
{
    int i;   /* the row, then the page's scroll stop */

    func_8008EDBC(9);
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapDrawWindow(AT(g_tilemap0, 1, 0), 0x1E, 0x24, MAP_W);
    TileMapDrawBox(AT(g_tilemap0, 3, 1), 0x1C, 0x20, MAP_W);
    TileMapBlitRle(g_pdata_page_rle, AT(g_tilemap0, 5, 1), MAP_W);
    for (i = 0; i < 5; i++) {
        TileMapWriteBar(AT(g_tilemap0, 11 + i, 18), 10);
        TileMapWriteBar(AT(g_tilemap0, 24 + i, 2), 0x19);
        {
            short *dst = AT(g_tilemap1, 24 + i, 12);

            TileMapWriteRow(&D_800B92A0[0x4C + i * 3], dst, 0, 3);
        }
    }
    TileMapWriteRow(D_800B1898, AT(g_tilemap1, 29, 20), 0x1AE, 4);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 5, 18), 0x36F, 5);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 7, 18), 0x378, 2);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 8, 18), 0x37A, 2);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 10, 20), 0x45D, 4);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 30, 3), 0x3CB, 3);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 31, 3), 0x3D1, 3);
    SlotInitTagged(D_800B1E98, 0x2E, 0x24, 0x54, 0xC6);
    SlotInitTagged(D_800B9628, 0x1D, 0x20, 0x5C, 0xC8);
    SlotClear(1);
    SlotClear(PICK_CURSOR_SLOT);

    switch (g_menu->page.cur) {
    case 0:
        i = 0;
        break;
    case 1:
        i = PAGE_LOW;
        break;
    }
    g_cam_y = i;
    g_map_scroll_y = i;
    SlotSetPos(0, 0x52, 0x50, 0x48 - i);
    SlotSetPos(PAGE_BOTTOM_SLOT, 0x50, 0x48, 0x12C - g_cam_y);
    SlotInitTagged(g_pdata_top_def, PAGE_TOP_SLOT, 0x40, 0x44, 0x40 - g_cam_y);
    SlotInitTagged(g_pdata_bottom_def, PAGE_BOTTOM_SLOT, 0x50, 0x48, 0x120 - g_cam_y);
    SlotSetFlicker(PAGE_BOTTOM_SLOT, 1);
    SlotInitTagged(g_pdata_mark_up_def, PAGE_MARK_SLOT, 0x42, 0x20, 0x36);
    SlotInitTagged(g_pdata_mark_down_def, PAGE_MARK_SLOT + 1, 0x42, 0x20, 0xD0);
    PAGE_MARKS();
}

/* The persona's page: level, HP and SP, the two contact values, the five
   stats, its spells, resistance line, name and arcana. An old-style
   definition: the id is narrowed again on entry although the prototype above
   already has callers pass a short. */
void PersonaDataDraw(id)
    short id;
{
    PersonaData *pd;
    int          i;
    int          row;
    u_short      n;

    TileMapFillRect(AT(g_tilemap1, 5, 24), 0, 2, 1, MAP_W);
    TileMapFillRect(AT(g_tilemap1, 7, 21), 0, 7, 2, MAP_W);
    TileMapFillRect(AT(g_tilemap1, 30, 7), 0, 3, 2, MAP_W);
    TileMapFillRect(AT(g_tilemap1, 24, 16), 0, 2, 5, MAP_W);

    n = FormatDecimal(g_persona_data[id].level, g_hud_digits, 2);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 5, 25), GLYPH_DIGIT0, n);

    *AT(g_tilemap1, 7, 24) = GLYPH_SLASH;
    n = FormatDecimal(g_persona_data[id].hp, g_hud_digits, 3);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 7, 23), GLYPH_DIGIT0, n);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 7, 27), GLYPH_DIGIT0, n);
    *AT(g_tilemap1, 8, 24) = GLYPH_SLASH;
    n = FormatDecimal(g_persona_data[id].sp, g_hud_digits, 3);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 8, 23), GLYPH_DIGIT0, n);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 8, 27), GLYPH_DIGIT0, n);

    n = FormatDecimal(g_persona_data[id].unk0C, g_hud_digits, 3);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 30, 9), GLYPH_DIGIT0, n);
    n = FormatDecimal(g_persona_data[id].unk0E, g_hud_digits, 3);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 31, 9), GLYPH_DIGIT0, n);

    n = FormatDecimal(g_persona_data[id].stat[0], g_hud_digits, 2);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 24, 17), GLYPH_DIGIT0, n);
    n = FormatDecimal(g_persona_data[id].stat[1], g_hud_digits, 2);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 25, 17), GLYPH_DIGIT0, n);
    n = FormatDecimal(g_persona_data[id].stat[2], g_hud_digits, 2);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 26, 17), GLYPH_DIGIT0, n);
    n = FormatDecimal(g_persona_data[id].stat[3], g_hud_digits, 2);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 27, 17), GLYPH_DIGIT0, n);
    n = FormatDecimal(g_persona_data[id].stat[4], g_hud_digits, 2);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 28, 17), GLYPH_DIGIT0, n);

    i = g_arcana_labels[ARCANA_LABELS * ARCANA_LABEL_W - 1 +
                        g_persona_data[id].arcana];
    TileMapWriteRow(&D_800B1A98[i * 10], AT(g_tilemap1, 30, 18), 0, 10);
    i = func_80098B0C(g_persona_data[id].pad36[0]);
    TileMapWriteRow(&D_800B1A98[0x32 + i * 10], AT(g_tilemap1, 31, 18), 0, 10);
    DrawPersonaDataStatBars(id);

    for (i = 0; i < 5; i++) {
        DrawSpellName(g_persona_data[id].spell[i], AT(g_tilemap1, 11 + i, 18),
                      0, 1);
    }

    TileMapFillRect(AT(g_tilemap1, 33, 3), 0, 0x19, 1, MAP_W);
    TileMapWriteRow(&g_resist_labels[g_persona_data[id].resist * 25],
                    AT(g_tilemap1, 33, 3), 0, 0x19);
    CellsClear(g_pdata_name_cells, 10);
    CellsWriteRow(g_pdata_name_cells, g_persona_data[id].name, 0, 10);
    CellsWriteRow(g_pdata_arcana_cells,
                  &g_arcana_labels[(g_persona_data[id].arcana - 1) *
                                   ARCANA_LABEL_W],
                  0, ARCANA_LABEL_W);
}
