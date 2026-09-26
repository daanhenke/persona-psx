/* Persona 1 (JP) - the item menu's command list and the use list.  DNG only.
 *   0x80078F94 ItemMenuStep   0x80079068 ItemMenuOpen
 *   0x800790C8 ItemTopStep    0x800797CC ItemUseStep
 *   0x80079E3C ItemTargetStep
 *
 * The command list opens one of three screens: the use list (two columns of
 * the bag six rows high, an item greyed when it would do nothing now), the
 * member whose equipment is shown, or the bag sorted by hand. The use list
 * scrolls a row or a page at a time; accepting an item puts the target
 * marker on a member, or on every member for an item that treats the whole
 * party. The bag and member screens are in itemmenu.c.
 */
#define SLOT_SETPOS_INT
#define SLOT_TAGGED_INTXY
#define TILEMAP_INT_COUNT
#define PERSONAPAGE_DNG
#define SLOT_TAGGED_INT
#define SLOT_FLICKER_INT
#define SLOT_CLEAR_INT
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/menuctx.h>
#include <persona/common/formation.h>
#include <persona/common/item.h>
#include <persona/common/bg.h>
#include <persona/adv/personapage.h>

/* The work list the pages are drawn from, and the bag in the save game. */
#define g_item_list ((u_short *)0x800EAE4C)
#define g_items     ((u_short *)0x801F267C)
#define BAG_SIZE    0x17F
#define ITEM_ID     0x1FF
#define ITEM_ROW_H  12

/* The use list: six rows on screen, the last first row it scrolls to, and a
   page. */
#define USE_ROWS    6
#define USE_TOP_MAX 0xB9
#define USE_PAGE    6

/* ItemDef.unk06: the item treats the whole party, and it is used up. */
#define ITEM_WHOLE_PARTY 0x200
#define ITEM_CONSUMED    0x4000

/* Two items that go to work without a target. */
#define ITEM_NO_TARGET_A 0xF
#define ITEM_NO_TARGET_B 0x12

/* The item menu's commands. */
#define ITEM_USE    0
#define ITEM_EQUIP  1
#define ITEM_SORT   2

/* The entry under the use list's cursor. */
#define USE_SEL() \
    (g_use_top * 2 + g_menu->use_col.cur + g_menu->use_row.cur * 2)

extern short   g_menu_subsel;
extern short   g_menu_sel;
extern u_char  g_menu_blink;
extern short   g_header_scroll_y;
/* The use list's first row and its scroll step. */
extern short   g_use_top;
extern short   g_use_scroll_step;
extern u_char  g_fm_mark_def[];
extern short   g_fm_mark_pos[][2];
extern u_char  g_fm_prompt_cur_def[];
extern u_char  g_fm_hint_def[];
extern u_char  g_fm_hint2_def[];
extern u_char  D_80099FEC[];
extern u_char  D_8009A5BC[];
extern u_char  D_8009AA4C[];
extern u_char  D_8009ABFC[];
extern u_char  D_8009B074[];

extern void   DrawStatusHud(void);
extern void   func_80086E20(int a, int b);
extern void   MenuTopRedraw(void);
extern void   func_80086B08(int a, int b);
extern void   func_800891A8(int a, int b);
extern void   ItemUseLayout(void);
extern void   func_80092E5C(int);
extern void   func_800929D8(void);
extern void   func_8008E948(int member, int item);
extern void   CopyShorts(u_short *src, u_short *dst, u_short count);
extern void   DrawItemRowUsable(short slot, short *dst);
extern void   TextSlotStatRow(short slot);
extern void   BgMapInit(void *script, short speed);
extern void   ItemsCompact(void);
extern void   ItemsRemovePending(int item, int count);
extern int    SpellUsable(int item);
extern int    CharSpellUsable(int member, int item);
extern int    MenuStepMember(int *sel, u_char last);
extern u_char PageScrollValue(short *value, short lo, short hi, short step);
extern int    MenuScrollCursor(MenuList *m, short *row, short first, short last,
                               u_short *offset);
extern void   MenuResetRepeat(MenuList *m);
/* The field's message stepper. */
extern int    func_80076380(void);

extern void ItemMemberPick(void);
extern void ItemBagOpen(void);
extern void ItemBagStep(void);
extern void ItemSwapStep(void);

void ItemMenuOpen(void);
void ItemTopStep(void);
void ItemUseStep(void);
void ItemTargetStep(void);

void ItemMenuStep(void)
{
    switch (g_menu_subsel) {
    case 0:
        ItemMenuOpen();
        g_menu_subsel++;
        break;
    case 1:
        ItemTopStep();
        break;
    case 2:
        ItemUseStep();
        break;
    case 3:
        ItemMemberPick();
        break;
    case 4:
        ItemBagOpen();
        break;
    case 5:
        ItemBagStep();
        break;
    case 6:
        ItemSwapStep();
        break;
    case 7:
        ItemTargetStep();
        break;
    }
}

void ItemMenuOpen(void)
{
    MenuTopRedraw();
    SlotSetAnim(0x2D, 0, 0, 0, 0x30, 0, 0, 0);
    func_80086B08(0, 1);
    func_800891A8(0, 3);
}

/* The item menu's command list, a frame. */
void ItemTopStep(void)
{
    int   i = 0;
    short prev;

    prev = USE_SEL();
    if (MenuStepCursor(&g_menu->unk030)) {
        func_80086E20(0, 3);
    }
    DrawStatusHud();
    if (InputCheckAcceptA(2)) {
        switch (g_menu->unk030.cur) {
        case ITEM_USE:
            SlotClearAll();
            func_80092E5C(2);
            TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
            TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
            TileMapFillRect(g_tilemap2, 0, MAP_W, 0x40, MAP_W);
            ItemUseLayout();
            CopyShorts(g_items, g_item_list, BAG_SIZE);
            for (; i < USE_ROWS; i++) {
                DrawItemRowUsable((g_use_top + i) * 2,
                                  AT(g_tilemap2, (g_use_top + i) & 0x1F, 0));
                DrawItemRowUsable((g_use_top + i) * 2 + 1,
                                  AT(g_tilemap2, (g_use_top + i) & 0x1F, 14));
            }
            g_header_scroll_y = g_use_top * ITEM_ROW_H;
            TextSlotStatRow(prev);
            SlotClear(0x2F);
            SlotInitTagged(D_8009ABFC, 0x2E, 0x24, 0x36, 0xC);
            SlotInitTagged(D_8009AA4C, 0x3C, 0x300, 0x18, 0x18);
            SlotInitTagged(D_8009B074, 0x2D, 0x2FF, 0, 0x10);
            SlotInitTagged(g_pdata_mark_up_def, PAGE_MARK_SLOT, 0x42, 0xA8,
                           0x30);
            SlotInitTagged(g_pdata_mark_down_def, PAGE_MARK_SLOT + 1, 0x42,
                           0xA8, 0x6C);
            SlotSetAnim(0x2D, 0, 0, 0, 0x60, 0, 0, 0);
            SlotInitTagged(D_80099FEC, 1, 0x42,
                           g_menu->use_col.cur * 112 + 0x48,
                           g_menu->use_row.cur * 12 + 0x30);
            SlotSetFlicker(1, 1);
            g_slot_cur = &g_slots[PAGE_MARK_SLOT];
            if (g_use_top == 0) {
                g_slot_cur->attr |= SLOT_ATTR_HIDE;
            } else {
                g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
            }
            g_slot_cur = &g_slots[PAGE_MARK_SLOT + 1];
            if (g_use_top == USE_TOP_MAX) {
                g_slot_cur->attr |= SLOT_ATTR_HIDE;
            } else {
                g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
            }
            g_menu_subsel++;
            break;
        case ITEM_EQUIP:
            SlotClearAll();
            SlotInitTagged(D_8009AA4C, 0x3C, 0x300, 0x18, 0x18);
            SlotInitTagged(D_8009B074, 0x2D, 0x2FF, 0, 0x10);
            SlotSetAnim(0x2D, 0, 0, 0, 0x90, 0, 0, 0);
            SlotInitTagged(g_fm_mark_def, 1, 0x42,
                           (g_fm_mark_pos + 1)[g_menu->unk050.cur][0],
                           (g_fm_mark_pos + 1)[g_menu->unk050.cur][1]);
            SlotSetFlicker(1, 1);
            g_menu_subsel += 2;
            break;
        case ITEM_SORT:
            SlotClearAll();
            SlotInitTagged(D_8009AA4C, 0x3C, 0x300, 0x18, 0x18);
            SlotInitTagged(D_8009B074, 0x2D, 0x2FF, 0, 0x10);
            SlotSetAnim(0x2D, 0, 0, 0, 0xC0, 0, 0, 0);
            CopyShorts(g_items, g_item_list, BAG_SIZE);
            SlotInitTagged(g_fm_prompt_cur_def, 3, 0x23, 0xF0,
                           g_menu->unk040.cur * 16 + 0x4A);
            SlotInitTagged(g_fm_hint_def, 9, 0x24, 0xF0, 0x48);
            SlotInitTagged(g_fm_hint_def, 0xA, 0x24, 0xF0, 0x58);
            SlotInitTagged(g_fm_hint2_def, 0xC, 0x22, 0xF0, 0x48);
            SlotInitTagged(g_fm_hint2_def, 0xD, 0x22, 0xF0, 0x58);
            SlotSetAnim(0xC, 0, 0, 0, 0x30, 0, 0, 0);
            SlotSetAnim(0xD, 0, 0, 0, 0x60, 0, 0, 0);
            SlotSetFlicker(3, 1);
            SlotInitTagged(D_8009ABFC, 0x2E, 0x24, 0x36, 0xE);
            BgMapInit(D_8009A5BC, 0);
            g_bg_layers[4].x = 0x38;
            g_bg_layers[4].y = 0x10;
            g_bg_layers[4].w = 0xF0;
            g_bg_layers[4].h = 0x10;
            g_bg_shown = 0x10;
            g_menu_subsel += 3;
            break;
        }
    } else if (InputCheckAcceptB(2) || g_menu_allow_hold) {
        g_menu_blink = 0xFF;
        g_menu_sel = 0;
        g_menu_subsel = 0;
    }
}

/* The use list, a frame: the cursor walks two columns and the list scrolls a
   row (12 lines) at a time, or a page with the page buttons, redrawing the
   rows that come into view. Accepting an item that would do something puts
   the target marker up; backing out writes the bag back. */
void ItemUseStep(void)
{
    int      i;
    short    prev;
    u_short *item;
    u_int    e;
    int      id;
    int      slot;
    int      k;

    prev = USE_SEL();
    DrawStatusHud();
    if ((short)(g_header_scroll_y % ITEM_ROW_H) == 0) {
        if (g_use_scroll_step != 0) {
            if (g_menu->use_row.delay < 3) {
                g_menu->use_row.delay = 0;
            }
            g_use_scroll_step = 0;
        }
        if (PageScrollValue(&g_use_top, 0, USE_TOP_MAX, USE_PAGE)) {
            for (k = 0; k < USE_ROWS; k++) {
                DrawItemRowUsable((g_use_top + k) * 2,
                                  AT(g_tilemap2, (g_use_top + k) & 0x1F, 0));
                DrawItemRowUsable((g_use_top + k) * 2 + 1,
                                  AT(g_tilemap2, (g_use_top + k) & 0x1F, 14));
            }
            g_header_scroll_y = g_use_top * ITEM_ROW_H;
        } else if (MenuScrollCursor(&g_menu->use_row, &g_use_top, 0,
                                    USE_TOP_MAX,
                                    (u_short *)&g_use_scroll_step)) {
            if (g_use_scroll_step < 0) {
                DrawItemRowUsable(g_use_top * 2,
                                  AT(g_tilemap2, g_use_top & 0x1F, 0));
                DrawItemRowUsable(g_use_top * 2 + 1,
                                  AT(g_tilemap2, g_use_top & 0x1F, 14));
            } else if (g_use_scroll_step > 0) {
                DrawItemRowUsable((g_use_top + 5) * 2,
                                  AT(g_tilemap2, (g_use_top + 5) & 0x1F, 0));
                DrawItemRowUsable((g_use_top + 5) * 2 + 1,
                                  AT(g_tilemap2, (g_use_top + 5) & 0x1F, 14));
            }
        } else {
            MenuStepCursor(&g_menu->use_col);
        }
    } else {
        MenuResetRepeat(&g_menu->use_row);
    }
    g_header_scroll_y += g_use_scroll_step;
    SlotSetPos(1, 0x42, g_menu->use_col.cur * 112 + 0x48,
               g_menu->use_row.cur * 12 + 0x30);
    g_slot_cur = &g_slots[PAGE_MARK_SLOT];
    if (g_use_top == 0) {
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
    } else {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }
    g_slot_cur = &g_slots[PAGE_MARK_SLOT + 1];
    if (g_use_top == USE_TOP_MAX) {
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
    } else {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }
    if (prev != USE_SEL()) {
        prev = USE_SEL();
        TextSlotStatRow(prev);
    }
    func_80076380();
    if ((short)(g_header_scroll_y % ITEM_ROW_H) != 0) {
        return;
    }
    if (InputCheckAcceptA(1)) {
        item = &g_item_list[prev];
        e = *item;
        id = e & ITEM_ID;
        if (SpellUsable(id) && (int)e / 512 != 0) {
            i = *item & ITEM_ID;
            SlotSetFlicker(1, 0);
            if (i != ITEM_NO_TARGET_A && i != ITEM_NO_TARGET_B) {
                if (g_item_defs[id].unk06 & ITEM_WHOLE_PARTY) {
                    for (i = 0; i <= g_party_last; i++) {
                        slot = i + 2;
                        SlotInitTagged(g_fm_mark_def, slot, 0x42,
                                       (g_fm_mark_pos + 1)[i][0],
                                       (g_fm_mark_pos + 1)[i][1]);
                        SlotSetFlicker(slot, 1);
                        g_slot_cur = &g_slots[i + 2];
                        g_slot_cur->flicker = 0;
                    }
                } else {
                    MenuListInit(&g_menu->list[1], 0, 0, g_party_last, 0x10);
                    SlotInitTagged(g_fm_mark_def, 2, 0x42, g_fm_mark_pos[1][0],
                                   g_fm_mark_pos[1][1]);
                    SlotSetFlicker(2, 1);
                }
            }
            g_menu_subsel += 5;
        }
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        func_800929D8();
        ItemsCompact();
        g_menu_subsel = 0;
    }
}

/* Using the item, a frame: the target marker walks the party, unless the item
   treats them all or needs no target. Using it redraws the list; the screen
   stays on the item while there are more of it and it would still do
   something. */
void ItemTargetStep(void)
{
    int   i;
    short cur;
    short sel;
    int   id;

    sel = cur = USE_SEL();
    DrawStatusHud();
    func_80076380();
    id = g_item_list[cur] & ITEM_ID;
    if (id == ITEM_NO_TARGET_A || id == ITEM_NO_TARGET_B) {
        goto use;
    }
    if (!(g_item_defs[id].unk06 & ITEM_WHOLE_PARTY) &&
        MenuStepMember(&g_menu->list[1].cur, g_party_last)) {
        SlotSetPos(2, 0x42, (g_fm_mark_pos + 1)[g_menu->list[1].cur][0],
                   (g_fm_mark_pos + 1)[g_menu->list[1].cur][1]);
    }
    if (InputCheckAcceptA(1)) {
        if (g_item_defs[id].unk06 & ITEM_WHOLE_PARTY) {
            for (i = 0; i <= g_party_last; i++) {
                func_8008E948(i, id);
            }
        } else {
            if (!CharSpellUsable(g_menu->list[1].cur, id)) {
                return;
            }
        use:
            func_8008E948(g_menu->list[1].cur, id);
        }
        ItemUseLayout();
        if (g_item_defs[id].unk06 & ITEM_CONSUMED) {
            ItemsRemovePending(id, 1);
        }
        for (i = 0; i < USE_ROWS; i++) {
            DrawItemRowUsable((g_use_top + i) * 2,
                              AT(g_tilemap2, (g_use_top + i) & 0x1F, 0));
            DrawItemRowUsable((g_use_top + i) * 2 + 1,
                              AT(g_tilemap2, (g_use_top + i) & 0x1F, 14));
        }
        if ((g_item_list[sel] >> 9) == 0) {
            TextSlotStatRow(sel);
            goto back;
        }
        if (!SpellUsable(id)) {
            goto back;
        }
        return;
    } else if (!InputCheckAcceptB(1) && !g_menu_allow_hold) {
        return;
    }
back:
    SlotSetFlicker(1, 1);
    for (i = 0; i < 5; i++) {
        SlotClear(i + 2);
    }
    g_menu_subsel -= 5;
}
