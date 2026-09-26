/* Persona 1 (JP) - fusion by result.  ADV only.
 *   0x8009BCE4 FuseListResults    0x8009BDF8 FuseListHas
 *   0x8009BE48 FuseSearchStep     0x8009BEE4 FuseResultPick
 *   0x8009C30C FusePairPick       0x8009CA6C FuseConfirmStep
 *   0x8009CD00 FuseListPairs      0x8009CE0C FusePairRowDraw
 *   0x8009D120 FusePairsOpen
 *
 * The facility host's kind 8. Every pair of Personas in the stock is fused
 * on paper and the distinct results listed; picking one lists the pairs that
 * make it, and picking a pair shows what it would give - its portrait read
 * off the disc - before the fusion itself (fusion.c) runs.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/menuctx.h>
#include <persona/common/bg.h>
#include <persona/common/persona.h>
#include <persona/adv/personapage.h>

#define STEP_DONE      0xFF
#define PAGE_MARK_SLOT 32

/* What a paper fusion comes out as. */
typedef struct {
    u_short arcana;     /* 0 if the pair cannot be fused */
    u_short unk2;
    u_short unk4;
    u_short persona;
    u_short flag;
} FuseResult;

#define g_fuse (*(FuseResult *)0x801F1B8C)

/* The work lists, reached by address: the distinct results, 0xFF-ended, and
   for the one picked, the stock indices of each pair that gives it. */
#define FUSE_RESULTS_MAX 0x48
#define FUSE_PAIRS_MAX   0x100
#define g_fuse_results ((u_short *)0x80130000)
#define g_fuse_pairs   ((u_short (*)[2])0x80130200)

#define RESULT_ROWS 6
#define PAIR_ROWS   5
#define ROW_H       24

extern short   g_persona_data_step;
extern u_char  g_menu_allow_hold;
extern short   g_item_top;
extern short   g_swap_top;
extern short   D_800B8458;
extern short   g_header_scroll_y;
extern short   D_800BB950;       /* how many results */
extern short   D_800BB95C;       /* how many pairs give the one picked */
extern short   D_800BC050;
extern short   D_800BBB34;
extern volatile int g_cd_busy;
extern u_char  g_kind_labels[];
extern u_char  g_fm_prompt_cur_def[];
extern u_char  g_fm_hint_def[];
extern u_char  g_fm_hint2_def[];
extern u_char  D_800B1D08[];
extern u_char  D_800B2330[];
extern u_char  D_800B1EB8[];
extern u_char  D_800B130C[];
extern u_char  D_800BA66C[];

/* Called without a prototype here: the count comes back as an int. */
extern u_char PageScrollValue(short *value, short lo, short hi, short step);
extern short  MenuScrollCursor(MenuList *m, short *row, short first,
                               short last, u_short *offset);
extern void   StatusPersonaLayout(void);
extern void   func_800A1990(u_char a, u_char b, short mode, FuseResult *out,
                            short special);
extern void   func_800A14C4(short row);
extern short  func_800A2904(void);
extern void   func_800A0E60(void);
extern void   func_800AB1EC(void);
extern void   func_8009DE28(void);
extern void   func_8009F0FC(short persona, short a, short b);
extern void   func_8009E2CC(void);
extern void   func_8009EAB8(void);
extern void   func_8009F7D0(void);

short FuseListHas(short id);
void  FuseResultPick(void);
void  FusePairPick(void);
void  FuseConfirmStep(void);
short FuseListPairs(short id);
void  FusePairRowDraw(short row);
void  FusePairsOpen(void);

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

short FuseListResults(void)
{
    int      i;
    int      j;
    int      n;
    int      count;
    u_short  empty;
    u_short *res;

    empty = 0xFF;
    for (i = FUSE_RESULTS_MAX - 1; i >= 0; i--) {
        g_fuse_results[i] = empty;
    }
    n = PersonaStockCompact() + 1;
    count = 0;
    for (i = 0; i < n; i++) {
        res = &g_fuse.persona;
        for (j = 0; j < n; j++) {
            func_800A1990(g_persona_stock[i], g_persona_stock[j], 0, &g_fuse, 0);
            if (*res && !FuseListHas(*res)) {
                g_fuse_results[count++] = *res;
            }
        }
    }
    return count;
}

short FuseListHas(short id)
{
    int i;

    for (i = 0; g_fuse_results[i] != 0xFF; i++) {
        if (g_fuse_results[i] == id) {
            return 1;
        }
    }
    return 0;
}

void FuseSearchStep(void)
{
    switch (g_persona_data_step) {
    case 0:
        FuseResultPick();
        break;
    case 1:
        FusePairPick();
        break;
    case 2:
        FuseConfirmStep();
        break;
    case 3:
        func_8009E2CC();
        break;
    case 4:
        func_8009EAB8();
        break;
    case 5:
        func_8009F7D0();
        break;
    }
}

void FuseResultPick(void)
{
    int   i;
    short n;

    i = D_800B8458;
    if (D_800BB950 != 0) {
        if (D_800BB950 < RESULT_ROWS) {
            MenuStepCursor(&g_menu->status_who);
        } else {
            if ((short)(g_map_scroll_y % ROW_H) == 0) {
                if (i != 0) {
                    if (g_menu->status_who.delay < 3) {
                        g_menu->status_who.delay = 0;
                    }
                    D_800B8458 = 0;
                }
                if (PageScrollValue(&g_swap_top, 0, D_800BB950 - RESULT_ROWS,
                                    RESULT_ROWS)) {
                    for (i = 0; i < RESULT_ROWS; i++) {
                        func_800A14C4(g_swap_top + i);
                    }
                    g_map_scroll_y = g_swap_top * ROW_H;
                } else if (MenuScrollCursor(&g_menu->status_who, &g_swap_top, 0,
                                            D_800BB950 - RESULT_ROWS,
                                            (u_short *)&D_800B8458)) {
                    if (D_800B8458 < 0) {
                        func_800A14C4(g_swap_top);
                    } else if (D_800B8458 > 0) {
                        func_800A14C4(g_swap_top + RESULT_ROWS - 1);
                    }
                }
            }
            MARKS(g_swap_top, D_800BB950 - RESULT_ROWS);
            g_map_scroll_y += D_800B8458;
        }
    }
    SlotSetPos(1, 0x42, 0x48, g_menu->status_who.cur * ROW_H + 0x30);
    if ((short)(g_map_scroll_y % ROW_H) == 0) {
        if (InputCheckAcceptA(1)) {
            if (D_800BB950 != 0) {
                i = g_swap_top + g_menu->status_who.cur;
                D_800BB95C = n = FuseListPairs(g_fuse_results[i]);
                if (n >= RESULT_ROWS) {
                    MenuListInit(&g_menu->unk030, 0, 0, PAIR_ROWS - 1, 0x14);
                } else {
                    MenuListInit(&g_menu->unk030, 0, 0, n - 1, 0x16);
                }
                g_item_top = 0;
                FusePairsOpen();
                g_persona_data_step += 1;
            }
        } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
            g_persona_data_step = STEP_DONE;
        }
    }
}

void FusePairPick(void)
{
    int   i;
    int   k;
    short cost;

    if (D_800BB95C < PAIR_ROWS) {
        MenuStepCursor(&g_menu->unk030);
    } else {
        if ((short)(g_map_scroll_y % ROW_H) == 0) {
            if (D_800B8458 != 0) {
                if (g_menu->unk030.delay < 3) {
                    g_menu->unk030.delay = 0;
                }
                D_800B8458 = 0;
            }
            if (PageScrollValue(&g_item_top, 0, D_800BB95C - PAIR_ROWS,
                                PAIR_ROWS)) {
                for (i = 0; i < PAIR_ROWS; i++) {
                    FusePairRowDraw(g_item_top + i);
                }
                g_map_scroll_y = g_item_top * ROW_H;
            } else if (MenuScrollCursor(&g_menu->unk030, &g_item_top, 0,
                                        D_800BB95C - PAIR_ROWS,
                                        (u_short *)&D_800B8458)) {
                if (D_800B8458 < 0) {
                    FusePairRowDraw(g_item_top);
                } else if (D_800B8458 > 0) {
                    FusePairRowDraw(g_item_top + PAIR_ROWS - 1);
                }
            }
        }
        MARKS(g_item_top, D_800BB95C - PAIR_ROWS);
        g_map_scroll_y += D_800B8458;
    }
    SlotSetPos(1, 0x42, 0x48, g_menu->unk030.cur * ROW_H + 0x54);
    if ((short)(g_map_scroll_y % ROW_H) == 0) {
        if (InputCheckAcceptA(1)) {
            i = g_item_top + g_menu->unk030.cur;
            g_menu->top.cur = g_fuse_pairs[i][1];
            g_menu->status_page.cur = g_fuse_pairs[i][0];
            func_800A1990(g_persona_stock[g_menu->status_page.cur],
                          g_persona_stock[g_menu->top.cur], 0, &g_fuse, 0);
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
                    SlotSetFlicker(1, 0);
                    SlotSetFlicker(3, 1);
                    SlotInitTagged(D_800B1EB8, 0x2E, 0x24, 0x36, 0xE);
                    BgMapInit(D_800BA66C, 0);
                    g_bg_layers[4].x = 0x38;
                    g_bg_layers[4].y = 0x10;
                    g_bg_layers[4].w = 0xF0;
                    g_bg_layers[4].h = 0x10;
                    g_bg_shown |= 0x10;
                    g_persona_data_step++;
                    return;
                }
                AdvResolveSceneLoc(5, g_fuse.persona, 0);
                CdReadFileToAddrAsync(&g_adv_scene_file, 8, PORTRAIT_READ);
                while (g_cd_busy != -1) {
                    RunFrame();
                }
                TimQueueAt(PORTRAIT_TIM, 0x140, 0x168, 0, 0x1E6);
                g_bg_map1.ncellh = 0x40;
                SlotClearAll();
                SlotInitTagged(D_800B1D08, 0x3C, 0x300, 0x18, 0x18);
                SlotInitTagged(D_800B2330, 0x2D, 0x2FF, 0, 0x10);
                SlotSetAnim(0x2D, 0, 0, 0, 0x60, 0xC, 0, 0);
                MenuListInit(&g_menu->page, 0, 0, 1, 0x14);
                StatusPersonaLayout();
                func_8009F0FC(g_fuse.persona, 1, 0);
                g_persona_data_step += 3;
            }
        } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
            func_800A0E60();
            g_persona_data_step -= 1;
        }
    }
}

void FuseConfirmStep(void)
{
    MenuStepCursor(&g_menu->list[1]);
    SlotSetPos(3, 0x23, 0xF0, g_menu->list[1].cur * 16 + 0x4A);
    MsgStep();
    if (InputCheckAcceptA(1)) {
        if (g_menu->list[1].cur == 0) {
            D_800BC050++;
            func_8009DE28();
            g_persona_data_step++;
            return;
        }
        AdvResolveSceneLoc(5, g_fuse.persona, 0);
        CdReadFileToAddrAsync(&g_adv_scene_file, 8, PORTRAIT_READ);
        while (g_cd_busy != -1) {
            RunFrame();
        }
        TimQueueAt(PORTRAIT_TIM, 0x140, 0x168, 0, 0x1E6);
        SlotClearAll();
        SlotInitTagged(D_800B1D08, 0x3C, 0x300, 0x18, 0x18);
        SlotInitTagged(D_800B2330, 0x2D, 0x2FF, 0, 0x10);
        SlotSetAnim(0x2D, 0, 0, 0, 0x60, 0xC, 0, 0);
        MenuListInit(&g_menu->page, 0, 0, 1, 0x14);
        StatusPersonaLayout();
        func_8009F0FC(g_fuse.persona, 0, 0);
        g_bg_map1.ncellh = 0x40;
        g_persona_data_step += 2;
        return;
    }
    if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        func_800AB1EC();
        SlotClear(0x2E);
        SlotSetFlicker(1, 1);
        g_bg_shown ^= 0x10;
        g_persona_data_step--;
    }
}

short FuseListPairs(short id)
{
    int     i;
    int     j;
    int     last;
    int     count;
    u_short empty;

    empty = 0xFFFF;
    for (i = FUSE_PAIRS_MAX - 1; i >= 0; i--) {
        ((u_short *)g_fuse_pairs)[i] = empty;
    }
    count = 0;
    last = PersonaStockCompact();
    for (i = 0; i <= last; i++) {
        for (j = 0; j <= last; j++) {
            func_800A1990(g_persona_stock[i], g_persona_stock[j], 0, &g_fuse, 0);
            if (g_fuse.persona == id) {
                g_fuse_pairs[count][0] = i;
                g_fuse_pairs[count][1] = j;
                count++;
            }
        }
    }
    return count;
}

/* Two rows a pair: each Persona's arcana, level and name. */
void FusePairRowDraw(short row)
{
    int      r;
    short   *line;
    int      k;
    u_char   p;

    r = row & 0xF;
    line = AT(g_tilemap1, r * 2, 0);
    TileMapFillRect(line, 0, 0x1C, 2, MAP_W);
    k = g_fuse_pairs[row][0];
    if (k != 0xFFFF) {
        *AT(g_tilemap1, r * 2, 0) = k + 0x418;
        *AT(g_tilemap1, r * 2, 25) = 0xD1;
        p = g_persona_stock[k];
        TileMapWriteRow(&g_arcana_labels[(g_persona_data[p].arcana - 1) * 6],
                        AT(g_tilemap1, r * 2, 2), 0, 6);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, r * 2, 9), 0x383, 2);
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, r * 2, 12), 0xC0,
            FormatDecimal(g_persona_data[p].level, g_hud_digits, 2));
        DrawPersonaName(p, AT(g_tilemap1, r * 2, 14), 0);
        k = g_fuse_pairs[row][1];
        *AT(g_tilemap1, r * 2 + 1, 0) = k + 0x418;
        p = g_persona_stock[k];
        TileMapWriteRow(&g_arcana_labels[(g_persona_data[p].arcana - 1) * 6],
                        AT(g_tilemap1, r * 2 + 1, 2), 0, 6);
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, r * 2 + 1, 9), 0x383, 2);
        TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, r * 2 + 1, 12), 0xC0,
            FormatDecimal(g_persona_data[p].level, g_hud_digits, 2));
        DrawPersonaName(p, AT(g_tilemap1, r * 2 + 1, 14), 0);
        return;
    }
    TileMapFillRect(line, 0x1A3, 1, 2, MAP_W);
    TileMapFillRect(AT(g_tilemap1, r * 2, 2), 0x1A3, 6, 2, MAP_W);
    TileMapFillRect(AT(g_tilemap1, r * 2, 9), 0x1A3, 4, 2, MAP_W);
    TileMapFillRect(AT(g_tilemap1, r * 2, 14), 0x1A3, 10, 2, MAP_W);
}

void FusePairsOpen(void)
{
    int i;
    int id;

    func_8008EDBC(0x1F);
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap2, 0, MAP_W, 0x40, MAP_W);
    TileMapDrawWindow(g_tilemap0, 0x20, 0x12, MAP_W);
    TileMapDrawBox(g_tilemap0 + MAP_W + 1, 0x1E, 0x10, MAP_W);
    TileMapWriteBar(AT(g_tilemap0, 3, 2), 0xC);
    TileMapWriteBar(AT(g_tilemap0, 3, 14), 4);
    TileMapWriteBar(AT(g_tilemap0, 3, 18), 0xC);
    TileMapFillRect(AT(g_tilemap0, 2, 2), 0x17, 0x1C, 1, MAP_W);
    for (i = 0; i < PAIR_ROWS; i++) {
        TileMapFillRect(AT(g_tilemap0, i * 2 + 6, 2), 0x17, 0x1C, 1, MAP_W);
        TileMapWriteBar(AT(g_tilemap0, i * 2 + 7, 2), 0xC);
        TileMapWriteBar(AT(g_tilemap0, i * 2 + 7, 14), 4);
        TileMapWriteBar(AT(g_tilemap0, i * 2 + 7, 18), 0xC);
        FusePairRowDraw(g_item_top + i);
    }
    do {
        i = g_swap_top + g_menu->status_who.cur;
        id = g_fuse_results[i];
    } while (0);
    TileMapWriteRow(&g_kind_labels[(g_persona_defs[id].kind - 1) * 10],
                    g_tilemap2, 0, 10);
    TileMapWriteRow(str_cell_run, AT(g_tilemap2, 0, 11), 0x383, 2);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap2, 0, 14), 0xC0,
                       FormatDecimal(g_persona_defs[id].level, g_hud_digits, 2));
    TileMapWriteRow(g_persona_defs[id].name, AT(g_tilemap2, 0, 16), 0, 10);
    SlotClearAll();
    SlotInitTagged(D_800B1D08, 0x3C, 8, 0x18, 0x18);
    SlotInitTagged(D_800B2330, 0x2D, 7, 0, 0x10);
    SlotSetAnim(0x2D, 0, 0, 0, 0x30, 0x24, 0, 0);
    if (D_800BB95C >= RESULT_ROWS) {
        SlotInitTagged(g_pdata_mark_up_def, PAGE_MARK_SLOT, 0x42, 0xB0, 0x48);
        SlotInitTagged(g_pdata_mark_down_def, PAGE_MARK_SLOT + 1, 0x42, 0xB0,
                       0xCC);
        MARKS(g_item_top, D_800BB95C - PAIR_ROWS);
    }
    SlotInitTagged(D_800B130C, 1, 0x42, 0x48, g_menu->unk030.cur * ROW_H + 0x54);
    SlotSetFlicker(1, 1);
    D_800B8458 = 0;
    g_cam_y = 0;
    g_header_scroll_y = 0;
    g_bg_map1.ncellh = 0x20;
    g_map_scroll_y = g_item_top * ROW_H;
}
