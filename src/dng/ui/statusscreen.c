/* Persona 1 (JP) - the status menu's member page.  DNG only.
 *   0x80080CA0 MenuStatusOpen   0x80080E24 MenuStatusSelect
 *   0x80081728 MenuStatusView
 *
 * The field's copy of ADV's member page (src/adv/ui/statusscreen.c), built
 * against the field's own message stepper. Picking a member opens their
 * page: the member's face is read off the disc (AdvResolveSceneLoc kind 3,
 * five sectors streamed to the staging buffer while the frame loop keeps
 * running) and queued into VRAM with one palette entry patched, the page is
 * laid out over the two character-map layers, and a message names the
 * member. The page has three stops the cursor scrolls between, 8 lines a
 * frame.
 */
#define SLOT_SETPOS_INT
#define SLOT_TAGGED_INTXY
#define TILEMAP_INT_COUNT
#define PERSONAPAGE_DNG
#include <decomp/types.h>
#include <libcd.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/main/cd.h>
#include <persona/common/menuctx.h>
#include <persona/common/char.h>
#include <persona/common/formation.h>
#include <persona/common/bg.h>
#include <persona/adv/personapage.h>

#define FACE_KIND    3
#define FACE_SECTORS 5

/* The face TIM's palette entry the page overrides, reached by address. */
#define g_face_clut_fix (*(u_short *)0x800F401C)
#define FACE_CLUT_FIX   0x8C63

/* The page's three stops. */
#define STATUS_MID 0x70
#define STATUS_LOW 0xB8

#define MEMBER_CURSOR_SLOT 1

/* The page marks, hidden at the top and bottom stops. */
#define STATUS_MARKS()                                                        \
    g_slot_cur = &g_slots[PAGE_MARK_SLOT];                                    \
    if (g_cam_y == 0) {                                                       \
        g_slot_cur->attr |= SLOT_ATTR_HIDE;                                   \
    } else {                                                                  \
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;                                  \
    }                                                                         \
    g_slot_cur++;                                                             \
    if (g_cam_y == STATUS_LOW) {                                              \
        g_slot_cur->attr |= SLOT_ATTR_HIDE;                                   \
    } else {                                                                  \
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;                                  \
    }

extern short   g_menu_subsel;
extern short   g_menu_sel;
extern u_char  g_menu_blink;
extern u_char  g_party_last;
extern u_char  g_fm_mark_def[];
extern short   g_fm_mark_pos[][2];
extern u_char  g_grow_slot_def[];
extern u_char  D_8009AA4C[];
extern u_char  D_8009B074[];
extern u_char  D_8009ABDC[];
extern u_char  D_8009B330[];
extern u_char  D_8005E714[];

/* The member message's two copies of the member's key, and each key's face
   file. */
extern u_char  g_char_msg_key[];
extern u_char  g_char_msg_key2;
extern u_char  g_char_face_files[];

extern void  MenuScreenDraw(void);
extern short MenuStepMember(int *sel, u_char last);
extern void  StatusDrawMain(int slot);
extern void  DrawStatusHud(void);
extern void  BgMapInit(void *script, short speed);
extern void  func_80092E5C(int);
/* The field's message stepper. */
extern int   func_80076380(void);

void MenuStatusOpen(void)
{
    func_80092E5C(0);
    SlotClearAll();
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap2, 0, MAP_W, 0x40, MAP_W);
    MenuScreenDraw();
    SlotClear(0x2F);
    SlotInitTagged(D_8009AA4C, 0x3C, 0x300, 0x18, 0x18);
    SlotSetAnim(0x3C, 0, 0, 0, 0xA0, 0, 0, 0);
    SlotInitTagged(D_8009B074, 0x2D, 0x2FF, 0, 0x10);
    SlotSetAnim(0x2D, 0, 0, 0, 0xC0, 0xC, 0, 0);
    SlotInitTagged(g_fm_mark_def, MEMBER_CURSOR_SLOT, 0x42,
                   (g_fm_mark_pos + 1)[g_menu->status_who.cur][0],
                   (g_fm_mark_pos + 1)[g_menu->status_who.cur][1]);
    SlotSetFlicker(MEMBER_CURSOR_SLOT, 1);
    g_cam_y = 0;
    g_map_scroll_y = 0;
}

/* The member marker, a frame; accepting a member opens their page. */
void MenuStatusSelect(void)
{
    int      i;   /* the member, then their key, then a row */
    int      y;
    u_char  *arc;

    MenuStepMember(&g_menu->status_who.cur, g_party_last);
    SlotSetPos(MEMBER_CURSOR_SLOT, 0x42,
               (g_fm_mark_pos + 1)[g_menu->status_who.cur][0],
               (g_fm_mark_pos + 1)[g_menu->status_who.cur][1]);
    if (InputCheckAcceptA(1)) {
        i = g_menu->status_who.cur;
        i = g_chars[g_party_at[i]].key;
        g_bg_layers[4].x = 0x4C;
        g_bg_layers[4].y = 0xC8;
        g_bg_layers[4].w = 0xF0;
        g_bg_layers[4].h = 0x10;
        g_char_msg_key[0] = i;
        g_char_msg_key2 = i;
        BgMapInit(g_char_msg_key - 2, 0);
        AdvResolveSceneLoc(FACE_KIND, g_char_face_files[i], D_8005E714);
        CdReadFileToAddrAsync(&g_adv_scene_file, FACE_SECTORS, PORTRAIT_READ);
        while (g_cd_busy != -1) {
            RunFrame();
        }
        g_face_clut_fix = FACE_CLUT_FIX;
        TimQueueAt(PORTRAIT_TIM, 0x140, 0x168, 0, 0x1E6);

        func_80092E5C(1);
        TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
        TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
        TileMapFillRect(g_tilemap2, 0, MAP_W, 0x20, MAP_W);
        TileMapDrawWindow(AT(g_tilemap0, 1, 0), 0x1D, 0x20, MAP_W);
        TileMapDrawBox(AT(g_tilemap0, 2, 1), 0x1B, 0x1E, MAP_W);
        for (i = 0; i < 5; i++) {
            TileMapWriteBar(AT(g_tilemap0, 11 + i, 2), 0x19);
        }
        for (i = 0; i < 7; i++) {
            TileMapWriteBar(AT(g_tilemap0, 17 + i, 2), 2);
            TileMapWriteBar(AT(g_tilemap0, 17 + i, 4), 10);
            TileMapWriteBar(AT(g_tilemap0, 17 + i, 19), 5);
            TileMapWriteBar(AT(g_tilemap0, 17 + i, 24), 3);
        }
        TileMapWriteBar(AT(g_tilemap0, 24, 19), 5);
        TileMapWriteBar(AT(g_tilemap0, 24, 24), 3);
        TileMapWriteBar(AT(g_tilemap0, 26, 2), 0xB);
        TileMapWriteBar(AT(g_tilemap0, 27, 2), 0xB);
        TileMapWriteBar(AT(g_tilemap0, 28, 2), 0xB);

        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 3, 15), 0x36F, 5);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 4, 15), 0x37C, 3);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 5, 15), 0x374, 4);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 7, 15), 0x36D, 7);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 8, 15), 0x382, 1);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 8, 16), 0x382, 1);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 9, 15), 0x374, 4);
        for (i = 0; i < 7; i++) {
            TileMapWriteRow(str_cell_run, AT(g_tilemap1, 17 + i, 2),
                            0x3B1 + i * 2, 2);
        }
        arc = D_8009B330;
        TileMapWriteRow(arc, AT(g_tilemap1, 11, 12), 0, 3);
        TileMapWriteRow(arc + 3, AT(g_tilemap1, 12, 12), 0, 3);
        TileMapWriteRow(arc + 6, AT(g_tilemap1, 13, 12), 0, 3);
        TileMapWriteRow(arc + 9, AT(g_tilemap1, 14, 12), 0, 3);
        TileMapWriteRow(arc + 12, AT(g_tilemap1, 15, 12), 0, 3);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 17, 19), 0x3B1, 2);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 18, 19), 0x3B1, 2);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 19, 19), 0x3B3, 2);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 20, 19), 0x3B3, 2);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 17, 21), 0x3BF, 3);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 18, 21), 0x3C2, 3);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 19, 21), 0x3BF, 3);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 20, 21), 0x3C2, 3);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 21, 20), 0x3C5, 3);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 22, 20), 0x3C8, 3);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 23, 20), 0x3CB, 3);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 24, 20), 0x3D1, 3);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 25, 4), 0x457, 6);
        {
            short *cell;
            int    glyph;

            for (i = 2, cell = AT(g_tilemap1, 28, 2), glyph = 0x41A; i >= 0;
                 i--) {
                *cell = glyph;
                cell -= MAP_W;
                glyph--;
            }
        }
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 26, 16), 0x378, 2);
        *AT(g_tilemap1, 26, 23) = GLYPH_SLASH;
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 27, 16), 0x37A, 2);
        *AT(g_tilemap1, 27, 23) = GLYPH_SLASH;
        StatusDrawMain(g_menu->status_who.cur);

        g_bg_layer_otz[2] = 0x20;
        SlotClear(MEMBER_CURSOR_SLOT);
        SlotInitTagged(D_8009ABDC, 0x2E, 0x24, 0x48, 0xC6);
        MenuListInit(&g_menu->status_page, 0, 0, 2, 0x14);
        switch (g_menu->status_page.cur) {
        case 0:
            y = 0;
            break;
        case 1:
            y = STATUS_MID;
            break;
        case 2:
            y = STATUS_LOW;
            break;
        }
        SlotInitTagged(g_grow_slot_def, PAGE_TOP_SLOT, 1, 0x40, 0x20 - y);
        SlotInitTagged(g_pdata_bottom_def, PAGE_BOTTOM_SLOT, 0x50, 0x40,
                       0x84 - y);
        SlotSetFlicker(PAGE_BOTTOM_SLOT, 1);
        g_cam_y = y;
        g_map_scroll_y = y;
        SlotInitTagged(g_pdata_mark_up_def, PAGE_MARK_SLOT, 0x42, 0x18, 0x36);
        SlotInitTagged(g_pdata_mark_down_def, PAGE_MARK_SLOT + 1, 0x42, 0x18,
                       0xD0);
        if (g_party_last != 0) {
            SlotInitTagged(g_pdata_arrow_l_def, ARROW_L_SLOT, 0x23, 0x4A, 0xD8);
            SlotInitTagged(g_pdata_arrow_r_def, ARROW_R_SLOT, 0x23, 0xF6, 0xD8);
        }
        STATUS_MARKS();
        g_bg_shown |= 0x10;
        g_menu_subsel++;
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        g_menu_blink = 0xFF;
        g_menu_sel = 0;
        g_menu_subsel = 0;
    }
    DrawStatusHud();
}

/* A member's page, a frame: the arrows move to the next member, rereading
   their face; the page scrolls between its three stops. */
void MenuStatusView(void)
{
    int i;   /* the member, then their key */
    int y;

    if (!MenuStepCursor(&g_menu->status_page) &&
        MenuStepCursor(&g_menu->status_who)) {
        i = g_menu->status_who.cur;
        i = g_chars[g_party_at[i]].key;
        g_char_msg_key[0] = i;
        g_char_msg_key2 = i;
        BgMapInit(g_char_msg_key - 2, 0);
        AdvResolveSceneLoc(FACE_KIND, g_char_face_files[i], D_8005E714);
        CdReadFileToAddrAsync(&g_adv_scene_file, FACE_SECTORS, PORTRAIT_READ);
        while (g_cd_busy != -1) {
            RunFrame();
        }
        g_face_clut_fix = FACE_CLUT_FIX;
        TimQueueAt(PORTRAIT_TIM, 0x140, 0x168, 0, 0x1E6);
        StatusDrawMain(g_menu->status_who.cur);
    }

    switch (g_menu->status_page.cur) {
    case 0:
        y = 0;
        break;
    case 1:
        y = STATUS_MID;
        break;
    case 2:
        y = STATUS_LOW;
        break;
    }
    if (y < g_cam_y) {
        g_cam_y -= PAGE_STEP;
    }
    if (g_cam_y < y) {
        g_cam_y += PAGE_STEP;
    }
    if (y < g_map_scroll_y) {
        g_map_scroll_y -= PAGE_STEP;
    }
    if (g_map_scroll_y < y) {
        g_map_scroll_y += PAGE_STEP;
    }
    SlotSetPos(PAGE_TOP_SLOT, 1, 0x40, 0x20 - g_cam_y);
    SlotSetPos(PAGE_BOTTOM_SLOT, 0x50, 0x40, 0x84 - g_cam_y);
    STATUS_MARKS();
    func_80076380();
    if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        MenuStatusOpen();
        g_bg_layer_otz[2] = 0x40;
        g_menu_subsel--;
    }
}
