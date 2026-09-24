/* Persona 1 (JP) - the persona data screen.  ADV only.
 *   0x8009715C PersonaDataScreen   0x80097AE4 PersonaDataPick
 *   0x80097D20 PersonaDataView     0x80098074 PersonaDataLayout
 *   0x80098480 PersonaDataDraw
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

/* The persona ids the list shows, reached by hardcoded address. */
#define g_persona_list ((u_char *)0x800EAE4C)
#define g_slots        ((Slot *)0x800DC10C)

/* Where a portrait is read to: nine sectors, the archive entry's eight-byte
   header and then the TIM. */
#define PORTRAIT_KIND    4
#define PORTRAIT_SECTORS 9
#define PORTRAIT_READ    ((u_long *)0x800F4000)
#define PORTRAIT_TIM     ((u_long *)0x800F4008)

/* The sprites. */
#define PICK_CURSOR_SLOT 2
#define ARROW_L_SLOT     0x22
#define ARROW_R_SLOT     0x23
#define PAGE_TOP_SLOT    0x1E
#define PAGE_BOTTOM_SLOT 0x1F
#define PAGE_MARK_SLOT   32
#define HINT_SLOT        0x2C

/* The page scrolls 8 lines a frame to one of two stops. */
#define PAGE_STEP 8
#define PAGE_LOW  0xE0

/* The two page marks hide at the end they point past. */
#define PAGE_MARKS()                                                              g_slot_cur = &g_slots[PAGE_MARK_SLOT];                                        if (g_cam_y == 0) {                                                               g_slot_cur->attr |= SLOT_ATTR_HIDE;                                       } else {                                                                          g_slot_cur->attr &= ~SLOT_ATTR_HIDE;                                      }                                                                             g_slot_cur++;                                                                 if (g_cam_y == PAGE_LOW) {                                                        g_slot_cur->attr |= SLOT_ATTR_HIDE;                                       } else {                                                                          g_slot_cur->attr &= ~SLOT_ATTR_HIDE;                                      }

#define g_tilemap0 ((short *)0x800EE180)
#define AT(map, row, col) (&(map)[(row) * MAP_W + (col)])

extern void TileMapDrawWindow(short *dst, u_char w, u_char h, u_char stride);
extern void TileMapDrawBox(short *dst, u_short w, short h, u_short stride);
extern void TileMapBlitRle(const u_short *src, short *dst, u_short stride);
extern void TileMapWriteBar(short *dst, u_char width);
extern void func_8008EDBC(int);
extern short func_80098B0C(short kind);
extern void DrawSpellName(short spell, short *dst, u_short base, short rule);
extern void CellsClear(GsCELL *dst, u_char count);
extern void CellsWriteRow(GsCELL *dst, const u_char *src, u_char page,
                          u_short count);

#define GLYPH_SLASH 0xCA

extern u_char D_800B1A98[];
extern u_char g_resist_labels[];
extern GsCELL g_pdata_name_cells[];
extern GsCELL g_pdata_arcana_cells[];

extern u_short g_pdata_page_rle[];
extern u_char  D_800B92A0[];
extern u_char  D_800B1898[];
extern u_char  D_800B1E98[];
extern u_char  D_800B9628[];
extern u_char  g_pdata_top_def[];
extern u_char  g_pdata_bottom_def[];
extern u_char  g_pdata_mark_up_def[];
extern u_char  g_pdata_mark_down_def[];

extern u_char InputCheckAcceptA(u_char repeat);
extern u_char InputCheckAcceptB(u_char repeat);
extern int    MsgStep(void);
extern void   RunFrame(void);
extern void   TimQueueAt(u_long *tim, short x, short y, short cx, short cy);
extern void   AdvResolveSceneLoc(short kind, int index, void *unused);
extern void   DrawPersonaDataStatBars(short id);
extern void   SoundPlaySeq(u_short slot, u_short seq, short vab);

extern CdlFILE g_adv_scene_file;
extern u_char  g_persona_list_count;
extern short   g_persona_data_step;
extern short   g_cam_y;
extern short   g_map_scroll_y;
extern Slot   *g_slot_cur;
extern int     g_pad_pressed[];
extern int     g_BB998;
extern u_short g_key_menu_close;

extern u_char g_pdata_arrow_l_def[];
extern u_char g_pdata_arrow_r_def[];
extern u_char g_pdata_cursor_def[];

void func_800972AC(void);
void func_8009772C(void);
void PersonaDataPick(void);
void PersonaDataView(void);
void PersonaDataLayout(void);
void PersonaDataDraw(short id);

INCLUDE_ASM("adv/nonmatchings/ui/personadata", func_80096B30);

void PersonaDataScreen(void)
{
    g_persona_data_step = 0;
    do {
        RunFrame();
        switch (g_persona_data_step) {
        case 0:
            func_800972AC();
            g_persona_data_step++;
            break;
        case 1:
            func_8009772C();
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

INCLUDE_ASM("adv/nonmatchings/ui/personadata", func_800972AC);

INCLUDE_ASM("adv/nonmatchings/ui/personadata", func_8009772C);

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
        func_800972AC();
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
