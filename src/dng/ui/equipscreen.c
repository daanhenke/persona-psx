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
#include <persona/common/bg.h>

#define g_seq_handle ((short *)0x801F537C)

#define STEP_DONE 0xFF
#define PAD_TOGGLE 0x100

/* The item list scrolls a row of 12 lines at a time. */
#define LIST_ROW_H 12

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
extern u_char  g_pdata_mark_up_def[];
extern u_char  g_pdata_mark_down_def[];
extern u_char  D_8009ABFC[];
extern short   g_header_scroll_y;
extern short   D_8009FE20;
extern short   D_800A04D4;
extern short   g_equip_last;

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
extern void   CharEquip();
extern int    PageScrollValue();
extern int    MenuScrollCursor();
extern void   MenuResetRepeat(MenuList *m);
extern void   CharApplyStats();
extern void   CharRecalcStats();
extern void   CharPreviewEquip();
extern void   CharPreviewDraw(short member, Char *c);
extern void   ItemsCommitPending(void);
extern void   CopyShorts(u_short *src, u_short *dst, u_short count);
extern void   TextItemStatRow(short item, short x, short y);
extern u_short EquipPickStronger(short member, short group);
extern void   CharEquipBest(short member, short group);
extern int    ItemsListUsable(short key, short slot);
extern void   EquipDrawListRow(short row, short *dst);
extern void   bcopy(void *src, void *dst, int n);
extern void   ItemsClearPending(void);
extern void   EquipDrawMember(short slot);
extern void   EquipShowCursor(void);
extern void   EquipStepMain(void);
extern void   EquipStepList(void);
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
            EquipStepMain();
            break;
        case 1:
            EquipStepList();
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

#include "../../common/ui/equipsteps.c"

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

/* ItemsListUsable, a unit of its own in ADV and S2D. */
#include "../../common/game/itemslistusable.c"

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

/* EquipDrawMember, a unit of its own in ADV. */
#include "../../common/ui/equipmember.c"
