/* Persona 1 (JP) - the equipment screen.  DNG only.
 *   0x8008BF88 EquipScreen
 *
 * The field's build of ADV's screen (src/adv/ui/equipscreen.c): the same
 * loop over the four steps by g_equip_step, with dng's own layout and member
 * drawer. The steps and their helpers follow it.
 */
#define SLOT_SETPOS_INT
#define SLOT_TAGGED_INTXY
#define TILEMAP_INT_COUNT
#define PERSONAPAGE_DNG
#define NAME_KR
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/menuctx.h>
#include <persona/adv/personapage.h>
#include <persona/common/item.h>
#include <persona/common/itemname.h>
#include <persona/common/char.h>
#include <persona/common/status.h>

#define g_seq_handle ((short *)0x801F537C)

#define STEP_DONE 0xFF
#define PAD_TOGGLE 0x100

extern u_char  g_equip_step;
extern int     g_select_toggle;
extern u_char  D_800A083C;
extern u_char  g_menu_allow_hold;
extern u_short g_key_menu_close;
extern int     g_pad_pressed[];
extern u_char  g_party_last;
extern u_char  D_8009AA4C[];
extern u_char  D_8009B074[];
extern u_char  D_8009B330[];
extern u_char  D_80099FEC[];
/* The item ids each equipment slot takes, lowest and highest. */
extern short   g_equip_id_range[][2];
extern u_char  g_pdata_cursor_def[];
extern u_char  g_fm_prompt_cur_def[];

extern u_char PartyLastSlot(void);
extern void   MenuSetLayers(int);
extern void   EquipScreenLayout(void);
extern void   DrawGauge(int);
extern void   SoundOpenSeq(u_short slot, u_short seq, short vab);
extern void   SoundPlaySeq(u_short slot, u_short seq, short vab);
extern void   FadeUpBlocking(short step, short limit);
extern void   FadeDownBlocking(short step, short floor);
extern void   SsSetNck(short handle);
extern void   ItemsCompact(void);
/* Declared without prototypes where the screen calls them: the slot goes
   over as the int it was worked out in. */
extern void   CharUnequip();
extern void   CharApplyStats();
extern void   CharRecalcStats();
extern void   CharPreviewEquip();
extern void   CharPreviewDraw(short member, Char *c);
extern void   func_800929D8(void);
extern u_short EquipPickStronger(short member, short group);
extern void   CharEquipBest(short member, short group);
extern void   bcopy(void *src, void *dst, int n);
extern void   ItemsClearPending(void);
extern void   EquipDrawMember(short slot);
extern void   EquipShowCursor(void);
extern void   func_8008C5C4(void);
extern void   func_8008CD68(void);
extern void   EquipStepRemove(void);
extern void   EquipStepOptimise(void);

void EquipScreen(short standalone)
{
    int i;

    if (standalone) {
        g_party_last = PartyLastSlot();
        MenuListInit(&g_menu->unk050, 0, 0, g_party_last, 0x1A);
        MenuListInit(&g_menu->unk220, 0, 0, 8, 0x16);
        MenuListInit(&g_menu->unk230, 0, 0, 3, 0x14);
        MenuListInit(&g_menu->unk240, 0, 0, 1, 0x1A);
        MenuListInit(&g_menu->unk250, 0, 0, 1, 0x1A);
    }
    MenuSetLayers(3);
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap2, 0, MAP_W, 0x40, MAP_W);
    EquipScreenLayout();
    TileMapWriteRow(D_8009B330, AT(g_tilemap1, 3, 19), 0, 3);
    TileMapWriteRow(D_8009B330 + 3, AT(g_tilemap1, 4, 19), 0, 3);
    TileMapWriteRow(D_8009B330 + 6, AT(g_tilemap1, 5, 19), 0, 3);
    TileMapWriteRow(D_8009B330 + 9, AT(g_tilemap1, 6, 19), 0, 3);
    TileMapWriteRow(D_8009B330 + 12, AT(g_tilemap1, 7, 19), 0, 3);
    DrawGauge(0);
    for (i = 0; i < 7; i++) {
        TileMapWriteRow(str_cell_run, AT(g_tilemap1, 2 + i, 3),
                        0x3B1 + i * 2, 2);
    }
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 2, 27), 0x3B1, 2);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 3, 27), 0x3B1, 2);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 4, 27), 0x3B3, 2);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 5, 27), 0x3B3, 2);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 2, 29), 0x3BF, 3);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 3, 29), 0x3C2, 3);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 4, 29), 0x3BF, 3);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 5, 29), 0x3C2, 3);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 6, 28), 0x3C5, 3);
    TileMapWriteRow(str_cell_run, AT(g_tilemap1, 7, 28), 0x3C8, 3);
    EquipDrawMember(g_menu->unk050.cur);
    SlotClearAll();
    SlotInitTagged(D_8009AA4C, 0x3C, 0x300, 0x18, 0x18);
    SlotInitTagged(D_8009B074, 0x2D, 0x2FF, 0, 0x10);
    SlotSetAnim(0x2D, 0, 0, 0, 0x90, 0, 0, 0);
    SlotInitTagged(g_pdata_cursor_def, 1, 0x42, 0x20, 0x78);
    SlotInitTagged(g_fm_prompt_cur_def, 2, 0x42, 0x18, 0x84);
    SlotInitTagged(g_pdata_cursor_def, 3, 0x42, 0x38, 0x78);
    SlotInitTagged(D_80099FEC, 5, 0x42, 0x18, 0x84);
    SlotSetFlicker(1, 1);
    SlotSetFlicker(2, 1);
    SlotSetFlicker(3, 1);
    SlotSetFlicker(5, 1);
    EquipShowCursor();
    g_equip_step = 0;
    if (standalone) {
        SoundOpenSeq(0x18, 0, 0);
        SoundOpenSeq(0x19, 0, 0);
        SoundOpenSeq(0x1A, 0, 0);
        SoundOpenSeq(0x1B, 0, 0);
        SetDispMask(1);
        FadeUpBlocking(8, 0x80);
    }
    while (g_equip_step != STEP_DONE) {
        RunFrame();
        switch (g_equip_step) {
        case 0:
            func_8008C5C4();
            break;
        case 1:
            func_8008CD68();
            break;
        case 2:
            EquipStepRemove();
            break;
        case 3:
            EquipStepOptimise();
            break;
        }
        if (g_pad_pressed[0] & PAD_TOGGLE) {
            g_select_toggle ^= 1;
        }
        if (!g_menu_allow_hold && (g_key_menu_close & g_pad_pressed[0])) {
            g_menu_allow_hold = 1;
            SoundPlaySeq(0x18, 0, 1);
        }
    }
    if (standalone) {
        FadeDownBlocking(8, 0);
        D_800A083C = 0;
        SsSetNck(g_seq_handle[0x18]);
        SsSetNck(g_seq_handle[0x19]);
        SsSetNck(g_seq_handle[0x1A]);
        SsSetNck(g_seq_handle[0x1B]);
    }
}

INCLUDE_ASM("dng/nonmatchings/ui/equipscreen", func_8008C5C4);

INCLUDE_ASM("dng/nonmatchings/ui/equipscreen", func_8008CD68);

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
        func_800929D8();
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
            func_800929D8();
        } else {
            for (i = 0; i < CHAR_EQUIP; i++) {
                CharEquipBest(n, i);
                func_800929D8();
            }
            CharApplyStats(n);
            CharRecalcStats(n);
        }
        EquipDrawMember(g_menu->unk050.cur);
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        func_800929D8();
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

/* A number right-aligned so its last digit lands in `last`. */
#define MEMBER_NUMBER(value, width, row, last)                                     TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, row, last), GLYPH_DIGIT0,                         FormatDecimal(value, g_hud_digits, width))

/* The two cells of the "unchanged" arrow. */
#define ARROW_SAME 0x360

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

/* The same five cursors for the item list: the list's own when it is on
   the first row, else the row marker beside one of the seven rows. */
void EquipShowListCursor(void)
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
    switch (g_menu->list[1].cur) {
    case 0:
        /* Already slot 5: cse drops the store, but it keeps the pointer
           live into the case and so decides the registers. */
        g_slot_cur = &g_slots[5];
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        g_slot_cur = &g_slots[3];
        g_slot_cur->attr &= ~SLOT_ATTR_HIDE;
        SlotSetPos(3, 0x42, 0x28, g_menu->list[1].cur * 12 + 0x84);
        break;
    }
}

/* Stages into g_items_pending every entry whose id is in `slot`'s range and
   that the character with key `key` may wear; returns how many. */
int ItemsListUsable(int key, short slot)
{
    u_short *src;
    u_short *dst;
    int      mask;
    int      i;
    int      n;
    int      id;
    int      lo;
    int      hi;
    int      unused[2];

    mask = 1 << key;
    ItemsCompact();
    ItemsClearPending();
    i = 0;
    n = 0;
    src = g_items;
    dst = g_items_pending;
    lo = g_equip_id_range[slot][0];
    hi = g_equip_id_range[slot][1];
    for (; i < ITEM_SLOTS; i++) {
        id = *src & ITEM_ID;
        if (id != 0 && id >= lo && id <= hi && (g_item_defs[id].owners & mask)) {
            *dst++ = *src;
            n++;
        }
        src++;
    }
    return n;
}

/* One row of the item list: the name, and the count right-aligned after
   it. */
void EquipDrawListRow(short row, short *dst)
{
    TileMapFillRect(dst, 0, 12, 1, MAP_W);
    if ((g_items_pending[row] & ITEM_ID) && (g_items_pending[row] >> 9)) {
        DrawItemName(g_items_pending[row] & ITEM_ID, dst, 0, 0);
        TileMapWriteRowRev(g_hud_digits, dst + 11, GLYPH_DIGIT0,
                           FormatDecimal(g_items_pending[row] >> 9,
                                         g_hud_digits, 2));
    }
}

/* The member in party slot `slot` as the screen opens on them: name, what
   they wear, their five stats and six battle numbers, each with the arrow
   the preview later recolours. */
void EquipDrawMember(short slot)
{
    Char *c;
    int   n;

    n = g_party[slot];
    c = &g_chars[n];
    TileMapFillRect(AT(g_tilemap1, 0, 5), 0, 8, 1, MAP_W);
    TileMapWriteRow(c->name, AT(g_tilemap1, 0, 5), 0, 8);

    TileMapFillRect(AT(g_tilemap1, 2, 5), 0, 10, CHAR_EQUIP, MAP_W);
    DrawItemName(g_chars[n].equip[0], AT(g_tilemap1, 2, 5), 0, 1);
    DrawItemName(g_chars[n].equip[1], AT(g_tilemap1, 3, 5), 0, 1);
    DrawItemName(g_chars[n].equip[2], AT(g_tilemap1, 4, 5), 0, 1);
    DrawItemName(g_chars[n].equip[3], AT(g_tilemap1, 5, 5), 0, 1);
    DrawItemName(g_chars[n].equip[4], AT(g_tilemap1, 6, 5), 0, 1);
    DrawItemName(g_chars[n].equip[5], AT(g_tilemap1, 7, 5), 0, 1);
    DrawItemName(g_chars[n].equip[6], AT(g_tilemap1, 8, 5), 0, 1);

    TileMapFillRect(AT(g_tilemap1, 3, 24), 0, 2, CHAR_STATS, MAP_W);
    MEMBER_NUMBER(g_chars[n].stat[0], 2, 3, 25);
    MEMBER_NUMBER(g_chars[n].stat[1], 2, 4, 25);
    MEMBER_NUMBER(g_chars[n].stat[2], 2, 5, 25);
    MEMBER_NUMBER(g_chars[n].stat[3], 2, 6, 25);
    MEMBER_NUMBER(g_chars[n].stat[4], 2, 7, 25);

    TileMapFillRect(AT(g_tilemap1, 2, 34), 0, 3, 6, MAP_W);
    MEMBER_NUMBER(g_chars[n].melee_atk, 3, 2, 36);
    MEMBER_NUMBER(g_chars[n].melee_hit, 3, 3, 36);
    MEMBER_NUMBER(g_chars[n].gun_atk, 3, 4, 36);
    MEMBER_NUMBER(g_chars[n].gun_hit, 3, 5, 36);
    MEMBER_NUMBER(g_chars[n].defence, 3, 6, 36);
    MEMBER_NUMBER(g_chars[n].evade, 3, 7, 36);

    /* `n` again as the row counter: one variable for both, as the
       registers show. */
    for (n = 0; n < CHAR_STATS; n++) {
        AT(g_tilemap1, 3, 22)[n * MAP_W] = ARROW_SAME;
        AT(g_tilemap1, 3, 23)[n * MAP_W] = ARROW_SAME + 1;
    }
    for (n = 0; n < 6; n++) {
        AT(g_tilemap1, 2, 32)[n * MAP_W] = ARROW_SAME;
        AT(g_tilemap1, 2, 33)[n * MAP_W] = ARROW_SAME + 1;
    }
}
