/* Persona 1 (JP) - fusing two Personas.  ADV only.
 *   0x8009D5A8 FusionStep        0x8009D634 FusionPairPick
 *   0x8009DAE4 FusionConfirm     0x8009DE28 FuseItemsOpen
 *   0x8009E2CC FuseItemPick      0x8009EAB8 FuseResultView
 *   0x8009F0FC FusePersonaDraw   0x8009F7D0 FuseExecute
 *   0x800A0388 PersonaSlotsFree  0x800A03C8 PersonaSlotsCompact
 *
 * The facility host's kind 7, and the back half of kind 8 (fusesearch.c):
 * two Personas out of the stock, optionally an item offered on top, the
 * result's page with its portrait, then the fusion itself. A result more
 * than ten levels above the hero, or one already owned, is refused.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/menuctx.h>
#include <persona/common/bg.h>
#include <persona/common/persona.h>
#include <persona/common/char.h>
#include <persona/adv/personapage.h>

#define STEP_DONE      0xFF
#define PAGE_MARK_SLOT 32
#define ITEM_ID        0x1FF
#define GLYPH_DIGIT0   0xC0

typedef struct {
    u_short arcana;
    u_short unk2;
    u_short unk4;
    u_short persona;
    u_short flag;
} FuseResult;

#define g_fuse (*(FuseResult *)0x801F1B8C)

/* The item offered, one of the bag's work list. */
#define g_item_list ((u_short *)0x800EAE4C)

#define g_persona_slots ((u_char *)0x801F2574)

/* What the scene that plays a fusion reads: the result and both sources,
   whether an item was offered, and whether it went wrong. */
#define g_fuse_args  ((short *)0x801F1BA0)
#define g_fuse_scene (*(short *)0x801F1BCA)
#define g_seq_handle ((short *)0x801F537C)
#define SLOT_EMPTY      0xFF
#define PERSONA_SLOTS   16

extern short   g_persona_data_step;
extern u_char  g_menu_allow_hold;
extern short   D_800B8458;
extern short   D_800BB7F4;       /* the item list's top row */
extern short   D_800BB820;       /* 0 from the stock (kind 7), else from a search */
extern short   D_800BC050;       /* an item is offered */
extern short   D_800BBB34;       /* how many items the list holds */
extern volatile int g_cd_busy;
extern u_char  g_kind_labels[];
extern u_char  g_fm_prompt_cur_def[];
extern u_char  g_fm_hint_def[];
extern u_char  g_fm_hint2_def[];
extern u_char  D_800B1D08[];
extern u_char  D_800B2330[];
extern u_char  D_800B1EB8[];
extern u_char  D_800B148C[];
extern u_char  D_800B1528[];
extern u_char  D_800BA66C[];
extern u_char  D_800BA680[];
extern u_char  D_800BA694[];
extern u_char  D_800BA6A8[];
extern u_char  D_800B198B[];
extern GsCELL  g_spage_kind_cells[];
extern GsCELL  g_spage_name_cells[];
extern const u_char g_persona_list_rule[];

extern u_char PageScrollValue(short *value, short lo, short hi, short step);
extern short  MenuScrollCursor(MenuList *m, short *row, short first,
                               short last, u_short *offset);
extern void   MenuResetRepeat(MenuList *m);
extern void   StatusPersonaLayout(void);
extern void   DrawPersonaStatBars(Persona *p);
extern short  func_80098B0C(short kind);
/* PersonaFind is called without a prototype here. */
extern void   DrawItemRow(short n, short *dst);
extern void   TextItemStatRow(short item, short x, short y);
extern void   func_800A1990(u_char a, u_char b, short mode, FuseResult *out,
                            short special);
extern short  func_800A2904(void);
extern void   func_800A1690(short persona);
extern void   func_800A08E4(void);
extern void   func_800A11EC(void);
extern void   func_800AB0D0(short n);
extern void   func_800AB1EC(void);
extern void   D_8008D924(const PersonaDef *d, short stat);
extern void   D_80076EB0(short n);
extern void   FusePairsOpen(void);
/* PersonaFindFree, PersonaFill and PersonaFind are called without
   prototypes here. */
extern short  func_800A26A0(short persona);
extern short  func_800A080C(short kind);
extern short  func_800A0744(void);
extern short  func_800A26DC(u_char a, u_char b);
extern void   func_800A0450(short p, short kind, short level);
extern void   func_800AB23C(short persona, u_char src, short *spell,
                            short *rank);
extern void   ItemsRemovePending(short item, short count);
extern void   ItemsCommitPending(void);
extern void   ItemsCompact(void);
extern int    rand(void);
extern void   LoadFileToAddrAsync(char *name, void *dst);
extern void   SsSeqSetDecrescendo(short seq, long vol, long ticks);
extern u_short g_cutscene_on;
extern short  D_800BC5C0;       /* the fusion went wrong */
extern int    g_pad_held[];
extern u_char g_pad_config;

void FusionPairPick(void);
void FusionConfirm(void);
void FuseItemsOpen(void);
void FuseItemPick(void);
void FuseResultView(void);
void FusePersonaDraw(short id, short unused, short built);
void FuseExecute(void);
short PersonaSlotsFree(void);

#define MARKS(top, last)                                                      \
    g_slot_cur = &g_slots[PAGE_MARK_SLOT];                                    \
    if ((top) == 0) {                                                         \
        g_slot_cur->attr |= SLOT_ATTR_HIDE;                                   \
    } else {                                                                  \
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;                                  \
    }                                                                         \
    g_slot_cur = &g_slots[PAGE_MARK_SLOT + 1];                                \
    if ((top) != (last)) {                                                    \
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;                                  \
    } else {                                                                  \
        g_slot_cur->attr |= SLOT_ATTR_HIDE;                                   \
    }

/* The result's portrait, read off the disc, and its page. */
#define SHOW_RESULT(owned)                                                    \
    AdvResolveSceneLoc(5, g_fuse.persona, 0);                                 \
    CdReadFileToAddrAsync(&g_adv_scene_file, 8, PORTRAIT_READ);               \
    while (g_cd_busy != -1) {                                                 \
        RunFrame();                                                           \
    }                                                                         \
    TimQueueAt(PORTRAIT_TIM, 0x140, 0x168, 0, 0x1E6);                         \
    SlotClearAll();                                                           \
    SlotInitTagged(D_800B1D08, 0x3C, 0x300, 0x18, 0x18);                      \
    SlotInitTagged(D_800B2330, 0x2D, 0x2FF, 0, 0x10);                         \
    SlotSetAnim(0x2D, 0, 0, 0, 0x60, 0xC, 0, 0);                              \
    MenuListInit(&g_menu->page, 0, 0, 1, 0x14);                               \
    StatusPersonaLayout();                                                    \
    FusePersonaDraw(g_fuse.persona, (owned), 0)

/* Back to picking the pair, whichever screen that was. */
#define BACK_TO_PAIRS()                                                       \
    if (D_800BB820 == 0) {                                                    \
        func_800A11EC();                                                      \
        SlotInitTagged(D_800B148C, 1, 0x42, g_menu->top.cur * 8 + 0xC8, 0x30);\
        SlotInitTagged(D_800B1528, 2, 0x42, 0x20,                             \
                       g_menu->status_page.cur * 12 + 0x30);                  \
        SlotSetFlicker(1, 1);                                                 \
        SlotSetFlicker(2, 1);                                                 \
        g_slot_cur = &g_slots[1];                                             \
        g_slot_cur->flicker = 0;                                              \
        g_slot_cur = &g_slots[2];                                             \
        g_slot_cur->flicker = 0;                                              \
    } else {                                                                  \
        FusePairsOpen();                                                      \
    }

#define ITEM_AT(n) g_item_list[n]

void FusionStep(void)
{
    switch (g_persona_data_step) {
    case 0:
        FusionPairPick();
        break;
    case 1:
        FusionConfirm();
        break;
    case 2:
        FuseItemPick();
        break;
    case 3:
        FuseResultView();
        break;
    case 4:
        FuseExecute();
        break;
    }
}

void FusionPairPick(void)
{
    short    cost;
    u_short *res;

    if (!MenuStepCursor(&g_menu->status_page)) {
        MenuStepCursor(&g_menu->top);
    }
    SlotSetPos(1, 0x42, g_menu->top.cur * 8 + 0xC8, 0x30);
    SlotSetPos(2, 0x42, 0x20, g_menu->status_page.cur * 12 + 0x30);
    res = &g_fuse.persona;
    func_800A1990(g_persona_stock[g_menu->status_page.cur],
                  g_persona_stock[g_menu->top.cur], 0, &g_fuse, 0);
    func_800A1690(*res);
    if (InputCheckAcceptA(1)) {
        D_800BC050 = 0;
        if (g_fuse.arcana != 0) {
            D_800BBB34 = cost = func_800A2904();
            if (cost != 0) {
                if (cost < 10) {
                    D_800BBB34 = 10;
                }
                MenuListInit(&g_menu->list[1], 0, 0, 1, 0x1E);
                SlotInitTagged(g_fm_prompt_cur_def, 3, 0x23, 0xF0,
                               g_menu->list[1].cur * 16 + 0x4A);
                SlotInitTagged(g_fm_hint_def, 9, 0x24, 0xF0, 0x48);
                SlotInitTagged(g_fm_hint_def, 0xA, 0x24, 0xF0, 0x58);
                SlotInitTagged(g_fm_hint2_def, 0xC, 0x22, 0xF0, 0x48);
                SlotInitTagged(g_fm_hint2_def, 0xD, 0x22, 0xF0, 0x58);
                SlotSetAnim(0xC, 0, 0, 0, 0x30, 0, 0, 0);
                SlotSetAnim(0xD, 0, 0, 0, 0x60, 0, 0, 0);
                SlotSetFlicker(3, 1);
                SlotInitTagged(D_800B1EB8, 0x2E, 0x24, 0x36, 0xE);
                SlotSetFlicker(1, 0);
                SlotSetFlicker(2, 0);
                BgMapInit(D_800BA66C, 0);
                g_bg_layers[4].x = 0x38;
                g_bg_layers[4].y = 0x10;
                g_bg_layers[4].w = 0xF0;
                g_bg_layers[4].h = 0x10;
                g_bg_shown |= 0x10;
                g_persona_data_step++;
                return;
            }
            SHOW_RESULT(1);
            g_bg_map1.ncellh = 0x40;
            g_persona_data_step += 3;
        }
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        g_persona_data_step = STEP_DONE;
    }
}

void FusionConfirm(void)
{
    MenuStepCursor(&g_menu->list[1]);
    SlotSetPos(3, 0x23, 0xF0, g_menu->list[1].cur * 16 + 0x4A);
    MsgStep();
    if (InputCheckAcceptA(1)) {
        if (g_menu->list[1].cur == 0) {
            D_800BC050++;
            FuseItemsOpen();
            g_persona_data_step++;
            return;
        }
        SHOW_RESULT(0);
        g_bg_map1.ncellh = 0x40;
        g_persona_data_step += 2;
        return;
    }
    if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        SlotInitTagged(D_800B148C, 1, 0x42, g_menu->top.cur * 8 + 0xC8, 0x30);
        SlotInitTagged(D_800B1528, 2, 0x42, 0x20,
                       g_menu->status_page.cur * 12 + 0x30);
        func_800AB1EC();
        SlotClear(0x2E);
        SlotSetFlicker(1, 1);
        SlotSetFlicker(2, 1);
        g_slot_cur = &g_slots[1];
        g_slot_cur->flicker = 0;
        g_slot_cur = &g_slots[2];
        g_slot_cur->flicker = 0;
        g_bg_shown ^= 0x10;
        g_persona_data_step--;
    }
}

/* The items that can be offered, two columns of five rows. */
void FuseItemsOpen(void)
{
    int   i;
    short n;

    n = D_800BB7F4 * 2 + g_menu->unk2E0.cur + g_menu->unk2D0.cur * 2;
    func_8008EDBC(0x1B);
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap2, 0, MAP_W, 0x40, MAP_W);
    TileMapDrawWindow(g_tilemap0, 0x20, 0x11, MAP_W);
    TileMapDrawBox(g_tilemap0 + MAP_W + 1, 0x1E, 0xF, MAP_W);
    for (i = 0; i < 5; i++) {
        TileMapWriteBar(AT(g_tilemap0, i + 3, 3), 0xA);
        TileMapWriteBar(AT(g_tilemap0, i + 3, 13), 2);
        TileMapWriteBar(AT(g_tilemap0, i + 3, 17), 0xA);
        TileMapWriteBar(AT(g_tilemap0, i + 3, 27), 2);
    }
    TileMapFillRect(AT(g_tilemap0, 3, 15), 0x17, 2, 5, MAP_W);
    TileMapFillRect(AT(g_tilemap0, 9, 2), 0x17, 0x1C, 6, MAP_W);
    for (i = 0; i < 5; i++) {
        DrawItemRow((D_800BB7F4 + i) * 2,
                    AT(g_tilemap1, (D_800BB7F4 + i) & 0x1F, 0));
        DrawItemRow((D_800BB7F4 + i) * 2 + 1,
                    AT(g_tilemap1, (D_800BB7F4 + i) & 0x1F, 14));
    }
    TileMapWriteRow(str_cell_run, AT(g_tilemap2, 5, 12), 0x383, 2);
    *AT(g_tilemap2, 1, 13) = 0xCD;
    *AT(g_tilemap2, 3, 8) = 0xCD;
    *AT(g_tilemap2, 3, 21) = 0xCF;
    func_800A08E4();
    SlotClearAll();
    SlotInitTagged(D_800B1D08, 0x3C, 8, 0x18, 0x18);
    SlotInitTagged(D_800B2330, 0x2D, 7, 0, 0x10);
    SlotSetAnim(0x2D, 0, 0, 0, 0x30, 0x24, 0, 0);
    SlotInitTagged(D_800B1EB8, 0x2E, 0x24, 0x3E, 0xE);
    TextItemStatRow(ITEM_AT(n) & ITEM_ID, 0x40, 0x10);
    SlotInitTagged(g_pdata_cursor_def, 4, 0x42, g_menu->unk2E0.cur * 0x68 + 0x50,
                   g_menu->unk2D0.cur * 12 + 0x30);
    SlotSetFlicker(4, 1);
    SlotInitTagged(g_pdata_mark_up_def, PAGE_MARK_SLOT, 0x42, 0xB0, 0x30);
    SlotInitTagged(g_pdata_mark_down_def, PAGE_MARK_SLOT + 1, 0x42, 0xB0, 0x60);
    MARKS(D_800BB7F4, (D_800BBB34 + 1) / 2 - 5);
    g_bg_map1.ncellh = 0x20;
    g_cam_y = 0;
    g_bg_layers[4].x = 0x40;
    g_map_scroll_y = D_800BB7F4 * 12;
}

void FuseItemPick(void)
{
    u_short *items = g_item_list;
    int      i;
    short    prev;

    prev = D_800BB7F4 * 2 + g_menu->unk2E0.cur + g_menu->unk2D0.cur * 2;
    if ((short)(g_map_scroll_y % 12) == 0) {
        if (D_800B8458 != 0) {
            if (g_menu->unk2D0.delay < 3) {
                g_menu->unk2D0.delay = 0;
            }
            D_800B8458 = 0;
        }
        if (PageScrollValue(&D_800BB7F4, 0, (D_800BBB34 + 1) / 2 - 5, 5)) {
            for (i = 0; i < 5; i++) {
                DrawItemRow((D_800BB7F4 + i) * 2,
                            AT(g_tilemap1, (D_800BB7F4 + i) & 0x1F, 0));
                DrawItemRow((D_800BB7F4 + i) * 2 + 1,
                            AT(g_tilemap1, (D_800BB7F4 + i) & 0x1F, 14));
            }
            g_map_scroll_y = D_800BB7F4 * 12;
        } else if (MenuScrollCursor(&g_menu->unk2D0, &D_800BB7F4, 0,
                                    (D_800BBB34 + 1) / 2 - 5,
                                    (u_short *)&D_800B8458)) {
            if (D_800B8458 < 0) {
                DrawItemRow(D_800BB7F4 * 2, AT(g_tilemap1, D_800BB7F4 & 0x1F, 0));
                DrawItemRow(D_800BB7F4 * 2 + 1,
                            AT(g_tilemap1, D_800BB7F4 & 0x1F, 14));
            } else if (D_800B8458 > 0) {
                DrawItemRow((D_800BB7F4 + 4) * 2,
                            AT(g_tilemap1, (D_800BB7F4 + 4) & 0x1F, 0));
                DrawItemRow((D_800BB7F4 + 4) * 2 + 1,
                            AT(g_tilemap1, (D_800BB7F4 + 4) & 0x1F, 14));
            }
        } else {
            MenuStepCursor(&g_menu->unk2E0);
        }
    } else {
        MenuResetRepeat(&g_menu->unk2D0);
    }
    MARKS(D_800BB7F4, (D_800BBB34 + 1) / 2 - 5);
    SlotSetPos(4, 0x42, g_menu->unk2E0.cur * 0x70 + 0x50,
               g_menu->unk2D0.cur * 12 + 0x30);
    g_map_scroll_y += D_800B8458;
    if (prev != D_800BB7F4 * 2 + g_menu->unk2E0.cur + g_menu->unk2D0.cur * 2) {
        prev = D_800BB7F4 * 2 + g_menu->unk2E0.cur + g_menu->unk2D0.cur * 2;
        if ((items[prev] & ITEM_ID) && (items[prev] >> 9)) {
            TextItemStatRow(ITEM_AT(prev) & ITEM_ID, 0x40, 0x10);
        } else {
            TextItemStatRow(0, 0x40, 0x10);
        }
        func_800AB0D0(prev);
        func_800A08E4();
    }
    MsgStep();
    if ((short)(g_map_scroll_y % 12) == 0) {
        if (InputCheckAcceptA(1)) {
            if ((items[prev] & ITEM_ID) && (items[prev] >> 9)) {
                SHOW_RESULT(*(short *)0x801F1BBA);
                g_bg_map1.ncellh = 0x40;
                g_persona_data_step++;
            }
        } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
            BACK_TO_PAIRS();
            g_persona_data_step -= 2;
        }
    }
}

void FuseResultView(void)
{
    int      stop;
    u_char  *msg;
    u_short *res;

    MenuStepCursor(&g_menu->page);
    switch (g_menu->page.cur) {
    case 0:
        stop = 0;
        break;
    case 1:
        stop = 0xE0;
        break;
    }
    if (stop < g_cam_y) {
        g_cam_y -= 8;
    }
    if (g_cam_y < stop) {
        g_cam_y += 8;
    }
    if (stop < g_map_scroll_y) {
        g_map_scroll_y -= 8;
    }
    if (g_map_scroll_y < stop) {
        g_map_scroll_y += 8;
    }
    SlotSetPos(0, 0x52, 0x50, 0x34 - g_cam_y);
    SlotSetPos(0x1F, 0x50, 0x48, 0x120 - g_cam_y);
    g_slot_cur = &g_slots[PAGE_MARK_SLOT];
    if (g_cam_y == 0) {
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
    } else {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }
    g_slot_cur++;
    if (g_cam_y == 0xE0) {
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
    } else {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }
    if (g_cam_y == 0 || g_cam_y == 0xE0) {
        if (InputCheckAcceptA(1)) {
            SlotInitTagged(D_800B1EB8, 0x2F, 0x24, 0x36, 0x1C);
            res = &g_fuse.persona;
            if (PersonaFind(*res) == -1) {
                stop = g_persona_defs[*res].level;
                if (g_chars[0].level + 10 < stop) {
                    msg = D_800BA6A8;
                    goto refuse;
                }
                BgMapInit(D_800BA680, 0);
                MenuListInit(&g_menu->list[0], 0, 0, 1, 0x1E);
                SlotInitTagged(g_fm_hint_def, 0x24, 0x24, 0x108, 0x48);
                SlotInitTagged(g_fm_hint_def, 0x25, 0x24, 0x108, 0x58);
                SlotInitTagged(g_fm_hint_def, 0x26, 0x24, 0x108, 0x68);
                SlotInitTagged(g_fm_hint2_def, 0x27, 0x22, 0x108, 0x48);
                SlotInitTagged(g_fm_hint2_def, 0x28, 0x22, 0x108, 0x58);
                SlotInitTagged(g_fm_hint2_def, 0x29, 0x22, 0x108, 0x68);
                SlotSetAnim(0x28, 0, 0, 0, 0x30, 0, 0, 0);
                SlotSetAnim(0x29, 0, 0, 0, 0x60, 0, 0, 0);
                SlotInitTagged(g_fm_prompt_cur_def, 0x2A, 0x23, 0x108,
                               g_menu->list[0].cur * 16 + 0x5A);
                SlotSetFlicker(0x2A, 1);
                g_bg_layers[4].x = 0x38;
                g_bg_layers[4].y = 0x1E;
                g_bg_layers[4].w = 0xF0;
                g_bg_layers[4].h = 0x10;
                g_bg_shown |= 0x10;
                g_persona_data_step++;
                return;
            }
            msg = D_800BA694;
        refuse:
            BgMapInit(msg, 0);
            g_bg_layers[4].x = 0x38;
            g_bg_layers[4].y = 0x1E;
            g_bg_layers[4].w = 0xF0;
            g_bg_layers[4].h = 0x10;
            g_bg_shown |= 0x10;
            D_80076EB0(1);
            goto back;
        }
        if (InputCheckAcceptB(1) || g_menu_allow_hold) {
            if (D_800BC050) {
                FuseItemsOpen();
                g_persona_data_step--;
            } else {
            back:
                BACK_TO_PAIRS();
                g_persona_data_step -= 3;
            }
            g_bg_map1.ncellh = 0x20;
        }
    }
}

/* A Persona's page: from its definition, or from the record of one already
   built. */
void FusePersonaDraw(short id, short unused, short built)
{
    const PersonaDef *d = &g_persona_defs[id];
    Persona          *p = &g_personas[id];
    int               i;

    CellsClear(g_spage_kind_cells, 10);
    CellsClear(g_spage_name_cells, 10);
    if (!built) {
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 5, 25), GLYPH_DIGIT0,
                           FormatDecimal(d->level, g_hud_digits, 2));
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 6, 25), GLYPH_DIGIT0,
                           FormatDecimal(d->sp_cost, g_hud_digits, 3));
        for (i = 0; i < 6; i++) {
            DrawSpellName(d->raw[i], AT(g_tilemap1, 9 + i, 18), 0, 1);
        }
        TileMapWriteRow(g_persona_list_rule, AT(g_tilemap1, 15, 19), 0xD7, 8);
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 24, 17), GLYPH_DIGIT0,
                           FormatDecimal(d->stat[0], g_hud_digits, 2));
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 25, 17), GLYPH_DIGIT0,
                           FormatDecimal(d->stat[1], g_hud_digits, 2));
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 26, 17), GLYPH_DIGIT0,
                           FormatDecimal(d->stat[2], g_hud_digits, 2));
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 27, 17), GLYPH_DIGIT0,
                           FormatDecimal(d->stat[3], g_hud_digits, 2));
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 28, 17), GLYPH_DIGIT0,
                           FormatDecimal(d->stat[4], g_hud_digits, 2));
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 30, 9), GLYPH_DIGIT0,
                           FormatDecimal(d->mag_atk, g_hud_digits, 3));
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 31, 9), GLYPH_DIGIT0,
                           FormatDecimal(d->mag_def, g_hud_digits, 3));
        CellsWriteRow(g_spage_name_cells, d->name, 0, 10);
        CellsWriteRow(g_spage_kind_cells, &g_kind_labels[(d->kind - 1) * 10], 0,
                      10);
        D_8008D924(d, 0);
        D_8008D924(d, 1);
        D_8008D924(d, 2);
        D_8008D924(d, 3);
        D_8008D924(d, 4);
        i = D_800B198B[d->kind];
        TileMapWriteRow(&D_800B1A98[i * 10], AT(g_tilemap1, 30, 18), 0, 10);
        i = func_80098B0C(d->pad27[0]);
        TileMapWriteRow(&D_800B1A98[0x32 + i * 10], AT(g_tilemap1, 31, 18), 0, 10);
        TileMapFillRect(AT(g_tilemap1, 33, 3), 0, 0x19, 1, MAP_W);
        TileMapWriteRow(&g_resist_labels[d->resist * 25], AT(g_tilemap1, 33, 3),
                        0, 0x19);
    } else {
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 5, 25), GLYPH_DIGIT0,
                           FormatDecimal(p->level, g_hud_digits, 2));
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 6, 25), GLYPH_DIGIT0,
                           FormatDecimal(p->sp_cost, g_hud_digits, 3));
        for (i = 0; i < 7; i++) {
            DrawSpellName(p->raw[i], AT(g_tilemap1, 9 + i, 18), 0, 1);
        }
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 24, 17), GLYPH_DIGIT0,
                           FormatDecimal(p->stat[0], g_hud_digits, 2));
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 25, 17), GLYPH_DIGIT0,
                           FormatDecimal(p->stat[1], g_hud_digits, 2));
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 26, 17), GLYPH_DIGIT0,
                           FormatDecimal(p->stat[2], g_hud_digits, 2));
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 27, 17), GLYPH_DIGIT0,
                           FormatDecimal(p->stat[3], g_hud_digits, 2));
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 28, 17), GLYPH_DIGIT0,
                           FormatDecimal(p->stat[4], g_hud_digits, 2));
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 30, 9), GLYPH_DIGIT0,
                           FormatDecimal(p->mag_atk, g_hud_digits, 3));
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, 31, 9), GLYPH_DIGIT0,
                           FormatDecimal(p->mag_def, g_hud_digits, 3));
        CellsWriteRow(g_spage_name_cells, p->name, 0, 10);
        CellsWriteRow(g_spage_kind_cells, &g_kind_labels[(p->kind - 1) * 10], 0,
                      10);
        DrawPersonaStatBars(p);
        i = D_800B198B[p->kind];
        TileMapWriteRow(&D_800B1A98[i * 10], AT(g_tilemap1, 30, 18), 0, 10);
        i = func_80098B0C(g_persona_defs[p->key].pad27[0]);
        TileMapWriteRow(&D_800B1A98[0x32 + i * 10], AT(g_tilemap1, 31, 18), 0, 10);
        TileMapFillRect(AT(g_tilemap1, 33, 3), 0, 0x19, 1, MAP_W);
        TileMapWriteRow(&g_resist_labels[g_persona_defs[p->key].resist * 25],
                        AT(g_tilemap1, 33, 3), 0, 0x19);
    }
}

/* 97.7%: allocation and scheduling differ in the accident case that picks
   an affinity (the image keeps the result pointer in s0 and stores the new
   slot after loading both sources) and in the bonus block. */
#ifdef NON_MATCHING
/* The fusion itself. An accident can strike, more readily for some results
   than others: the result comes out with a new affinity, two points better
   or five points worse in every stat, or as a different Persona altogether.
   The two sources leave the stock, the result goes into the first free
   Persona slot, and the scene that plays the fusion is told what fused. */
void FuseExecute(void)
{
    Persona *personas = g_personas;
    int      n;
    int      m;
    int      p;
    int      q;
    u_short *res;
    short    spell_a;
    short    rank_a;
    short    spell_b;
    short    rank_b;

    MenuStepCursor(&g_menu->list[0]);
    SlotSetPos(0x2A, 0x23, 0x108, g_menu->list[0].cur * 16 + 0x5A);
    MsgStep();
    if (InputCheckAcceptA(1)) {
        if (g_menu->list[0].cur != 0) {
            goto cancel;
        }
        D_800BC5C0 = 0;
        g_fuse_args[4] = 0;
        g_fuse_args[3] = D_800BC050;
        if (func_800A26A0(g_fuse.persona)) {
            g_fuse.flag = 1;
            goto plain;
        }
        switch (g_fuse.unk2) {
        case 0:
            n = 8;
            break;
        case 1:
            n = 16;
            break;
        case 2:
            n = 4;
            break;
        }
        if (rand() % n) {
            goto plain;
        }
        switch (n = func_800A080C(g_fuse.unk2)) {
        case 0: {
            u_short *r = &g_fuse.persona;

            p = PersonaFindFree();
            PersonaFill(p, *r);
            q = p;
            g_persona_slots[PersonaSlotsFree()] = p;
            m = g_persona_stock[g_menu->top.cur];
            func_800AB23C(*r, g_persona_stock[g_menu->status_page.cur],
                          &spell_a, &rank_a);
            func_800AB23C(*r, m, &spell_b, &rank_b);
            if (spell_a == 0xFF) {
                if (spell_b == spell_a) {
                    goto done;
                }
                personas[p].raw[6] = spell_b;
            } else if (spell_b != 0xFF) {
                if (rank_a < rank_b) {
                    personas[p].raw[6] = spell_a;
                } else {
                    personas[p].raw[6] = spell_b;
                }
            } else if (spell_a != spell_b) {
                personas[p].raw[6] = spell_a;
            } else {
                personas[p].raw[6] = spell_b;
            }
            D_800BC5C0 = 1;
            goto done;
        }
        case 1:
            p = PersonaFindFree();
            PersonaFill(p, g_fuse.persona);
            q = p;
            g_persona_slots[PersonaSlotsFree()] = p;
            personas[p].stat[0] += 2;
            personas[p].stat[1] += 2;
            personas[p].stat[2] += 2;
            personas[p].stat[3] += 2;
            personas[p].stat[4] += 2;
            D_800BC5C0 = 1;
            goto done;
        case 2:
            p = PersonaFindFree();
            PersonaFill(p, g_fuse.persona);
            q = p;
            g_persona_slots[PersonaSlotsFree()] = p;
            personas[p].stat[0] -= 5;
            personas[p].stat[1] -= 5;
            personas[p].stat[2] -= 5;
            personas[p].stat[3] -= 5;
            personas[p].stat[4] -= 5;
            D_800BC5C0 = 1;
            goto done;
        case 3:
            p = 0;
            if (D_800BC050) {
                n = D_800BB7F4 * 2 + g_menu->unk2E0.cur + g_menu->unk2D0.cur * 2;
                p = ITEM_AT(n) & ITEM_ID;
            }
            func_800A1990(g_persona_stock[g_menu->status_page.cur],
                          g_persona_stock[g_menu->top.cur], p, &g_fuse, 0x16);
            goto check;
        case 4:
            p = 0;
            if (D_800BC050) {
                n = D_800BB7F4 * 2 + g_menu->unk2E0.cur + g_menu->unk2D0.cur * 2;
                p = ITEM_AT(n) & ITEM_ID;
            }
            func_800A1990(g_persona_stock[g_menu->status_page.cur],
                          g_persona_stock[g_menu->top.cur], p, &g_fuse,
                          func_800A0744());
        check:
            res = &g_fuse.persona;
            if (PersonaFind(*res) != -1) {
                goto plain;
            }
            n = g_persona_defs[*res].level;
            if (g_chars[0].level + 10 < n) {
                goto plain;
            }
            D_800BC5C0 = 1;
            goto build;
        default:
            return;
        }
    plain:
        if (D_800BC050) {
            n = D_800BB7F4 * 2 + g_menu->unk2E0.cur + g_menu->unk2D0.cur * 2;
            func_800A1990(g_persona_stock[g_menu->status_page.cur],
                          g_persona_stock[g_menu->top.cur], ITEM_AT(n) & ITEM_ID,
                          &g_fuse, 0);
        } else {
            func_800A1990(g_persona_stock[g_menu->status_page.cur],
                          g_persona_stock[g_menu->top.cur], 0, &g_fuse, 0);
        }
    build:
        p = PersonaFindFree();
        q = p;
        PersonaFill(p, g_fuse.persona);
        n = PersonaSlotsFree();
        g_persona_slots[n] = p;
    done:
        func_800A0450(p, g_fuse.unk2, g_fuse.unk4);
        if (g_fuse.flag) {
            n = *(u_short *)0x801F1BAC;
            personas[p].stat[0] += *(u_char *)0x801F1BAE;
            personas[p].stat[1] += *(u_char *)0x801F1BB0;
            personas[p].stat[2] += *(u_char *)0x801F1BB2;
            personas[p].mag_atk += n;
            personas[p].stat[3] += *(u_char *)0x801F1BB4;
            personas[p].mag_def += n;
            personas[p].stat[4] += *(u_char *)0x801F1BB6;
            if (*(u_short *)0x801F1BBC) {
                personas[p].raw[6] = *(u_char *)0x801F1BBC;
            }
            n = D_800BB7F4 * 2 + g_menu->unk2E0.cur + g_menu->unk2D0.cur * 2;
            ItemsRemovePending(ITEM_AT(n) & ITEM_ID, 1);
            ItemsCommitPending();
            ItemsCompact();
        }
        if (personas[p].stat[0] == 0 || personas[p].stat[0] >= 0x80) {
            personas[p].stat[0] = 1;
        }
        if (personas[p].stat[1] == 0 || personas[p].stat[1] >= 0x80) {
            personas[p].stat[1] = 1;
        }
        if (personas[p].stat[2] == 0 || personas[p].stat[2] >= 0x80) {
            personas[p].stat[2] = 1;
        }
        if (personas[p].stat[3] == 0 || personas[p].stat[3] >= 0x80) {
            personas[p].stat[3] = 1;
        }
        if (personas[p].stat[4] == 0 || personas[p].stat[4] >= 0x80) {
            personas[p].stat[4] = 1;
        }
        p = func_800A26DC(g_persona_stock[g_menu->status_page.cur],
                          g_persona_stock[g_menu->top.cur]);
        if (p && (personas[q].unk3B & 0xF0) != 0x10) {
            personas[q].unk3B = p;
        }
        g_fuse_scene = 1;
        g_fuse_args[0] = g_fuse.persona;
        g_fuse_args[1] = g_persona_stock[g_menu->status_page.cur];
        g_fuse_args[2] = g_persona_stock[g_menu->top.cur];
        g_persona_stock[g_menu->status_page.cur] = 0;
        g_persona_stock[g_menu->top.cur] = 0;
        if (D_800BC5C0) {
            g_fuse_args[4] = 1;
        }
        if ((g_pad_held[0] & 0x800) && !g_pad_config) {
            g_fuse_scene = 0;
        } else if (g_cutscene_on) {
            LoadFileToAddrAsync("\\ADV\\TYNSE.BIN;1", (void *)0x80118000);
            SsSeqSetDecrescendo(g_seq_handle[0], 0x7F, 0x40);
        }
        g_persona_data_step = STEP_DONE;
        return;
    cancel:
        if (D_800BC050) {
            FuseItemsOpen();
            g_persona_data_step -= 2;
        } else if (D_800BB820 == 0) {
            func_800A11EC();
            SlotInitTagged(D_800B148C, 1, 0x42, g_menu->top.cur * 8 + 0xC8, 0x30);
            SlotInitTagged(D_800B1528, 2, 0x42, 0x20,
                           g_menu->status_page.cur * 12 + 0x30);
            SlotSetFlicker(1, 1);
            SlotSetFlicker(2, 1);
            g_slot_cur = &g_slots[1];
            g_slot_cur->flicker = 0;
            g_slot_cur = &g_slots[2];
            g_slot_cur->flicker = 0;
            g_persona_data_step -= 4;
        } else {
            FusePairsOpen();
            g_persona_data_step -= 4;
        }
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        g_bg_shown ^= 0x10;
        SlotClear(0x24);
        SlotClear(0x25);
        SlotClear(0x26);
        SlotClear(0x27);
        SlotClear(0x28);
        SlotClear(0x29);
        SlotClear(0x2A);
        SlotClear(0x2F);
        g_persona_data_step -= 1;
    }
}
#else
INCLUDE_ASM("adv/nonmatchings/ui/fusion", FuseExecute);
#endif

/* The first free slot of the sixteen, or -1. */
short PersonaSlotsFree(void)
{
    int i;

    for (i = 0; i < PERSONA_SLOTS; i++) {
        if (g_persona_slots[i] == SLOT_EMPTY) {
            return i;
        }
    }
    return -1;
}

/* Closes the gaps: each empty slot takes the next filled one after it. */
void PersonaSlotsCompact(void)
{
    u_char *slots = g_persona_slots;
    u_char  i;
    u_char  j;

    for (i = 0; i < PERSONA_SLOTS; i++) {
        if (slots[i] == SLOT_EMPTY) {
            for (j = i + 1; j < PERSONA_SLOTS; j++) {
                if (slots[j] != SLOT_EMPTY) {
                    slots[i] = slots[j];
                    slots[j] = SLOT_EMPTY;
                    break;
                }
            }
        }
    }
}
