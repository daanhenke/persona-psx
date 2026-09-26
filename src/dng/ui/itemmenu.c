/* Persona 1 (JP) - the item menu's steps.  DNG's copy.
 *   0x8007A49C ItemMemberPick   0x8007A670 ItemBagOpen
 *   0x8007AB90 ItemBagStep      0x8007B21C ItemSwapStep
 *
 * The field's build of ADV's item menu (src/adv/ui/itemmenu.c): the same
 * source against the field's callees and data. Where ADV spells out the way
 * back to the menu's command list, the field calls ItemMenuOpen, and it calls
 * PageScrollValue where ADV's copy has it expanded.
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
#include <persona/common/formation.h>
#include <persona/adv/personapage.h>

extern short   g_menu_subsel;
extern u_char  g_fm_mark_def[];
extern short   g_fm_mark_pos[][2];
extern short   g_header_scroll_y;
extern short   D_8009FE20;
extern short   D_800A04D4;
extern u_char  D_8009AA4C[];
extern u_char  D_8009B074[];

extern void   DrawStatusHud(void);
extern int    MenuStepMember(int *sel, u_char last);
extern void   EquipScreen(short standalone);
extern void   ItemMenuOpen(void);
/* The field's message stepper. */
extern int    func_80076380(void);
extern u_char PageScrollValue(short *value, short lo, short hi, short step);
extern void   ItemsMergePending(void);
extern void   CopyShorts(u_short *src, u_short *dst, u_short count);
extern void   DrawItemCell(short *dst, int col, int row, int bank);
extern void   DrawItemName(int id, short *dst, u_short base, int b);
extern void   TextItemStatRow(short item, short x, short y);

/* The work list the bag is drawn from, and the bag in the save game. An entry
   is an item id in the low nine bits and a count above them. */
#define g_item_list ((u_short *)0x800EAE4C)
#define g_items     ((u_short *)0x801F267C)
#define BAG_SIZE    0x17F
#define ITEM_ID     0x1FF
/* The last scroll row the bag's first line can show. */
#define ITEM_TOP_MAX 0xB4
#define ITEM_ROW_H   12
#define ITEM_PAGE    11

extern u_short  g_key_page_back;
extern u_short  g_key_page_fwd;
extern MenuList g_page_back_repeat;
extern MenuList g_page_fwd_repeat;
extern int      g_pad_held[];
extern short    g_item_scroll_step;
extern u_char   str_nothing[];

extern void  SoundPlaySeq(u_short slot, u_short seq, short vab);
extern int   MenuScrollCursor(MenuList *m, short *row, short first, short last,
                              u_short *offset);
extern void  MenuResetRepeat(MenuList *m);

extern short   g_item_top;
/* The swap cursor's scroll row. */
extern short   g_swap_top;
extern u_char  D_8009A5D0[];
extern u_char  D_80099FEC[];
extern u_char  D_8009ABFC[];

/* The member whose items are shown, a frame. */
void ItemMemberPick(void)
{
    short *x;
    short *y;

    DrawStatusHud();
    if (MenuStepMember(&g_menu->unk050.cur, g_party_last)) {
        g_header_scroll_y = 0;
        D_8009FE20 = 0;
        D_800A04D4 = 0;
        g_menu->unk230.cur = 0;
        g_menu->unk240.cur = 0;
    }
    x = &g_fm_mark_pos[1][0];
    SlotSetPos(1, 0x42, (g_fm_mark_pos + 1)[g_menu->unk050.cur][0],
               (y = x + 1)[g_menu->unk050.cur * 2]);
    if (InputCheckAcceptA(1)) {
        EquipScreen(0);
        ItemMenuOpen();
        SlotClearAll();
        SlotInitTagged(D_8009AA4C, 0x3C, 0x300, 0x18, 0x18);
        SlotInitTagged(D_8009B074, 0x2D, 0x2FF, 0, 0x10);
        SlotSetAnim(0x2D, 0, 0, 0, 0x90, 0, 0, 0);
        SlotClear(0x2F);
        SlotInitTagged(g_fm_mark_def, 1, 0x42, x[g_menu->unk050.cur * 2],
                       y[g_menu->unk050.cur * 2]);
        SlotSetFlicker(1, 1);
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        ItemMenuOpen();
        g_menu_subsel -= 2;
    }
}

/* The item bag, opened: two columns of item names twelve rows high over a
   list that scrolls, with the stats of the item under the cursor. The "use"
   command first merges the pending items and copies the bag into the work
   list the page is drawn from. */
void ItemBagOpen(void)
{
    int      i;
    u_short *item;

    if (MenuStepCursor(&g_menu->unk040)) {
        SlotSetPos(3, 0x23, 0xF0, g_menu->unk040.cur * 16 + 0x4A);
    }
    func_80076380();
    if (InputCheckAcceptA(1)) {
        MenuSetLayers(5);
        TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
        TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
        TileMapDrawWindow(AT(g_tilemap0, 0, 6), 0x1E, 0x11, MAP_W);
        TileMapDrawBox(AT(g_tilemap0, 1, 7), 0x1C, 0xF, MAP_W);
        for (i = 0; i < 11; i++) {
            TileMapWriteBar(AT(g_tilemap0, 2 + i, 8), 10);
            TileMapWriteBar(AT(g_tilemap0, 2 + i, 18), 2);
            TileMapWriteBar(AT(g_tilemap0, 2 + i, 22), 10);
            TileMapWriteBar(AT(g_tilemap0, 2 + i, 32), 2);
            *AT(g_tilemap0, 2 + i, 20) = 0x17;
            *AT(g_tilemap0, 2 + i, 21) = 0x17;
        }
        TileMapWriteBar(AT(g_tilemap0, 14, 22), 12);
        TileMapWriteRow(D_8009A5D0, g_tilemap1, 0, 9);
        if (g_menu->unk040.cur == 0) {
            ItemsMergePending();
            CopyShorts(g_item_list, g_items, BAG_SIZE);
            g_item_top = 0;
            g_swap_top = 0;
            g_menu->item_row.cur = 0;
            g_menu->item_col.cur = 0;
        }
        for (i = 0; i < 12; i++) {
            DrawItemCell(AT(g_tilemap2, (g_item_top + i) & 0x1F, 8), 0,
                         g_item_top + i, 0);
            DrawItemCell(AT(g_tilemap2, (g_item_top + i) & 0x1F, 22), 1,
                         g_item_top + i, 0);
        }
        SlotClear(3);
        SlotClear(8);
        SlotClear(9);
        SlotClear(0xA);
        SlotClear(0xB);
        SlotClear(0xC);
        SlotClear(0xD);
        SlotInitTagged(D_80099FEC, 1, 0x42, g_menu->item_col.cur * 112 + 0x40,
                       g_menu->item_row.cur * 12 + 0x38);
        SlotSetFlicker(1, 1);
        SlotInitTagged(D_8009ABFC, 0x2E, 0x24, 0x2E, 0x10);
        SlotInitTagged(g_pdata_mark_up_def, PAGE_MARK_SLOT, 0x42, 0xA0, 0x36);
        SlotInitTagged(g_pdata_mark_down_def, PAGE_MARK_SLOT + 1, 0x42, 0xA0,
                       0xB0);
        item = &g_item_list[(short)(g_item_top * 2 + g_menu->item_col.cur +
                                    g_menu->item_row.cur * 2)];
        if ((*item & ITEM_ID) && (*item >> 9)) {
            TextItemStatRow(*item & ITEM_ID, 0x30, 0x12);
            DrawItemName(*item & ITEM_ID, AT(g_tilemap1, 0, 11), 0xD7, 1);
        } else {
            TextItemStatRow(0, 0x30, 0x12);
            DrawItemName(0, AT(g_tilemap1, 0, 11), 0xD7, 1);
        }
        g_slot_cur = &g_slots[PAGE_MARK_SLOT];
        g_header_scroll_y = g_item_top * 12;
        if (g_item_top == 0) {
            g_slot_cur->attr |= SLOT_ATTR_HIDE;
        } else {
            g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
        }
        g_slot_cur = &g_slots[PAGE_MARK_SLOT + 1];
        if (g_item_top == ITEM_TOP_MAX) {
            g_slot_cur->attr |= SLOT_ATTR_HIDE;
        } else {
            g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
        }
        g_menu_subsel++;
    }
    if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        ItemMenuOpen();
        g_menu_subsel = 1;
    }
}

/* The item bag, a frame: the cursor walks two columns and the list scrolls
   a row (12 lines) at a time, or a page with the page buttons, redrawing the
   rows that come into view. The item under the cursor has its stats shown;
   accepting it moves on to what to do with it, and backing out writes the
   work list back to the bag. */
void ItemBagStep(void)
{
    int      i;
    short    prev;
    u_short *item;

    prev = g_item_top * 2 + g_menu->item_col.cur + g_menu->item_row.cur * 2;
    if ((short)(g_header_scroll_y % ITEM_ROW_H) == 0) {
        if (g_item_scroll_step != 0) {
            if (g_menu->item_row.delay < 3) {
                g_menu->item_row.delay = 0;
            }
            g_item_scroll_step = 0;
        }
        if (PageScrollValue(&g_item_top, 0, ITEM_TOP_MAX, ITEM_PAGE)) {
            for (i = 0; i < 12; i++) {
                DrawItemCell(AT(g_tilemap2, (g_item_top + i) & 0x1F, 8), 0,
                             g_item_top + i, 0);
                DrawItemCell(AT(g_tilemap2, (g_item_top + i) & 0x1F, 22), 1,
                             g_item_top + i, 0);
            }
            g_header_scroll_y = g_item_top * ITEM_ROW_H;
        } else if (MenuScrollCursor(&g_menu->item_row, &g_item_top, 0,
                                    ITEM_TOP_MAX,
                                    (u_short *)&g_item_scroll_step)) {
            if (g_item_scroll_step < 0) {
                DrawItemCell(AT(g_tilemap2, g_item_top & 0x1F, 8), 0,
                             g_item_top, 0);
                DrawItemCell(AT(g_tilemap2, g_item_top & 0x1F, 22), 1,
                             g_item_top, 0);
            } else if (g_item_scroll_step > 0) {
                DrawItemCell(AT(g_tilemap2, (g_item_top + 10) & 0x1F, 8), 0,
                             g_item_top + 10, 0);
                DrawItemCell(AT(g_tilemap2, (g_item_top + 10) & 0x1F, 22), 1,
                             g_item_top + 10, 0);
            }
        } else {
            MenuStepCursor(&g_menu->item_col);
        }
    } else {
        MenuResetRepeat(&g_menu->item_row);
    }
    g_header_scroll_y += g_item_scroll_step;
    SlotSetPos(1, 0x42, g_menu->item_col.cur * 112 + 0x40,
               g_menu->item_row.cur * 12 + 0x38);
    g_slot_cur = &g_slots[PAGE_MARK_SLOT];
    if (g_item_top == 0) {
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
    } else {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }
    g_slot_cur = &g_slots[PAGE_MARK_SLOT + 1];
    if (g_item_top == ITEM_TOP_MAX) {
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
    } else {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }

    if (prev != g_item_top * 2 + g_menu->item_col.cur + g_menu->item_row.cur * 2) {
        prev = g_item_top * 2 + g_menu->item_col.cur + g_menu->item_row.cur * 2;
        TileMapFillRect(AT(g_tilemap1, 0, 11), 0, 10, 1, MAP_W);
        item = &g_item_list[prev];
        if ((*item & ITEM_ID) && (*item >> 9)) {
            TextItemStatRow(*item & ITEM_ID, 0x30, 0x12);
            DrawItemName(*item & ITEM_ID, AT(g_tilemap1, 0, 11), 0xD7, 1);
        } else {
            TextItemStatRow(0, 0x30, 0x12);
            DrawItemName(0, AT(g_tilemap1, 0, 11), 0xD7, 1);
        }
    }
    func_80076380();
    if (InputCheckAcceptA(1)) {
        u_short *it;

        MenuListInit(&g_menu->list[0], g_menu->item_row.cur, 0, 10, 0x14);
        MenuListInit(&g_menu->list[1], g_menu->item_col.cur, 0, 1, 0x1A);
        TileMapFillRect(AT(g_tilemap1, 0, 11), 0, 10, 1, MAP_W);
        it = &g_item_list[prev];
        if ((*it & ITEM_ID) && (*it >> 9)) {
            TextItemStatRow(*it & ITEM_ID, 0x30, 0x12);
            DrawItemName(*it & ITEM_ID, AT(g_tilemap1, 0, 11), 0x1AE, 1);
        } else {
            TextItemStatRow(0, 0x30, 0x12);
            TileMapWriteRow(str_nothing, AT(g_tilemap1, 0, 12), 0x1AE, 7);
        }
        {
            int col = g_menu->item_col.cur;

            i = g_item_top + g_menu->item_row.cur;
            DrawItemCell(AT(g_tilemap2, i & 0x1F, 8) + col * 14, col, i, 2);
        }
        g_swap_top = g_item_top;
        g_menu_subsel++;
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        CopyShorts(g_item_list, g_items, BAG_SIZE);
        ItemMenuOpen();
        g_menu_subsel = 1;
    }
}

/* The picked item's index, kept in `sel` for the row's second cell. */
#define PICKED()                                                              \
    (sel = g_item_top * 2 + g_menu->item_col.cur + g_menu->item_row.cur * 2)

/* Moving an item, a frame: a second cursor with its own scroll picks where
   the item picked in the bag goes. Accepting swaps the two entries; backing
   out redraws the bag as it was. */
void ItemSwapStep(void)
{
    int      i;
    int      sel;
    int      j;
    short    prev;
    u_short *item;

    prev = g_swap_top * 2 + g_menu->list[1].cur + g_menu->list[0].cur * 2;
    if ((short)(g_header_scroll_y % ITEM_ROW_H) == 0) {
        if (g_item_scroll_step != 0) {
            if (g_menu->list[0].delay < 3) {
                g_menu->list[0].delay = 0;
            }
            g_item_scroll_step = 0;
        }
        if (PageScrollValue(&g_swap_top, 0, ITEM_TOP_MAX, ITEM_PAGE)) {
            for (i = 0; i < 11; i++) {
                j = (g_swap_top + i) * 2;
                DrawItemCell(AT(g_tilemap2, (g_swap_top + i) & 0x1F, 8), 0,
                             g_swap_top + i, (PICKED() == j) * 2);
                j = (g_swap_top + i) * 2 + 1;
                DrawItemCell(AT(g_tilemap2, (g_swap_top + i) & 0x1F, 22), 1,
                             g_swap_top + i, (sel == j) * 2);
            }
            g_header_scroll_y = g_swap_top * ITEM_ROW_H;
        } else if (MenuScrollCursor(&g_menu->list[0], &g_swap_top, 0,
                                    ITEM_TOP_MAX,
                                    (u_short *)&g_item_scroll_step)) {
            if (g_item_scroll_step < 0) {
                j = g_swap_top * 2;
                DrawItemCell(AT(g_tilemap2, g_swap_top & 0x1F, 8), 0,
                             g_swap_top, (PICKED() == j) * 2);
                j = g_swap_top * 2 + 1;
                DrawItemCell(AT(g_tilemap2, g_swap_top & 0x1F, 22), 1,
                             g_swap_top, (sel == j) * 2);
            } else if (g_item_scroll_step > 0) {
                j = (g_swap_top + 10) * 2;
                DrawItemCell(AT(g_tilemap2, (g_swap_top + 10) & 0x1F, 8), 0,
                             g_swap_top + 10,
                             ((sel = (g_item_top + g_menu->item_row.cur) * 2 +
                                     g_menu->item_col.cur) == j) * 2);
                j = (g_swap_top + 10) * 2 + 1;
                DrawItemCell(AT(g_tilemap2, (g_swap_top + 10) & 0x1F, 22), 1,
                             g_swap_top + 10, (sel == j) * 2);
            }
        } else {
            MenuStepCursor(&g_menu->list[1]);
        }
    } else {
        MenuResetRepeat(&g_menu->list[0]);
    }
    g_header_scroll_y += g_item_scroll_step;
    SlotSetPos(1, 0x42, g_menu->list[1].cur * 112 + 0x40,
               g_menu->list[0].cur * 12 + 0x38);
    g_slot_cur = &g_slots[PAGE_MARK_SLOT];
    if (g_swap_top == 0) {
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
    } else {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }
    g_slot_cur = &g_slots[PAGE_MARK_SLOT + 1];
    if (g_swap_top == ITEM_TOP_MAX) {
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
    } else {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }

    if (prev != g_swap_top * 2 + g_menu->list[1].cur + g_menu->list[0].cur * 2) {
        u_short *p;

        prev = g_swap_top * 2 + g_menu->list[1].cur + g_menu->list[0].cur * 2;
        p = &g_item_list[prev];
        if ((*p & ITEM_ID) && (*p >> 9)) {
            TextItemStatRow(*p & ITEM_ID, 0x30, 0x12);
        } else {
            TextItemStatRow(0, 0x30, 0x12);
        }
    }
    func_80076380();
    if (InputCheckAcceptA(1)) {
        sel = g_item_top * 2 + g_menu->item_col.cur + g_menu->item_row.cur * 2;
        if (sel != prev) {
            u_short *a;
            u_short *b;
            u_short  u;

            i = g_swap_top * 2 + g_menu->list[1].cur + g_menu->list[0].cur * 2;
            a = &g_item_list[i];
            j = *a;
            b = &g_item_list[sel];
            u = *b;
            g_item_top = g_swap_top;
            *a = u;
            *b = j;
            for (i = 0; i < 12; i++) {
                DrawItemCell(AT(g_tilemap2, (g_item_top + i) & 0x1F, 8), 0,
                             g_item_top + i, 0);
                DrawItemCell(AT(g_tilemap2, (g_item_top + i) & 0x1F, 22), 1,
                             g_item_top + i, 0);
            }
            g_menu->item_row.cur = g_menu->list[0].cur;
            g_menu->item_col.cur = g_menu->list[1].cur;
            SlotSetPos(1, 0x42, g_menu->item_col.cur * 112 + 0x40,
                       g_menu->item_row.cur * 12 + 0x2C);
            i = g_item_top * 2 + g_menu->item_col.cur + g_menu->item_row.cur * 2;
            TileMapFillRect(AT(g_tilemap1, 0, 11), 0, 10, 1, MAP_W);
            item = &g_item_list[i];
            if ((*item & ITEM_ID) && (*item >> 9)) {
                DrawItemName(*item & ITEM_ID, AT(g_tilemap1, 0, 11), 0xD7, 0);
                TextItemStatRow(*item & ITEM_ID, 0x30, 0x12);
            } else {
                DrawItemName(0, AT(g_tilemap1, 0, 11), 0xD7, 1);
                TextItemStatRow(0, 0x30, 0x12);
            }
            g_menu_subsel--;
        }
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        i = g_item_top * 2 + g_menu->item_col.cur + g_menu->item_row.cur * 2;
        TileMapFillRect(AT(g_tilemap1, 0, 11), 0, 10, 1, MAP_W);
        if (i != prev) {
            item = &g_item_list[i];
            if ((*item & ITEM_ID) && (*item >> 9)) {
                TextItemStatRow(*item & ITEM_ID, 0x30, 0x12);
                DrawItemName(*item & ITEM_ID, AT(g_tilemap1, 0, 11), 0xD7, 0);
            } else {
                TextItemStatRow(0, 0x30, 0x12);
                DrawItemName(0, AT(g_tilemap1, 0, 11), 0xD7, 1);
            }
        }
        for (i = 0; i < 11; i++) {
            DrawItemCell(AT(g_tilemap2, (g_item_top + i) & 0x1F, 8), 0,
                         g_item_top + i, 0);
            DrawItemCell(AT(g_tilemap2, (g_item_top + i) & 0x1F, 22), 1,
                         g_item_top + i, 0);
        }
        g_header_scroll_y = g_item_top * ITEM_ROW_H;
        g_menu_subsel--;
    }
}
