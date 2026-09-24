/* Persona 1 (JP) - the status menu's Persona pages.  ADV only.
 *   0x8006F3BC StatusPersonaPick     0x8006F6D0 StatusPersonaLayout
 *   0x8006FAA0 StatusPersonaPreview  0x8006FB30 StatusPersonaDraw
 *   0x8006FF00 StatusPersonaNames    0x8006FFC0 StatusPersonaView
 *   0x800704B0 StatusStockPick       0x8007081C StatusStockReleasePick
 *   0x80070ACC StatusStockReleaseConfirm  0x80070CBC StatusStockView
 *
 * A member's Personas: moving the cursor previews the member's stats with
 * each one active, and the view command reads the Persona's portrait off the
 * disc (AdvResolveSceneLoc kind 5) and opens its page. The Persona stock
 * opens the persona data screen's page instead (kind 4), and lets a Persona
 * go behind a yes/no prompt.
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
#include <persona/common/char.h>
#include <persona/common/persona.h>
#include <persona/common/formation.h>
#include <persona/adv/personapage.h>
#include <persona/common/bg.h>

/* The status menu's portraits: kind 5, eight sectors each. */
#define PORTRAIT_KIND    5
#define PORTRAIT_SECTORS 8

/* The stock's portraits are the persona data screen's: kind 4, nine
   sectors. */
#define STOCK_PORTRAIT_KIND    4
#define STOCK_PORTRAIT_SECTORS 9

extern void   CharApplyStats(u_char chr);
extern void   CharRecalcStats(u_char chr);

extern short   g_menu_subsel;

void func_8006E29C(short member);
void StatusPersonaLayout(void);
extern u_char D_800B16B0[];
void StatusPersonaNames(Persona *p);
extern void bcopy(void *src, void *dst, int len);
extern void func_80090644(Char *c, int a, int b);
extern void func_8006E94C(Char *c);
extern void DrawCharStatBars(Char *rec);
extern void DrawPersonaStatBars(Persona *p);
extern short func_80098B0C(short kind);
extern u_char D_800B198B[];
extern void   func_8006ED78(void);
extern void   func_8008C23C(short member);
extern void   BgMapInit(void *script, short speed);
extern void   PersonaDataLayout(void);
extern void   PersonaDataDraw(short id);
/* The last stock entry's index. */
extern short  g_stock_last;
extern u_char g_stock_release_msg[];
extern u_char D_800B1EB8[];
extern u_char g_fm_prompt_cur_def[];
extern u_char g_fm_hint_def[];
extern u_char g_fm_hint2_def[];
extern short  PersonaStockCompact(void);
extern void   PersonaStockDraw(void);
extern void   func_80076CE0(void);
extern u_char D_800B17E0[];
extern u_char D_800B1D08[];
extern u_char D_800B2330[];

#define PROMPT_CUR_SLOT 3
/* A description message's third byte, which is the Persona it describes. */
extern u_char g_persona_msg_key[];
extern u_char g_kind_labels[];
extern GsCELL g_spage_kind_cells[];
extern GsCELL g_spage_name_cells[];

/* The glyph between a Persona's level and its spell slots. */
#define GLYPH_SEP 0xCD
extern u_char D_800B167C[];
void StatusPersonaPreview(void);
void StatusPersonaDraw(short persona);

/* A member's Personas, a frame: the cursor walks the member's list. Under
   the equip command accepting one makes it the active entry; under the view
   command the Persona's portrait is read off the disc and its page opened. */
void StatusPersonaPick(void)
{
    Char  *chars = g_chars;
    int    member;
    u_char unused[0x60];

    if (g_menu->persona_slot.hi == 0xFF) {
        return;
    }
    if (MenuStepCursor(&g_menu->persona_slot)) {
        StatusPersonaPreview();
        SlotSetPos(PICK_CURSOR_SLOT, 0x42, 0x58,
                   g_menu->persona_slot.cur * 12 + 0x48);
    }
    if (InputCheckAcceptA(1)) {
        if (g_menu->persona_cmd.cur != 0) {
            MenuListInit(&g_menu->list[1], g_menu->persona_slot.cur, 0,
                         g_menu->persona_slot.hi, 0x1A);
            member = g_party_at[g_menu->status_member.cur];
            AdvResolveSceneLoc(PORTRAIT_KIND,
                               g_personas[chars[member].list[g_menu->list[1].cur]].key,
                               0);
            CdReadFileToAddrAsync(&g_adv_scene_file, PORTRAIT_SECTORS,
                                  PORTRAIT_READ);
            while (g_cd_busy != -1) {
                RunFrame();
            }
            TimQueueAt(PORTRAIT_TIM, 0x140, 0x168, 0, 0x1E6);
            MenuListInit(&g_menu->page, 0, 0, 1, 0x14);
            StatusPersonaLayout();
            member = g_party_at[g_menu->status_member.cur];
            StatusPersonaDraw(chars[member].list[g_menu->persona_slot.cur]);
            if (g_menu->list[1].hi != 0) {
                SlotInitTagged(g_pdata_arrow_l_def, 0x22, 0x23, 0x56, 0xD8);
                SlotInitTagged(g_pdata_arrow_r_def, 0x23, 0x23, 0x102, 0xD8);
            }
            g_menu_subsel++;
            return;
        }
        member = g_party_at[g_menu->status_member.cur];
        g_chars[member].entry = g_menu->persona_slot.cur;
        CharApplyStats(member);
        CharRecalcStats(member);
    } else if (!InputCheckAcceptB(1) && !g_menu_allow_hold) {
        return;
    }
    func_8006E29C(g_menu->status_member.cur);
    SlotClear(PICK_CURSOR_SLOT);
    SlotSetFlicker(1, 1);
    g_menu_subsel--;
}

/* The Persona view's page, laid out for the status menu: the persona data
   screen's page with seven numbered spell rows and without the top frame. */
void StatusPersonaLayout(void)
{
    int i;   /* the row, then the page's scroll stop */

    func_8008EDBC(9);
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapDrawWindow(AT(g_tilemap0, 1, 0), 0x1E, 0x24, MAP_W);
    TileMapDrawBox(AT(g_tilemap0, 3, 1), 0x1C, 0x20, MAP_W);
    for (i = 0; i < 7; i++) {
        TileMapWriteBar(AT(g_tilemap0, 9 + i, 18), 10);
        *AT(g_tilemap1, 9 + i, 17) = 0x418 + i;
    }
    for (i = 0; i < 5; i++) {
        int row = 24 + i;

        TileMapWriteBar(AT(g_tilemap0, row, 2), 0x19);
        TileMapWriteRow(&D_800B92A0[0x4C + i * 3], AT(g_tilemap1, row, 12),
                        0, 3);
    }
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 5, 18), 0x36F, 5);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 6, 21), 0x37A, 2);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 8, 20), 0x45D, 4);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 30, 2), 0x3CB, 3);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 31, 2), 0x3D1, 3);
    TileMapWriteRow(D_800B1898, AT(g_tilemap1, 29, 20), 0x1AE, 4);
    SlotInitTagged(D_800B16B0, 0, 8, 0x40, 0x34);
    SlotInitTagged(D_800B1E98, 0x2E, 0x24, 0x54, 0xC6);
    SlotInitTagged(D_800B167C, PAGE_TOP_SLOT, 0x20, 0x5C, 0xCC);
    SlotInitTagged(g_pdata_bottom_def, PAGE_BOTTOM_SLOT, 0x50, 0, 0);
    SlotSetFlicker(PAGE_BOTTOM_SLOT, 1);
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
    SlotSetPos(0, 0x52, 0x50, 0x34 - i);
    SlotSetPos(PAGE_BOTTOM_SLOT, 0x50, 0x48, 0x120 - g_cam_y);
    SlotInitTagged(g_pdata_mark_up_def, PAGE_MARK_SLOT, 0x42, 0x20, 0x36);
    SlotInitTagged(g_pdata_mark_down_def, PAGE_MARK_SLOT + 1, 0x42, 0x20, 0xD0);
    PAGE_MARKS();
}

/* The member's stats as they would be with the highlighted Persona active:
   worked out on a copy of the member's record and drawn from it. */
inline void StatusPersonaPreview(void)
{
    Char c;

    bcopy(&g_chars[g_party_at[g_menu->status_member.cur]], &c, sizeof(Char));
    c.entry = g_menu->persona_slot.cur;
    func_80090644(&c, 0xFF, 0xFF);
    func_8006E94C(&c);
    *AT(g_tilemap1, 10, 20) = GLYPH_SEP;
    DrawCharStatBars(&c);
}

/* A carried Persona's page: its level and spell slots, SP cost, the seven
   spells, the five stats, the two contact values, its class lines, its
   resistance line, its names and its stat bars. */
void StatusPersonaDraw(persona)
    short persona;
{
    Persona *p = &g_personas[persona];
    int      i;
    u_short  n;

    TileMapFillRect(AT(g_tilemap1, 5, 24), 0, 4, 2, MAP_W);
    n = FormatDecimal(g_personas[persona].level, g_hud_digits, 2);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 5, 25), GLYPH_DIGIT0, n);
    *AT(g_tilemap1, 5, 26) = GLYPH_SEP;
    FormatDecimal(g_personas[persona].slots, g_hud_digits, 1);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 5, 27), GLYPH_DIGIT0, 1);
    n = FormatDecimal(g_personas[persona].sp_cost, g_hud_digits, 3);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 6, 26), GLYPH_DIGIT0, n);
    for (i = 0; i < PERSONA_SPELLS; i++) {
        DrawSpellName(p->spell[i], AT(g_tilemap1, 9 + i, 18), 0, 1);
    }

    TileMapFillRect(AT(g_tilemap1, 24, 16), 0, 2, 5, MAP_W);
    n = FormatDecimal(p->stat[0], g_hud_digits, 2);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 24, 17), GLYPH_DIGIT0, n);
    n = FormatDecimal(p->stat[1], g_hud_digits, 2);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 25, 17), GLYPH_DIGIT0, n);
    n = FormatDecimal(p->stat[2], g_hud_digits, 2);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 26, 17), GLYPH_DIGIT0, n);
    n = FormatDecimal(p->stat[3], g_hud_digits, 2);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 27, 17), GLYPH_DIGIT0, n);
    n = FormatDecimal(p->stat[4], g_hud_digits, 2);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 28, 17), GLYPH_DIGIT0, n);

    TileMapFillRect(AT(g_tilemap1, 30, 7), 0, 3, 2, MAP_W);
    n = FormatDecimal(p->unk10, g_hud_digits, 3);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 30, 9), GLYPH_DIGIT0, n);
    n = FormatDecimal(p->unk12, g_hud_digits, 3);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 31, 9), GLYPH_DIGIT0, n);

    i = D_800B198B[p->kind];
    TileMapWriteRow(&D_800B1A98[i * 10], AT(g_tilemap1, 30, 18), 0, 10);
    i = func_80098B0C(g_persona_defs[p->key].pad27[0]);
    TileMapWriteRow(&D_800B1A98[0x32 + i * 10], AT(g_tilemap1, 31, 18), 0, 10);
    TileMapFillRect(AT(g_tilemap1, 33, 3), 0, 0x19, 1, MAP_W);
    TileMapWriteRow(&g_resist_labels[g_persona_defs[p->key].resist * 25],
                    AT(g_tilemap1, 33, 3), 0, 0x19);
    StatusPersonaNames(p);
    DrawPersonaStatBars(p);
}

/* The Persona's kind and name into their cell rows. */
void StatusPersonaNames(Persona *p)
{
    CellsClear(g_spage_kind_cells, 10);
    CellsClear(g_spage_name_cells, 10);
    CellsWriteRow(g_spage_kind_cells, &g_kind_labels[(p->kind - 1) * 10], 0, 10);
    CellsWriteRow(g_spage_name_cells, g_persona_defs[p->key].name, 0, 10);
}

/* The Persona view, a frame: moving between the member's Personas reads the
   next one's portrait and redraws the page, and its description opens in the
   message window; the page scrolls between its halves. Backing out redraws
   the status page with the preview of the Persona the cursor is on. */
void StatusPersonaView(void)
{
    Char    *chars = g_chars;
    Persona *personas = g_personas;
    int      member;   /* the member, then the page's scroll stop */
    int      persona;

    if (!MenuStepCursor(&g_menu->page) && MenuStepCursor(&g_menu->list[1])) {
        member = g_party_at[g_menu->status_member.cur];
        persona = chars[member].list[g_menu->list[1].cur];
        AdvResolveSceneLoc(PORTRAIT_KIND, g_personas[persona].key, 0);
        CdReadFileToAddrAsync(&g_adv_scene_file, PORTRAIT_SECTORS,
                              PORTRAIT_READ);
        while (g_cd_busy != -1) {
            RunFrame();
        }
        TimQueueAt(PORTRAIT_TIM, 0x140, 0x168, 0, 0x1E6);
        StatusPersonaDraw(persona);
        g_menu->persona_slot.cur = g_menu->list[1].cur;
        g_persona_msg_key[0] = personas[persona].key;
        BgMapInit(g_persona_msg_key - 2, 0);
    }

    switch (g_menu->page.cur) {
    case 0:
        member = 0;
        break;
    case 1:
        member = PAGE_LOW;
        break;
    }
    if (member < g_cam_y) {
        g_cam_y -= PAGE_STEP;
    }
    if (g_cam_y < member) {
        g_cam_y += PAGE_STEP;
    }
    if (member < g_map_scroll_y) {
        g_map_scroll_y -= PAGE_STEP;
    }
    if (g_map_scroll_y < member) {
        g_map_scroll_y += PAGE_STEP;
    }
    SlotSetPos(0, 0x52, 0x50, 0x34 - g_cam_y);
    SlotSetPos(PAGE_BOTTOM_SLOT, 0x50, 0x48, 0x120 - g_cam_y);
    PAGE_MARKS();

    if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        func_8006ED78();
        func_8006E29C(g_menu->status_member.cur);
        func_8008C23C(g_menu->status_member.cur);
        StatusPersonaPreview();
        SlotInitTagged(g_pdata_cursor_def, 1, 0x42, 0x58,
                       g_menu->persona_cmd.cur * 12 + 0x24);
        SlotInitTagged(g_pdata_cursor_def, PICK_CURSOR_SLOT, 0x42, 0x58,
                       g_menu->persona_slot.cur * 12 + 0x48);
        SlotSetPos(PAGE_BOTTOM_SLOT, 0x50, 0x48, 0x9C);
        SlotSetFlicker(PICK_CURSOR_SLOT, 1);
        SlotClear(0);
        SlotClear(PAGE_TOP_SLOT);
        SlotClear(PAGE_MARK_SLOT);
        SlotClear(PAGE_MARK_SLOT + 1);
        SlotClear(ARROW_L_SLOT);
        SlotClear(ARROW_R_SLOT);
        SlotClear(0x2E);
        SlotClear(0x2F);
        g_map_scroll_y = 0;
        g_cam_y = 0;
        g_menu_subsel--;
    }
}

/* The Persona stock, a frame. Accepting a Persona reads its portrait and
   opens the persona data page on it; the row after the last one starts
   releasing one, with its help line up. */
void StatusStockPick(void)
{
    u_char id;

    if (MenuStepCursor(&g_menu->stock)) {
        if (g_menu->stock.cur != g_stock_last + 1) {
            SlotSetPos(1, 0x42, 0x48, g_menu->stock.cur * 12 + 0x24);
        } else {
            SlotSetPos(1, 0x42, 0x48, 0xC0);
        }
    }
    if (InputCheckAcceptA(1)) {
        if (g_menu->stock.cur != g_stock_last + 1) {
            MenuListInit(&g_menu->page, 0, 0, 1, 0x14);
            MenuListInit(&g_menu->list[0], g_menu->stock.cur, 0, g_stock_last,
                         0x1A);
            id = g_persona_stock[g_menu->stock.cur];
            AdvResolveSceneLoc(STOCK_PORTRAIT_KIND, id, 0);
            CdReadFileToAddrAsync(&g_adv_scene_file, STOCK_PORTRAIT_SECTORS,
                                  PORTRAIT_READ);
            while (g_cd_busy != -1) {
                RunFrame();
            }
            TimQueueAt(PORTRAIT_TIM, 0x140, 0x168, 0, 0x1E6);
            PersonaDataLayout();
            PersonaDataDraw(id);
            if (g_menu->list[0].hi > 0) {
                SlotInitTagged(g_pdata_arrow_l_def, ARROW_L_SLOT, 0x23, 0x56,
                               0xD8);
                SlotInitTagged(g_pdata_arrow_r_def, ARROW_R_SLOT, 0x23, 0x102,
                               0xD8);
            }
            g_menu_subsel += 3;
        } else {
            BgMapInit(g_stock_release_msg, 0);
            g_bg_layers[4].x = 0x38;
            g_bg_layers[4].y = 0xE;
            g_bg_layers[4].w = 0xF0;
            g_bg_layers[4].h = 0x10;
            g_bg_shown |= 0x10;
            MenuListInit(&g_menu->stock_release, 0, 0, g_stock_last, 0x1E);
            SlotInitTagged(D_800B1EB8, 0x2E, 0x24, 0x36, 0xC);
            SlotInitTagged(g_pdata_cursor_def, PICK_CURSOR_SLOT, 0x42, 0x48,
                           g_menu->stock_release.cur * 12 + 0x24);
            SlotSetFlicker(1, 0);
            SlotSetFlicker(PICK_CURSOR_SLOT, 1);
            g_menu_subsel++;
        }
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        g_menu_subsel = 0;
    }
}

/* Releasing a Persona, a frame: the cursor picks the stock entry, and
   accepting it puts up the yes/no prompt, "no" first. */
void StatusStockReleasePick(void)
{
    if (MenuStepCursor(&g_menu->stock_release)) {
        SlotSetPos(PICK_CURSOR_SLOT, 0x42, 0x48,
                   g_menu->stock_release.cur * 12 + 0x24);
    }
    MsgStep();
    if (InputCheckAcceptA(1)) {
        MenuListInit(&g_menu->list[1], 1, 0, 1, 0x1E);
        SlotInitTagged(g_fm_prompt_cur_def, PROMPT_CUR_SLOT, 0x23, 0xE8,
                       g_menu->list[1].cur * 16 + 0xC2);
        SlotInitTagged(g_fm_hint_def, 8, 0x24, 0xE8, 0xB0);
        SlotInitTagged(g_fm_hint_def, 9, 0x24, 0xE8, 0xC0);
        SlotInitTagged(g_fm_hint_def, 0xA, 0x24, 0xE8, 0xD0);
        SlotInitTagged(g_fm_hint2_def, 0xB, 0x22, 0xE8, 0xB0);
        SlotInitTagged(g_fm_hint2_def, 0xC, 0x22, 0xE8, 0xC0);
        SlotInitTagged(g_fm_hint2_def, 0xD, 0x22, 0xE8, 0xD0);
        SlotSetAnim(0xB, 0, 0, 0, 0xC0, 0, 0, 0);
        SlotSetAnim(0xC, 0, 0, 0, 0x30, 0, 0, 0);
        SlotSetAnim(0xD, 0, 0, 0, 0x60, 0, 0, 0);
        SlotSetFlicker(PICK_CURSOR_SLOT, 0);
        SlotSetFlicker(PROMPT_CUR_SLOT, 1);
        g_menu_subsel++;
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        g_bg_shown ^= 0x10;
        SlotClear(PICK_CURSOR_SLOT);
        SlotClear(0x2E);
        SlotSetFlicker(1, 1);
        g_menu_subsel--;
    }
}

/* The release prompt, a frame. "Yes" empties the entry and closes the
   stock's gap; releasing the last one leaves the stock screen. */
void StatusStockReleaseConfirm(void)
{
    if (MenuStepCursor(&g_menu->list[1])) {
        SlotSetPos(PROMPT_CUR_SLOT, 0x23, 0xE8, g_menu->list[1].cur * 16 + 0xC2);
    }
    MsgStep();
    if (InputCheckAcceptA(1)) {
        if (g_menu->list[1].cur == 0) {
            g_persona_stock[g_menu->stock_release.cur] = STOCK_FREE;
            g_stock_last = PersonaStockCompact();
            PersonaStockDraw();
            MenuListInit(&g_menu->stock, g_stock_last + 1, 0, g_stock_last + 1,
                         0x1E);
            MenuListInit(&g_menu->stock_release, 0, 0, g_stock_last, 0x1E);
            if (g_stock_last == -1) {
                func_80076CE0();
                g_menu_subsel = 0;
                return;
            }
        }
    } else if (!InputCheckAcceptB(1) && !g_menu_allow_hold) {
        return;
    }
    SlotClear(PROMPT_CUR_SLOT);
    SlotClear(8);
    SlotClear(9);
    SlotClear(0xA);
    SlotClear(0xB);
    SlotClear(0xC);
    SlotClear(0xD);
    SlotSetPos(PICK_CURSOR_SLOT, 0x42, 0x48, g_menu->stock_release.cur * 12 + 0x24);
    SlotSetFlicker(PICK_CURSOR_SLOT, 1);
    g_menu_subsel--;
}

/* A stock Persona's data page, a frame. While the page rests at either stop
   the cursor moves between stock entries, reading each one's portrait; backing
   out redraws the stock list. */
void StatusStockView(void)
{
    int id;   /* the Persona, then the page's scroll stop, then the row */

    if (!MenuStepCursor(&g_menu->page) && (g_cam_y == 0 || g_cam_y == PAGE_LOW) &&
        MenuStepCursor(&g_menu->list[0])) {
        id = g_persona_stock[g_menu->list[0].cur];
        AdvResolveSceneLoc(STOCK_PORTRAIT_KIND, id, 0);
        CdReadFileToAddrAsync(&g_adv_scene_file, STOCK_PORTRAIT_SECTORS,
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

    if ((g_cam_y == 0 || g_cam_y == PAGE_LOW) &&
        (InputCheckAcceptB(1) || g_menu_allow_hold)) {
        func_8008EDBC(0xA);
        SlotClearAll();
        TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
        TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
        TileMapDrawWindow(g_tilemap0, 0x1E, 0x12, MAP_W);
        TileMapDrawBox(AT(g_tilemap0, 1, 1), 0x1C, 0x10, MAP_W);
        TileMapWriteRow(D_800B17E0, AT(g_tilemap1, 13, 2), 0x285, 6);
        for (id = 0; id < STOCK_ROWS; id++) {
            TileMapWriteBar(AT(g_tilemap0, 2 + id, 2), 0xB);
            TileMapWriteBar(AT(g_tilemap0, 2 + id, 13), 5);
            TileMapWriteBar(AT(g_tilemap0, 2 + id, 18), 10);
        }
        g_stock_last = PersonaStockCompact();
        PersonaStockDraw();
        TileMapWriteBar(AT(g_tilemap0, 15, 2), 10);
        SlotInitTagged(D_800B1D08, 0x3C, 0x300, 0x18, 0x18);
        SlotInitTagged(D_800B2330, 0x2D, 0x2FF, 0, 0x10);
        SlotSetAnim(0x2D, 0, 0, 0, 0x90, 0xC, 0, 0);
        SlotInitTagged(g_pdata_cursor_def, 1, 0x42, 0, 0);
        g_menu->stock.cur = g_menu->list[0].cur;
        if (g_menu->stock.cur != g_stock_last + 1) {
            SlotSetPos(1, 0x42, 0x48, g_menu->stock.cur * 12 + 0x24);
        } else {
            SlotSetPos(1, 0x42, 0x48, 0xC0);
        }
        SlotSetFlicker(1, 1);
        g_cam_y = 0;
        g_map_scroll_y = 0;
        g_menu_subsel -= 3;
    }
}
