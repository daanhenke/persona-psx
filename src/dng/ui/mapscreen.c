/* Persona 1 (JP) - the automap screen.  DNG only.
 *   0x80095578 MapScreenOpen     0x80095B84 MapScreen
 *   0x80095C70 MapFindPlayer     0x80095D7C MapScreenLayout
 *
 * The field's copy of ADV's automap screen (src/adv/ui/mapscreen.c). The map
 * opens from the field like the other full screens: the archive the preload
 * left unpacked at 0x80180000 hands its eight images to VRAM, the ordering
 * table is rebuilt and the character-map layers laid out again.
 *
 * Where the player stands comes from the field's own record when the map is
 * opened from a room, and from the scene pack's header (read to 0x80100000)
 * when it is opened from a scene. The map can be turned to face the way the
 * player does, unless the "north up" option is set; the cursor lists start on
 * the player's tile in that frame.
 */
#define SLOT_SETPOS_INT
#define SLOT_TAGGED_INTXY
#define TILEMAP_INT_COUNT
#define PERSONAPAGE_DNG
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/main/state.h>
#include <persona/common/imageanim.h>
#include <persona/adv/ot.h>
#include <persona/common/menuctx.h>
#include <persona/common/bg.h>
#include <persona/adv/personapage.h>

#define PACK_AT      ((u_long *)0x80180000)
#define g_cfg        ((u_char *)0x801F2AC4)
#define g_seq_handle ((short *)0x801F537C)

/* The map screen's step once it has been closed. */
#define MAP_CLOSED 0xFF

/* The map's turn: a cursor of its own, in the slot the formation grid's
   column uses on its screen. */
#define MAP_TURN grid[1]

/* The field's record of where the player is. */
extern u_short D_801F15BE;
extern u_short D_801F15C0;
/* By address: sym.dng.txt ignores it, as the field's entry reads it through
   g_dng (field.h), which cse folds to the fixed address. */
#define D_801F15A4 (*(u_char *)0x801F15A4)
extern u_char  D_801F15A6;
extern u_char  D_801F15A7;

/* And the scene pack's. */
extern u_char  D_80100066;
extern u_short D_80100068;
extern u_char  D_8010006A;
extern u_char  D_8010006B;

extern short   D_8009FDF8;
extern short   D_8009FDFC;
extern short   D_8009FE00;
extern short   D_8009FE08;
extern short   D_8009FE0C;
extern short   D_8009FE88;

extern u_char  g_map_north_up;
extern short   g_persona_data_step;
extern u_char  g_menu_allow_hold;
extern u_short g_key_menu_close;
extern int     g_pad_held[];
extern int     g_pad_pressed[];
extern u_char  D_800A083C;
extern u_char  D_800A04B8;
extern int     D_8009FB3C;
extern int     g_select_toggle;
extern short   g_header_scroll_y;
extern short   g_cam_x, g_map_scroll_x, D_8009FE78;
extern short   g_view_dx, g_view_dy, g_view2_dx, g_view2_dy;
extern u_char  D_8009AA4C[];
extern u_char  g_map_title_def[];
extern u_char  g_map_arrow_defs[4][0x10];   /* up, down, left, right */
extern u_char  D_8009B074[];

extern void VramClearRect(int x, int y, int w, int h);
extern void BgReset(void);
extern void FlushImageUploads(void);
extern void SoundOpenSeq(u_short slot, u_short seq, short vab);
extern void SoundPlaySeq(u_short slot, u_short seq, short vab);
extern void CellsInit(void);
extern void FontCellsInit(void);
extern void PadLoadBindings(u_char config);
extern void PadSetPageButtons(u_char config);
extern void FadeBlackout(void);
extern void ImageAnimStopAll(void);
extern void FadeUpBlocking(short step, short limit);
extern void FadeDownBlocking(short step, short floor);
extern void SsSetNck(short handle);
extern void MapDrawMarkers(void);
extern void MapDrawName(short map);
extern void DrawCompass(short facing);
/* The call here passes a fifth argument, 0, which the routine never reads;
   this unit was built against a declaration that had one. */
extern void MapPlaceMarker(short map_dir, short player_dir, short x, short y,
                           int unused);
extern void RoomRotatePoint(short from, short x, short y, short to,
                            short *ox, short *oy);
extern void func_80096B4C(short map, short turn);
extern void func_80096818(void);
extern void func_80092E5C(int);
/* The field's message stepper. */
extern int  func_80076380(void);

void MapScreen(void);
void MapFindPlayer(void);
void MapScreenLayout(void);

void MapScreenOpen(void)
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
    GsDefDispBuff(0, 0, 0, 0xF0);

    g_image_queue_count = 0;
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[1]), 0x380, 0x1C8, 0, 0x1FC);
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[0]), 0x380, 0x100, 0x3C0, 0x1A0);
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[2]), 0x300, 0x100, 0x3C0, 0x1B0);
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[3]), 0x200, 0x100, 0, 0x1F8);
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[4]), 0x280, 0x100, 0, 0x1F0);
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[5]), 0x300, 0x140, 0x3D0, 0x180);
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[6]), 0x300, 0x1A0, 0x3E0, 0x180);
    TimQueueAt((u_long *)((u_char *)PACK_AT + pack[7]), 0x300, 0x130, 0x3F0, 0x1B0);
    BgReset();
    FlushImageUploads();
    DrawSync(0);

    g_ot[0].length = g_ot[1].length = 11;
    g_ot[0].org = (GsOT_TAG *)0x800D6000;
    g_ot[1].org = (GsOT_TAG *)0x800D9000;
    g_ot_index = GsGetActiveBuff();
    GsSetWorkBase((PACKET *)(0x800C0000 + g_ot_index * 0xB000));
    GsClearOt(0, 0, &g_ot[g_ot_index]);

    SoundOpenSeq(0x18, 0, 0);
    SoundOpenSeq(0x19, 0, 0);
    SoundOpenSeq(0x1A, 0, 0);
    SoundOpenSeq(0x1B, 0, 0);
    g_menu = (MenuCtx *)0x800ECC80;
    g_select_toggle = 0;
    D_800A04B8 = 0;
    g_menu_allow_hold = 0;
    D_800A083C = 1;
    CellsInit();
    FontCellsInit();

    g_bg_layer_otz[0] = 0x60;
    g_bg_layer_otz[1] = 0x40;
    g_bg_layer_otz[2] = 0x40;
    g_bg_layer_otz[3] = 0xA0;
    g_bg_layer_otz[4] = 0x20;
    D_8009FB3C = 0;
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
    MapFindPlayer();
    MapScreenLayout();
    MapDrawMarkers();
    FadeUpBlocking(8, 0x80);
    MapScreen();
    FadeDownBlocking(8, 0);
    SsSetNck(g_seq_handle[0x18]);
    SsSetNck(g_seq_handle[0x19]);
    SsSetNck(g_seq_handle[0x1A]);
    SsSetNck(g_seq_handle[0x1B]);
}

/* Ten frames of the message box first, then the map's own step until it is
   closed. */
void MapScreen(void)
{
    int i;

    MapFindPlayer();
    MapScreenLayout();
    MapDrawMarkers();
    g_persona_data_step = 0;
    for (i = 0; i < 10; i++) {
        func_80076380();
        RunFrame();
    }
    while (g_persona_data_step != MAP_CLOSED) {
        RunFrame();
        if (g_persona_data_step == 0) {
            func_80096818();
        }
        if (!g_menu_allow_hold && (g_key_menu_close & g_pad_pressed[0])) {
            g_menu_allow_hold = 1;
            SoundPlaySeq(0x18, 0, 1);
        }
    }
}

void MapFindPlayer(void)
{
    switch (g_state_next) {
    case 0:
        D_8009FDF8 = D_801F15BE;
        D_8009FE00 = D_801F15BE;
        D_8009FE88 = D_801F15C0;
        D_8009FE08 = D_801F15A4;
        D_8009FE0C = D_801F15A6;
        D_8009FDFC = D_801F15A7;
        break;
    case 3:
        D_8009FDF8 = D_80100066;
        D_8009FE00 = D_80100068 >> 8;
        D_8009FE88 = D_80100068 & 0xFF;
        D_8009FE08 = D_8010006A;
        D_8009FE0C = D_8010006B;
        if (g_state_prev == 0) {
            D_8009FDFC = D_801F15A7;
        } else {
            D_8009FDFC = 0;
        }
        break;
    }
}

void MapScreenLayout(void)
{
    int   turn;
    int   sy;
    int   sx;
    short x;
    short y;

    g_bg_layer_otz[1] = 0x40;
    g_bg_layer_otz[2] = 0x41;
    g_bg_map0.ncellw = MAP_W;
    g_bg_map0.index = (u_short *)g_tilemap0;
    g_bg_map0.ncellh = 0xF;
    g_bg_map1.cellw = 0x10;
    g_bg_map1.cellh = 0x10;
    g_bg_map1.ncellw = 0x2A;
    g_bg_map1.ncellh = 0x22;
    g_bg_map1.index = (u_short *)g_tilemap1;
    g_bg_layers[2].attribute = 0x9000000;
    g_bg_layers[2].x = 0;
    g_bg_layers[2].y = 0;
    g_bg_map2.base = (GsCELL *)0x800E3E4C;
    g_bg_map2.cellw = 0x10;
    g_bg_map2.cellh = 0x10;
    g_bg_map2.ncellw = 0x2A;
    g_bg_map2.ncellh = 0x22;
    g_bg_map2.index = (u_short *)g_tilemap2;
    g_cam_y = 0;
    g_cam_x = 0;
    SlotClearAll();
    g_view_dy = 0;
    g_view_dx = 0;
    g_view2_dy = 0;
    g_view2_dx = 0;
    g_menu_allow_hold = 0;
    func_80092E5C(0x13);
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0x3DC, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap2, 0x3DC, MAP_W, 0x20, MAP_W);
    TileMapDrawWindow(g_tilemap0, 0x22, 0xF, MAP_W);
    TileMapDrawBox(g_tilemap0 + MAP_W + 1, 0x20, 0xD, MAP_W);
    SlotInitTagged(g_map_arrow_defs[0], 0x20, 0x24, 0xA8, 0x2A);
    SlotInitTagged(g_map_arrow_defs[1], 0x21, 0x24, 0xA8, 0xEA);
    SlotInitTagged(g_map_arrow_defs[2], 0x22, 0x24, 0x18, 0x90);
    SlotInitTagged(g_map_arrow_defs[3], 0x23, 0x24, 0x138, 0x90);
    SlotInitTagged(D_8009AA4C, 0x3C, 0x300, 0x18, 0x18);
    SlotInitTagged(g_map_title_def, 0x2E, 0x24, 0x58, 0x10);
    SlotInitTagged(D_8009B074, 0x2D, 0x2FF, 0, 0x10);
    SlotSetAnim(0x2D, 0, 0, 0, 0x60, 0x18, 0, 0);

    if (g_map_north_up) {
        turn = 0;
    } else {
        turn = -D_8009FDFC & 3;
    }
    MenuListInit(&g_menu->MAP_TURN, turn, 0, 3, 0x10);
    RoomRotatePoint(0, D_8009FE08, D_8009FE0C, turn, &x, &y);
    MenuListInit(&g_menu->list[1], x, 0, 0x17, 0x18);
    MenuListInit(&g_menu->list[0], y, 0, 0x17, 0x14);
    func_80096B4C(D_8009FE88, g_menu->MAP_TURN.cur);
    MapDrawMarkers();
    DrawCompass(g_menu->MAP_TURN.cur);
    MapPlaceMarker(g_menu->MAP_TURN.cur, D_8009FDFC, D_8009FE08, D_8009FE0C,
                   0);
    func_80096B4C(D_8009FE88, g_menu->MAP_TURN.cur);
    MapDrawName(D_8009FE88);
    sy = g_menu->list[0].cur << 4;
    sx = g_menu->list[1].cur << 4;
    g_map_scroll_y = g_header_scroll_y = sy;
    g_map_scroll_x = D_8009FE78 = sx;
}
