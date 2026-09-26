/* Persona 1 (JP) - the "other" and config menus.  DNG only.
 * DNG's copy of src/adv/ui/configmenu.c.
 *   0x800834B0 ConfigMenuStep   0x80083594 ConfigMenuOpen
 *   0x80083670 MenuOtherStep    0x800837BC ConfigListStep
 *   0x800840C0 ConfigOptionStep 0x800841B0 ConfigPadStep
 *
 * The top menu's last command opens a short list: the field map (when there
 * is one), the persona data screen, and the config pages. The config pages
 * run as steps of their own.
 */
#define SLOT_SETPOS_INT
#define SLOT_TAGGED_INTXY
#define TILEMAP_INT_COUNT
#define PERSONAPAGE_DNG
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/menuctx.h>
#include <persona/common/bg.h>
#include <persona/adv/personapage.h>
#include <persona/common/cel.h>

/* The pad layout's byte in the save-game options. */
#define g_pad_layout ((u_char *)0x801F2AC7)

#define OTHER_MAP     0
#define OTHER_PERSONA 1
#define OTHER_CONFIG  2

extern short   g_menu_subsel;
extern short   g_menu_sel;
extern u_char  g_menu_blink;
extern int     g_state_next;
extern int     g_pad_pressed[];
extern u_short D_80100066;

extern void DrawStatusHud(void);
extern void PersonaDataScreen(void);
extern void SoundPlaySeq(u_short slot, u_short seq, short vab);
/* Both take the layout unmasked in DNG. */
extern void PadDrawLayout(int layout);
extern void PadLoadBindings(u_char config);
extern void PadSetPageButtons(u_char config);
extern void DrawPlaceLabel(short in_battle);
extern void func_80093D18(int n);
extern void ConfigStepTactics(void);
extern void ConfigStepChoice(void);
extern void ConfigCloseChoice(void);
extern void func_8008546C(void);
extern void func_80086B08(int a, int b);
extern void func_800891A8(int a, int b);
extern void func_80086E20(int a, int b);
extern void MapScreen(void);
extern void ConfigPageOpen(void);
extern void ConfigStepRows(void);

void ConfigMenuOpen(void);
void MenuOtherStep(void);
void ConfigListStep(void);

/* The tactics byte of the save-game options: the row in its upper bits, the
   column in bit 0. Read by address here, by name below. */
#define TACTICS ((u_char *)0x801F2AC6)
/* A character-map layer as rows; the tactics page indexes it this way. */
#define MAP2D(map) ((short (*)[MAP_W])(map))
extern u_char D_801F2AC6;
extern u_char g_pad_title[];
extern u_char g_tactics_title[];
extern u_char D_8009A4D0[];
extern u_char D_8009AA4C[];
extern u_char D_8009B074[];
extern u_char D_8009B7DC[];
extern u_char g_fm_prompt_cur_def[];
extern void   ConfigListBeginEdit(void);
extern void   ConfigListApplyOption(void);
extern void   ConfigListPlaceMarkers(void);
extern void   ConfigRedrawBattlePage(void);
void ConfigOptionStep(void);
void ConfigPadStep(void);

void ConfigMenuStep(void)
{
    switch (g_menu_subsel) {
    case 0:
        ConfigMenuOpen();
        g_menu_subsel++;
        break;
    case 1:
        MenuOtherStep();
        break;
    case 2:
        ConfigListStep();
        break;
    case 3:
        ConfigOptionStep();
        break;
    case 4:
        ConfigPadStep();
        break;
    case 5:
        ConfigStepRows();
        break;
    case 6:
        ConfigStepTactics();
        break;
    case 7:
        ConfigStepChoice();
        break;
    case 8:
        ConfigCloseChoice();
        break;
    }
}

void ConfigMenuOpen(void)
{
    func_8008546C();
    SlotSetAnim(0x2D, 0, 0, 0, 0x30, 0x18, 0, 0);
    g_bg_layer_otz[2] = 0x40;
    g_bg_map0.ncellh = 0x40;
    g_bg_map1.ncellh = 0x40;
    g_bg_map0.ncellw = MAP_W;
    g_bg_map1.cellw = 8;
    g_bg_map1.cellh = 12;
    g_bg_layers[2].attribute = 0x8000000;
    g_bg_map1.ncellw = MAP_W;
    g_bg_map2.cellw = 8;
    g_bg_map2.cellh = 12;
    g_bg_map2.ncellw = MAP_W;
    g_bg_map2.ncellh = 0x20;
    func_80086B08(2, 3);
    func_800891A8(2, 0x10);
}

void MenuOtherStep(void)
{
    DrawStatusHud();
    if (MenuStepCursor(&g_menu->unk100)) {
        func_80086E20(2, 0x10);
    }
    if (InputCheckAcceptA(2)) {
        switch (g_menu->unk100.cur) {
        case OTHER_MAP:
            if (g_state_next != 2 &&
                (g_state_next != 3 || D_80100066 != 0xFFFF)) {
                MapScreen();
            }
            g_menu_subsel = 0;
            break;
        case OTHER_PERSONA:
            PersonaDataScreen();
            g_menu_subsel = 0;
            break;
        case OTHER_CONFIG:
            ConfigPageOpen();
            g_menu_subsel++;
            break;
        }
    } else if (InputCheckAcceptB(2) || g_menu_allow_hold) {
        g_menu_blink = 0xFF;
        g_menu_sel = 0;
        g_menu_subsel = 0;
    }
}

/* The config list, a frame: the cursor walks the rows and each row's option
   is edited in place. Accepting the pad row lays out the pad page, the
   tactics row the tactics page, and the battle row redraws the battle page;
   each then runs as a step of its own. */
void ConfigListStep(void)
{
    u_char *tactics;
    int     i;
    int     y;

    if (MenuStepCursor(&g_menu->cfg_list)) {
        SlotSetPos(1, 0x42, 0x48, g_menu->cfg_list.cur * 24 + 0x48);
        ConfigListBeginEdit();
    } else if (MenuStepCursor(&g_menu->list[1])) {
        ConfigListApplyOption();
    }
    ConfigListPlaceMarkers();
    if (InputCheckAcceptA(1)) {
        switch (g_menu->cfg_list.cur) {
        case 2:
            func_80092E5C(0x10);
            TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
            TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
            TileMapDrawWindow(AT(g_tilemap0, 0, 9), 0x12, 4, MAP_W);
            TileMapDrawBox(AT(g_tilemap0, 1, 10), 0x10, 2, MAP_W);
            TileMapDrawWindow(AT(g_tilemap0, 4, 0), 0x24, 0xF, MAP_W);
            TileMapDrawBox(AT(g_tilemap0, 5, 1), 0x22, 0xD, MAP_W);
            TileMapWriteRow(g_pad_title, AT(g_tilemap1, 0, 11), 0, 10);
            TileMapWriteRow(D_8009A4D0, AT(g_tilemap1, 1, 9), 0, 4);
            TileMapWriteRow(D_8009A4D0, AT(g_tilemap1, 1, 17), 0, 4);
            for (i = 0; i < 4; i++) {
                *AT(g_tilemap1, 6 + i, 0) = 0x434 + i * 2;
                *AT(g_tilemap1, 6 + i, 1) = 0x435 + i * 2;
            }
            *AT(g_tilemap1, 10, 0) = 0x43C;
            *AT(g_tilemap1, 10, 1) = 0x43D;
            *AT(g_tilemap1, 10, 2) = 0x43E;
            *AT(g_tilemap1, 11, 0) = 0x442;
            *AT(g_tilemap1, 11, 1) = 0x443;
            *AT(g_tilemap1, 11, 2) = 0x444;
            *AT(g_tilemap1, 12, 0) = 0x43F;
            *AT(g_tilemap1, 12, 1) = 0x440;
            *AT(g_tilemap1, 12, 2) = 0x441;
            *AT(g_tilemap1, 13, 0) = 0x445;
            *AT(g_tilemap1, 13, 1) = 0x446;
            *AT(g_tilemap1, 13, 2) = 0x447;
            for (i = 0; i < 2; i++) {
                *AT(g_tilemap1, 14 + i, 0) = 0x428 + i * 6;
                *AT(g_tilemap1, 14 + i, 1) = 0x429 + i * 6;
                *AT(g_tilemap1, 14 + i, 2) = 0x42A + i * 6;
                *AT(g_tilemap1, 14 + i, 3) = 0x42B + i * 6;
                *AT(g_tilemap1, 14 + i, 4) = 0x42C + i * 6;
                *AT(g_tilemap1, 14 + i, 5) = 0x42D + i * 6;
            }
            *AT(g_tilemap1, 1, 14) = 0xC1;
            *AT(g_tilemap1, 1, 22) = 0xC2;
            MenuListInit(&g_menu->list[1], *g_pad_layout, 0, 1, 0x1A);
            MenuListInit(&g_menu->list[0], 0, 0, 1, 0x14);
            DrawPlaceLabel(g_menu->list[0].cur);
            PadDrawLayout(g_menu->list[0].cur * 2 + g_menu->list[1].cur);
            SlotInitTagged(g_fm_prompt_cur_def, 1, 0x42,
                           g_menu->list[1].cur * 64 + 0x68, 0x24);
            SlotSetFlicker(1, 1);
            SlotClear(2);
            SlotClear(3);
            SlotClear(4);
            SlotClear(5);
            SlotInitTagged(g_pdata_mark_up_def, PAGE_MARK_SLOT, 0x42, 0x110,
                           0x44);
            SlotInitTagged(g_pdata_mark_down_def, PAGE_MARK_SLOT + 1, 0x42,
                           0x110, 0xDE);
            g_slot_cur = &g_slots[PAGE_MARK_SLOT];
            if (g_menu->list[0].cur == 0) {
                g_slot_cur->attr |= SLOT_ATTR_HIDE;
            } else {
                g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
            }
            g_slot_cur++;
            if (g_menu->list[0].cur != 0) {
                g_slot_cur->attr |= SLOT_ATTR_HIDE;
            } else {
                g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
            }
            g_menu_subsel += 2;
            break;
        case 3:
            func_80092E5C(0xF);
            TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
            TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
            TileMapDrawWindow(AT(g_tilemap0, 0, 5), 0x1B, 9, MAP_W);
            TileMapDrawBox(AT(g_tilemap0, 1, 6), 0x19, 7, MAP_W);
            TileMapDrawWindow(AT(g_tilemap0, 10, 0), 0x25, 7, MAP_W);
            TileMapDrawBox(AT(g_tilemap0, 11, 1), 0x23, 5, MAP_W);
            for (i = 0; i < 4; i++) {
                TileMapWriteBar(AT(g_tilemap0, 3 + i, 8), 10);
                TileMapWriteBar(AT(g_tilemap0, 3 + i, 19), 10);
            }
            TileMapWriteRow(g_tactics_title, AT(g_tilemap1, 0, 1), 0, 10);
            for (i = 0; i < 8; i++) {
                y = i / 2 + 2;
                TileMapWriteRow(D_8009A4D0,
                                &MAP2D(g_tilemap1)[y][i % 2 * 11 + 4], 0, 4);
                FormatDecimal(i + 1, g_hud_digits, 1);
                TileMapWriteRowRev(g_hud_digits,
                                   &MAP2D(g_tilemap1)[y][i % 2 * 11 + 9],
                                   GLYPH_DIGIT0, 1);
            }
            tactics = TACTICS;
            MenuListInit(&g_menu->list[0], *tactics >> 1, 0, 3, 0x16);
            MenuListInit(&g_menu->list[1], *tactics & 1, 0, 1, 0x1A);
            SlotClearAll();
            SlotInitTagged(D_8009AA4C, 0x3C, 0x300, 0x18, 0x18);
            SlotInitTagged(D_8009B074, 0x2D, 0x2FF, 0, 0x10);
            SlotSetAnim(0x2D, 0, 0, 0, 0, 0x24, 0, 0);
            SlotInitTagged(g_pdata_cursor_def, 1, 0x42, 0, 0);
            SlotSetPos(1, 0x42, g_menu->list[1].cur * 88 + 0x50,
                       g_menu->list[0].cur * 12 + 0x3C);
            SlotSetFlicker(1, 1);
            SlotInitTagged(D_8009B7DC, 2, 8, 0x14, 0x96);
            for (i = 0; i < 8; i++) {
                g_cinema_cels0.h = 0x38;
            }
            g_cinema_cels6.h = 0x38;
            func_80093D18(D_801F2AC6);
            g_menu_subsel++;
            break;
        case 4:
            ConfigRedrawBattlePage();
            g_menu_subsel += 3;
            break;
        }
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        g_menu_subsel = 0;
    }
}

/* The option grid, a frame: two columns of on/off pairs. */
void ConfigOptionStep(void)
{
    if (MenuStepCursor(&g_menu->list[0]) || MenuStepCursor(&g_menu->list[1])) {
        SlotSetPos(1, 0x42, g_menu->list[1].cur * 88 + 0x50,
                   g_menu->list[0].cur * 12 + 0x3C);
        func_80093D18(g_menu->list[1].cur + g_menu->list[0].cur * 2);
    }
    if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        ConfigPageOpen();
        g_menu_subsel--;
    }
}

/* The pad layouts, a frame: the page and the layout on it. Moving to a layout
   takes it at once. */
void ConfigPadStep(void)
{
    u_char *layout;

    if (MenuStepCursor(&g_menu->list[1])) {
        SlotSetPos(1, 0x42, g_menu->list[1].cur * 64 + 0x68, 0x24);
        PadDrawLayout(g_menu->list[1].cur + g_menu->list[0].cur * 2);
        layout = g_pad_layout;
        *layout = g_menu->list[1].cur;
        PadLoadBindings(*layout);
        PadSetPageButtons(*layout);
    } else if (MenuStepCursor(&g_menu->list[0])) {
        DrawPlaceLabel(g_menu->list[0].cur);
        PadDrawLayout(g_menu->list[1].cur + g_menu->list[0].cur * 2);
    }
    /* The page arrows: one page each way. */
    g_slot_cur = &g_slots[PAGE_MARK_SLOT];
    if (g_menu->list[0].cur == 0) {
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
    } else {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }
    g_slot_cur++;
    if (g_menu->list[0].cur != 0) {
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
    } else {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }
    if (*(u_char *)g_pad_pressed || g_menu_allow_hold) {
        SoundPlaySeq(0x18, 0, 1);
        ConfigPageOpen();
        g_menu_subsel -= 2;
    }
}
