/* Persona 1 (JP) - the equipment screen's four steps and its cursor.
 *
 * Compiled into two overlays rather than called across the boundary:
 *                      DNG         ADV
 *   EquipStepMain      0x8008C5C4  0x80091C44
 *   EquipStepList      0x8008CD68  0x8009240C
 *   EquipStepRemove    0x8008D4B0  0x80092B9C
 *   EquipStepOptimise  0x8008D7C4  0x80092ED4
 *   EquipShowCursor    0x8008DADC  0x8009320C
 *
 * Included by each overlay's equipment screen (src/dng/ui/equipscreen.c,
 * src/adv/ui/equipscreen.c) right after EquipScreen, whose includes it uses.
 * ADV's unit (EQUIPSTEPS_ADV) has prototypes for the Char and scroll
 * helpers, where DNG's calls them unprototyped.
 */
#include <persona/common/item.h>
#include <persona/common/char.h>
#include <persona/common/status.h>
#include <persona/common/bg.h>

/* The item list scrolls a row of 12 lines at a time. */
#define LIST_ROW_H 12

extern u_char  g_equip_step;
extern u_char  g_menu_allow_hold;
extern u_char  g_pdata_cursor_def[];
extern u_char  g_pdata_mark_up_def[];
extern u_char  g_pdata_mark_down_def[];
extern u_char  D_8009ABFC[];
extern short   g_header_scroll_y;
extern short   D_8009FE20;
extern short   D_800A04D4;
extern short   g_equip_last;

extern void   EquipScreenLayout(void);
extern void   DrawGauge(int);
extern void   ItemsCompact(void);
#ifdef EQUIPSTEPS_ADV
/* ADV's unit has the prototypes, and narrows at the calls. */
extern void   CharUnequip(u_char chr, u_char slot);
extern void   CharEquip(u_char chr, u_char slot, u_short item);
extern u_char PageScrollValue(short *value, short lo, short hi, short step);
extern short  MenuScrollCursor(MenuList *m, short *row, short first, short last,
                              u_short *offset);
extern void   CharApplyStats(u_char chr);
extern void   CharRecalcStats(u_char chr);
extern void   CharPreviewEquip(Char *c, short slot, short item);
#else
/* Declared without prototypes where DNG's screen calls them: the slot goes
   over as the int it was worked out in. */
extern void   CharUnequip();
extern void   CharEquip();
extern int    PageScrollValue();
extern int    MenuScrollCursor();
extern void   CharApplyStats();
extern void   CharRecalcStats();
extern void   CharPreviewEquip();
#endif
extern void   MenuResetRepeat(MenuList *m);
extern void   CharPreviewDraw(short member, Char *c);
extern void   ItemsCommitPending(void);
extern void   CopyShorts(u_short *src, u_short *dst, u_short count);
extern void   TextItemStatRow(short item, short x, short y);
extern u_short EquipPickStronger(short member, short group);
extern void   CharEquipBest(short member, short group);
extern int    ItemsListUsable(short key, short slot);
extern void   EquipDrawListRow(short row, short *dst);
extern void   bcopy(void *src, void *dst, int n);
extern void   EquipDrawMember(short slot);
extern void   EquipShowCursor(void);
extern void   EquipShowListCursor(void);
extern int    MsgStep(void);

/* Step 0: the member and the row of commands. Accepting a slot row opens
   the item list for it (step 1); the command row opens taking things off
   (step 2) or letting the game choose (step 3).

   The slot rows' `g_equip_step++` (not `+= 1`) loads the step ahead of the
   scroll top, before the tail the three cases share. */
void EquipStepMain(void)
{
    Char  c;
    Char *chars = g_chars;
    int   n;
    u_short *entry;

    if (MenuStepCursor(&g_menu->unk220)) {
        goto reset;
    }
    if (g_menu->unk220.cur == 1) {
        MenuStepCursor(&g_menu->unk250);
    } else if (MenuStepCursor(&g_menu->unk050)) {
        EquipDrawMember(g_menu->unk050.cur);
    reset:
        g_header_scroll_y = 0;
        D_8009FE20 = 0;
        D_800A04D4 = 0;
        g_menu->unk230.cur = 0;
        g_menu->unk240.cur = 0;
    }
    EquipShowCursor();
    if (InputCheckAcceptA(1)) {
        switch (g_menu->unk220.cur) {
        case 1:
            if (g_menu->unk250.cur != 0) {
                DrawGauge(2);
                CopyShorts(g_items, g_items_pending, ITEM_SLOTS);
                MenuListInit(&g_menu->list[1], 0, 0, 7, 0x16);
                n = g_party[g_menu->unk050.cur];
                bcopy(&g_chars[n], &c, sizeof(Char));
                c.equip[0] = 0;
                c.equip[1] = 0;
                c.equip[2] = 0;
                c.equip[3] = 0;
                c.equip[4] = 0;
                c.equip[5] = 0;
                c.equip[6] = 0;
                CharPreviewEquip(&c, 0, 0);
                CharPreviewDraw(n, &c);
                EquipShowListCursor();
                g_equip_step += 2;
            } else {
                DrawGauge(1);
                CopyShorts(g_items, g_items_pending, ITEM_SLOTS);
                MenuListInit(&g_menu->list[1], 0, 0, 7, 0x16);
                n = g_party[g_menu->unk050.cur];
                bcopy(&g_chars[n], &c, sizeof(Char));
                c.equip[0] = EquipPickStronger(n, 0);
                c.equip[1] = EquipPickStronger(n, 1);
                c.equip[2] = EquipPickStronger(n, 2);
                c.equip[3] = EquipPickStronger(n, 3);
                c.equip[4] = EquipPickStronger(n, 4);
                c.equip[5] = EquipPickStronger(n, 5);
                CharPreviewEquip(&c, 6, EquipPickStronger(n, 6));
                CharPreviewDraw(n, &c);
                EquipShowListCursor();
                g_equip_step += 3;
            }
            break;
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            n = g_party[g_menu->unk050.cur];
            g_equip_last = ItemsListUsable(chars[n].key - 1,
                                           g_menu->unk220.cur - 2);
            g_equip_last = g_equip_last < 8 ? 8 : g_equip_last + 1;
            TileMapDrawWindow(AT(g_tilemap0, 0, 8), 0x1E, 7, MAP_W);
            TileMapDrawBox(AT(g_tilemap0, 0, 9), 0x1C, 6, MAP_W);
            for (n = 0; n < 4; n++) {
                TileMapWriteBar(AT(g_tilemap0, 1 + n, 10), 10);
                TileMapWriteBar(AT(g_tilemap0, 1 + n, 20), 2);
                *AT(g_tilemap0, 1 + n, 22) = 0x17;
                *AT(g_tilemap0, 1 + n, 23) = 0x17;
                TileMapWriteBar(AT(g_tilemap0, 1 + n, 24), 10);
                TileMapWriteBar(AT(g_tilemap0, 1 + n, 34), 2);
            }
            for (n = 0; n < 4; n++) {
                EquipDrawListRow((D_800A04D4 + n) * 2,
                                 AT(g_tilemap2, (D_800A04D4 + n) & 0x1F, 0));
                EquipDrawListRow((D_800A04D4 + n) * 2 + 1,
                                 AT(g_tilemap2, (D_800A04D4 + n) & 0x1F, 14));
            }
            SlotInitTagged(g_pdata_cursor_def, 4, 0x42, 0x30, 0x78);
            SlotSetPos(4, 0x42, g_menu->unk240.cur * 112 + 0x50,
                       g_menu->unk230.cur * 12 + 0x24);
            SlotInitTagged(D_8009ABFC, 0x2E, 0x24, 0x3E, 0xD);
            SlotInitTagged(g_pdata_mark_up_def, 0x20, 0x42, 0xAF, 0x22);
            SlotInitTagged(g_pdata_mark_down_def, 0x21, 0x42, 0xAF, 0x49);
            SlotSetFlicker(1, 0);
            SlotSetFlicker(2, 0);
            SlotSetFlicker(3, 0);
            SlotSetFlicker(4, 1);
            entry = &g_items_pending[(short)(D_800A04D4 * 2
                                             + g_menu->unk240.cur
                                             + g_menu->unk230.cur * 2)];
            TextItemStatRow(*entry & ITEM_ID, 0x42, 0xF);
            n = g_party[g_menu->unk050.cur];
            bcopy(&g_chars[n], &c, sizeof(Char));
            CharPreviewEquip(&c, g_menu->unk220.cur - 2, *entry & ITEM_ID);
            CharPreviewDraw(n, &c);
            g_slot_cur = &g_slots[0x20];
            if (D_800A04D4 == 0) {
                g_slot_cur->attr |= SLOT_ATTR_HIDE;
            } else {
                g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
            }
            g_slot_cur = &g_slots[0x21];
            if (D_800A04D4 == g_equip_last / 2 - 4) {
                g_slot_cur->attr |= SLOT_ATTR_HIDE;
            } else {
                g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
            }
            g_header_scroll_y = D_800A04D4 * 12;
            g_equip_step++;
            break;
        }
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        g_equip_step = STEP_DONE;
    }
}

/* Step 1, the item list for one slot, two columns scrolled a row (12 lines)
   at a time or a page with the page buttons, redrawing the rows that come
   into view; the entry under the cursor is tried on in the preview.
   Accepting wears it (or empties the slot) and closes the list, as backing
   out does.

   n is the loop counter, the member, and the accepted entry's index. */
void EquipStepList(void)
{
    Char     c;
    int      n;
    short    prev;
    u_short *entry;

    prev = D_800A04D4 * 2 + g_menu->unk240.cur + g_menu->unk230.cur * 2;
    if ((short)(g_header_scroll_y % LIST_ROW_H) == 0) {
        if (D_8009FE20 != 0) {
            if (g_menu->unk230.delay < 3) {
                g_menu->unk230.delay = 0;
            }
            D_8009FE20 = 0;
        }
        if (PageScrollValue(&D_800A04D4, 0, g_equip_last / 2 - 4, 4)) {
            for (n = 0; n < 4; n++) {
                EquipDrawListRow((D_800A04D4 + n) * 2,
                                 AT(g_tilemap2, (D_800A04D4 + n) & 0x1F, 0));
                EquipDrawListRow((D_800A04D4 + n) * 2 + 1,
                                 AT(g_tilemap2, (D_800A04D4 + n) & 0x1F, 14));
            }
            g_header_scroll_y = D_800A04D4 * LIST_ROW_H;
        } else if (MenuScrollCursor(&g_menu->unk230, &D_800A04D4, 0,
                                    g_equip_last / 2 - 4,
                                    (u_short *)&D_8009FE20)) {
            if (D_8009FE20 < 0) {
                EquipDrawListRow(D_800A04D4 * 2,
                                 AT(g_tilemap2, D_800A04D4 & 0x1F, 0));
                EquipDrawListRow(D_800A04D4 * 2 + 1,
                                 AT(g_tilemap2, D_800A04D4 & 0x1F, 14));
            } else if (D_8009FE20 > 0) {
                EquipDrawListRow((D_800A04D4 + 3) * 2,
                                 AT(g_tilemap2, (D_800A04D4 + 3) & 0x1F, 0));
                EquipDrawListRow((D_800A04D4 + 3) * 2 + 1,
                                 AT(g_tilemap2, (D_800A04D4 + 3) & 0x1F, 14));
            }
        } else {
            MenuStepCursor(&g_menu->unk240);
        }
    } else {
        MenuResetRepeat(&g_menu->unk230);
    }
    g_header_scroll_y += D_8009FE20;
    SlotSetPos(4, 0x42, g_menu->unk240.cur * 112 + 0x50,
               g_menu->unk230.cur * 12 + 0x24);
    if (prev != D_800A04D4 * 2 + g_menu->unk240.cur + g_menu->unk230.cur * 2) {
        entry = &g_items_pending[(short)(D_800A04D4 * 2 + g_menu->unk240.cur
                                         + g_menu->unk230.cur * 2)];
        TextItemStatRow(*entry & ITEM_ID, 0x42, 0xF);
        n = g_party[g_menu->unk050.cur];
        bcopy(&g_chars[n], &c, sizeof(Char));
        CharPreviewEquip(&c, g_menu->unk220.cur - 2, *entry & ITEM_ID);
        CharPreviewDraw(n, &c);
    }
    g_slot_cur = &g_slots[0x20];
    if (D_800A04D4 == 0) {
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
    } else {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }
    g_slot_cur = &g_slots[0x21];
    if (D_800A04D4 == g_equip_last / 2 - 4) {
        g_slot_cur->attr |= SLOT_ATTR_HIDE;
    } else {
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
    }
    MsgStep();
    if ((short)(g_header_scroll_y % LIST_ROW_H) == 0) {
        if (InputCheckAcceptA(1)) {
            /* The index goes through the loop counter; the member has a
               local of its own, set only from the party byte, so combine
               knows it fits the byte it is passed as. */
            u_short *e;
            int      m;

            n = D_800A04D4 * 2 + g_menu->unk240.cur + g_menu->unk230.cur * 2;
            e = &g_items_pending[n];
            m = g_party[g_menu->unk050.cur];
            if ((*e >> 9) && (*e & ITEM_ID)) {
                CharUnequip(m, g_menu->unk220.cur - 2);
                CharEquip(m, g_menu->unk220.cur - 2, *e & ITEM_ID);
            } else {
                CharUnequip(m, g_menu->unk220.cur - 2);
            }
            CharApplyStats(m);
            CharRecalcStats(m);
            EquipDrawMember(g_menu->unk050.cur);
            g_header_scroll_y = 0;
            D_8009FE20 = 0;
            D_800A04D4 = 0;
            g_menu->unk230.cur = 0;
            g_menu->unk240.cur = 0;
            goto close;
        } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        close:
            ItemsCommitPending();
            ItemsCompact();
            TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
            TileMapFillRect(g_tilemap2, 0, MAP_W, 0x20, MAP_W);
            EquipScreenLayout();
            EquipDrawMember(g_menu->unk050.cur);
            DrawGauge(0);
            SlotClear(4);
            SlotClear(4);
            SlotClear(0x20);
            SlotClear(0x21);
            SlotClear(0x2E);
            SlotSetFlicker(1, 1);
            SlotSetFlicker(2, 1);
            SlotSetFlicker(3, 1);
            g_bg_shown ^= 0x10;
            g_equip_step--;
        }
    }
}

/* A copy of the member in party slot `slot` with `what` taken off - one
   slot, or with EQUIP_ALL everything - put up beside them. */
#define PREVIEW_REMOVE(what)                                                       n = g_party[g_menu->unk050.cur];                                               bcopy(&g_chars[n], &c, sizeof(Char));                                          if ((what) == 0) {                                                                 c.equip[0] = 0;                                                                c.equip[1] = 0;                                                                c.equip[2] = 0;                                                                c.equip[3] = 0;                                                                c.equip[4] = 0;                                                                c.equip[5] = 0;                                                                c.equip[6] = 0;                                                                CharPreviewEquip(&c, 0, 0);                                                } else {                                                                           CharPreviewEquip(&c, (what) - 1, 0);                                       }                                                                              CharPreviewDraw(n, &c);                                                        EquipShowListCursor()

/* Step 2, taking things off: row 0 of the list is everything, rows 1-7 one
   slot each. The preview follows either cursor; accepting does it. */
void EquipStepRemove(void)
{
    Char c;
    int  n;

    if (MenuStepCursor(&g_menu->list[1])) {
    preview:
        PREVIEW_REMOVE(g_menu->list[1].cur);
    } else if (MenuStepCursor(&g_menu->unk050)) {
        /* The same preview; the image has it once, behind the first test. */
        goto preview;
    }
    if (InputCheckAcceptA(1)) {
        n = g_party[g_menu->unk050.cur];
        if (g_menu->list[1].cur != 0) {
            CharUnequip(n, g_menu->list[1].cur - 1);
            CharApplyStats(n);
            CharRecalcStats(n);
            EquipDrawMember(g_menu->unk050.cur);
        } else {
            CharUnequip(n, 0);
            CharUnequip(n, 1);
            CharUnequip(n, 2);
            CharUnequip(n, 3);
            CharUnequip(n, 4);
            CharUnequip(n, 5);
            CharUnequip(n, 6);
            CharApplyStats(n);
            CharRecalcStats(n);
            PREVIEW_REMOVE(0);
        }
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        ItemsCommitPending();
        TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
        TileMapFillRect(g_tilemap2, 0, MAP_W, 0x20, MAP_W);
        EquipScreenLayout();
        DrawGauge(0);
        EquipDrawMember(g_menu->unk050.cur);
        SlotSetFlicker(1, 1);
        SlotSetFlicker(2, 1);
        SlotSetFlicker(3, 1);
        g_equip_step -= 2;
    }
}

/* Step 3, letting the game choose: row 0 of the list is every slot, rows
   1-7 one slot each. The preview shows the strongest choice for it;
   accepting puts it on. */
void EquipStepOptimise(void)
{
    Char c;
    int  n;
    int  i;

    if (MenuStepCursor(&g_menu->list[1])) {
    preview:
        n = g_party[g_menu->unk050.cur];
        bcopy(&g_chars[n], &c, sizeof(Char));
        if (g_menu->list[1].cur == 0) {
            c.equip[0] = EquipPickStronger(n, 0);
            c.equip[1] = EquipPickStronger(n, 1);
            c.equip[2] = EquipPickStronger(n, 2);
            c.equip[3] = EquipPickStronger(n, 3);
            c.equip[4] = EquipPickStronger(n, 4);
            c.equip[5] = EquipPickStronger(n, 5);
            CharPreviewEquip(&c, 6, EquipPickStronger(n, 6));
        } else {
            CharPreviewEquip(&c, g_menu->list[1].cur - 1,
                             EquipPickStronger(n, g_menu->list[1].cur - 1));
        }
        CharPreviewDraw(n, &c);
        EquipShowListCursor();
    } else if (MenuStepCursor(&g_menu->unk050)) {
        goto preview;
    }
    if (InputCheckAcceptA(1)) {
        n = g_party[g_menu->unk050.cur];
        if (g_menu->list[1].cur != 0) {
            CharEquipBest(n, g_menu->list[1].cur - 1);
            CharApplyStats(n);
            CharRecalcStats(n);
            ItemsCommitPending();
        } else {
            for (i = 0; i < CHAR_EQUIP; i++) {
                CharEquipBest(n, i);
                ItemsCommitPending();
            }
            CharApplyStats(n);
            CharRecalcStats(n);
        }
        EquipDrawMember(g_menu->unk050.cur);
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        ItemsCommitPending();
        TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
        TileMapFillRect(g_tilemap2, 0, MAP_W, 0x20, MAP_W);
        EquipScreenLayout();
        EquipDrawMember(g_menu->unk050.cur);
        DrawGauge(0);
        SlotSetFlicker(1, 1);
        SlotSetFlicker(2, 1);
        SlotSetFlicker(3, 1);
        g_equip_step -= 3;
    }
}

/* Hides the five cursors, then shows the one for where the list cursor is:
   the member, the slot row, or one of the seven equipment rows. */
void EquipShowCursor(void)
{
    g_slot_cur = &g_slots[1];
    g_slot_cur->attr |= SLOT_ATTR_HIDE;
    g_slot_cur = &g_slots[2];
    g_slot_cur->attr |= SLOT_ATTR_HIDE;
    g_slot_cur = &g_slots[3];
    g_slot_cur->attr |= SLOT_ATTR_HIDE;
    g_slot_cur = &g_slots[4];
    g_slot_cur->attr |= SLOT_ATTR_HIDE;
    g_slot_cur = &g_slots[5];
    g_slot_cur->attr |= SLOT_ATTR_HIDE;
    switch (g_menu->unk220.cur) {
    case 0:
        g_slot_cur = &g_slots[1];
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
        break;
    case 1:
        g_slot_cur = &g_slots[2];
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
        SlotSetPos(2, 0x42, g_menu->unk250.cur * 48 + 0x18, 0x84);
        break;
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        g_slot_cur = &g_slots[3];
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
        SlotSetPos(3, 0x42, 0x28, g_menu->unk220.cur * 12 + 0x78);
        break;
    }
}
