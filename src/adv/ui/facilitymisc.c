/* Persona 1 (JP) - the facility screens' helpers.  ADV only.
 *
 * The stretch between the fusion screen and the shop counters: what the
 * velvet room, the shops and the other facilities lean on - lookups in the
 * Persona and fusion tables, counts, the money, and the screens' own steps.
 * Most of it is still the original's code.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/persona.h>
#include <persona/common/char.h>
#include <persona/common/item.h>
#include <persona/common/menuctx.h>
#include <persona/common/tilemap.h>
#include <persona/common/slot.h>
#include <persona/common/status.h>
#include <persona/adv/personapage.h>

#define g_persona_slots ((u_char *)0x801F2574)
#define g_item_list     ((u_short *)0x800EAE4C)
#define g_money         (*(u_int *)0x801F2674)
#define g_coins         (*(u_int *)0x801F2678)

#define SLOT_EMPTY    0xFF
#define FUSE_SPECIALS 40
#define COINS_MAX     99999999
#define AT(map, row, col) (&(map)[(row) * MAP_W + (col)])
#define MAP2D(map)        ((short (*)[MAP_W])(map))
#define ITEM_ID       0x1FF
#define ITEM_NO_SALE  0x1000

extern u_char g_moon;
extern u_char g_fuse_moon_picks[];
extern u_char D_800BA0E4[];     /* the fusion tables */
extern int  g_pad_pressed[];
extern void RunFrame(void);
extern short g_persona_data_step;
extern void ItemsCompact(void);
extern void ItemsClearPending(void);
extern void SlotSetPos(u_char slot, int attr, short x, short y);
extern void func_800A4E7C(void);
extern void CoinShopCountStep(void);
extern void func_800A5BCC(void);
extern void func_800A6308(void);
extern void MenuTopRedraw(void);
void FacilityOpen3(void);
extern void MenuWheelOpen2(int a, int b);
extern void D_8007A738(int a, int b);
extern short FacilityTradeCount(short i);
extern void DrawPersonaKeyName(u_char persona, short *dst, int base);
short FuseSpecialHas(short persona);
short CharCanUsePersona(short chr, short persona);
extern void DrawItemName(int id, short *dst, u_short base, int b);
extern const u_char g_persona_list_rule[];
extern short g_item_top;
extern MenuList g_shop_count;   /* the count picker */
extern short    g_fm_mark_pos[][2];
extern u_char   g_fm_mark_def[];
extern short    g_header_scroll_y;
extern short    g_map_scroll_y;
extern short    g_use_top;
extern short    D_800BB9A8;
extern short    D_800BC224;
extern u_char   D_800B1D08[];
extern u_char   D_800B2330[];
extern u_char   g_menu_allow_hold;
extern void     DrawStatusHud(void);
extern u_char   MenuStepMember(int *sel, u_char last);
extern void     EquipScreen(short standalone);
void CharPersonasDraw(short slot, short *dst);
extern int      g_facility_max;   /* the most the facility can hand over */
extern u_char   D_800BA648[];
extern u_char   D_800B1124[];
extern void     TextItemStatRow(short item, short x, short y);
extern void     MenuWheelTurn3(short kind, short list);
extern int      ItemsListShopSell(void);
extern void     ShopBuyOpen(void);
extern void     ShopSellOpen(void);
#define g_shop_items  ((u_short *)0x800EB590)
#define g_shop_prices ((int *)0x800EB5D0)
extern MenuList g_shop_tens;   /* the count's tens, which left and right step */
extern void     ShopTotalDraw(u_char row, u_char count);
extern void     CopyShorts(u_short *src, u_short *dst, u_short count);
extern void     ItemsAddPending(u_short id, u_short count);
extern void     ItemsCommitPending(void);
extern void     MoneySpend(u_int amount);
extern void     CoinsSpend(u_int amount);
extern void     ShopTotalRedraw(u_char row, u_char count);
extern void     MenuSetLayers(int layers);
extern void     TileMapWriteRun10(short *dst);
extern void     BgBoxShow(void);
extern void     DrawPartySlotStatus(int slot, int kind);
extern u_short  g_menu_bg_rle[];

/* What a paper fusion comes out as. */
typedef struct {
    u_short arcana;     /* 0 if the pair cannot be fused */
    u_short unk2;
    u_short unk4;
    u_short persona;
    u_short flag;
} FuseResult;

#define g_fuse       (*(FuseResult *)0x801F1B8C)
#define g_fuse_bytes ((u_char *)0x801F1B8C)  /* the chart reads it a byte at a time */

extern int  PersonaStockCompact();
extern void func_800A1990(u_char a, u_char b, short mode, FuseResult *out, short special);

#define TRADE_ITEMS  32 /* then the seven the moon decides */
#define TRADE_PICKS  3

/* The items the facility trades for, and the two parts each costs. The
   seven after the first thirty-two have three pairs each, one per pick. */
extern short  g_trade_items[];         /* [TRADE_ITEMS + 7], then 0 */
extern short  g_trade_recipes[];       /* [TRADE_ITEMS][2] */
extern short  g_trade_moon_recipes[];  /* [7][TRADE_PICKS][2] */
extern u_char ShopHave(short item);
extern void PersonaSwapOpen(void);
extern void FacilityMemberPick(void);
extern void func_800A2FF8(void);
extern void func_800A3388(void);
extern void func_800A3984(void);
extern void func_800A3D0C(void);
extern void func_800A6788(void);
extern void func_800A7118(void);
extern void func_800A76F4(void);
extern void FacilityTradeMoonPick(void);
extern void func_800A7D7C(void);
extern void ShopTopStep(void);
extern void func_800A8448(void);
extern void ShopBuyCountStep(void);
extern void func_800A8B88(void);
extern void func_800A91C8(void);
extern void ShopMemberPick(void);
extern void func_80077F8C(int a, int b);
extern void func_8007A62C(int a, int b);
extern int  MsgStep(void);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A1990);

#ifdef NON_MATCHING
/* Steps through an arcana's Personas in the fusion tables: dir 1 and 2
   go down one or two, 3 and 4 up, never past either end, and a special
   Persona is stepped over rather than landed on. */
short FuseArcanaStep(short arcana, short cur, short dir)
{
    int n;
    int r;

    switch (dir) {
    case 0:
        return cur;
    case 1:
        n = D_800BA0E4[0x1DA + arcana * 2];
        if (cur == n) {
            return n;
        }
        if (FuseSpecialHas(cur - 1)) {
            return cur;
        }
        return cur - 1;
    case 2:
        n = D_800BA0E4[0x1DA + arcana * 2];
        r = cur;
        if (cur == n) {
            return n;
        }
        if (!FuseSpecialHas(cur - 1)) {
            r--;
        }
        if (cur - 1 == n) {
            return r;
        }
        if (!FuseSpecialHas(cur - 2)) {
            r = cur - 2;
        }
        return r;
    case 3:
        n = D_800BA0E4[0x1DA + arcana * 2] + D_800BA0E4[0x1DB + arcana * 2] - 1;
        if (cur == n) {
            return n;
        }
        if (FuseSpecialHas(cur + 1)) {
            return cur;
        }
        return cur + 1;
    case 4:
        n = D_800BA0E4[0x1DA + arcana * 2] + D_800BA0E4[0x1DB + arcana * 2] - 1;
        r = cur;
        if (cur == n) {
            return n;
        }
        if (!FuseSpecialHas(cur + 1)) {
            r++;
        }
        if (cur + 1 == n) {
            return r;
        }
        if (!FuseSpecialHas(cur + 2)) {
            r = cur + 2;
        }
        return r;
    }
}
#else
INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", FuseArcanaStep);
#endif

/* Whether a Persona's definition lists `spell` among its first six. */
u_char PersonaDefHasSpell(short persona, short spell)
{
    int i;

    for (i = 0; i < 6; i++) {
        if (g_persona_defs[persona].raw[i] == spell) {
            return 1;
        }
    }
    return 0;
}

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A1E7C);

/* Whether a fusion makes one of the special Personas: 3 when the item
   and both Personas fit its recipe, 2 when the item does, 0 when nothing
   does, and 1 when the Persona is not one of the specials at all. */
short FuseSpecialMatch(short persona, short item, short a, short b)
{
    short r;

    r = 0;
    if (FuseSpecialHas(persona)) {
        switch (persona) {
        case 1:
            if (item == 0x88 && (a == 0x34 || a == 0x51) && (b == 0x34 || b == 0x51)) {
                r = 3;
            }
            break;
        case 3:
            if (item == 0x89) {
                r = 2;
            }
            break;
        case 4:
            if (item == 0x8A) {
                r = 2;
            }
            break;
        case 8:
            if (item == 0x87) {
                r = 2;
            }
            break;
        case 9:
            if (item == 0x80 && (a == 0x2C || a == 0x33) && (b == 0x33 || b == 0x2C)) {
                r = 3;
            }
            break;
        case 11:
            if (item == 0x81) {
                r = 2;
            }
            break;
        case 14:
            if (item == 0x82) {
                r = 2;
            }
            break;
        case 19:
            if (item == 0x8C && (a == 0x55 || a == 0x45) && (b == 0x45 || b == 0x55)) {
                r = 3;
            }
            break;
        case 21:
            if (item == 0x8D) {
                r = 2;
            }
            break;
        case 22:
            if (item == 0x8E) {
                r = 2;
            }
            break;
        case 27:
            if (item == 0x7B && (a == 0x10 || a == 0x5D) && (b == 0x5D || b == 0x10)) {
                r = 3;
            }
            break;
        case 28:
            if (item == 0x7C) {
                r = 2;
            }
            break;
        case 32:
            if (item == 0x7D) {
                r = 2;
            }
            break;
        case 35:
            if (item == 0x98 && (a == 0x60 || a == 0x69) && (b == 0x69 || b == 0x60)) {
                r = 3;
            }
            break;
        case 36:
            if (item == 0x99) {
                r = 2;
            }
            break;
        case 42:
            if (item == 0x9A) {
                r = 2;
            }
            break;
        case 46:
            if (item == 0x93 && (a == 0x0A || a == 0x63) && (b == 0x63 || b == 0x0A)) {
                r = 3;
            }
            break;
        case 47:
            if (item == 0x94) {
                r = 2;
            }
            break;
        case 53:
            if (item == 0x83) {
                r = 2;
            }
            break;
        case 57:
            if (item == 0x90 && (a == 0x4F || a == 0x57) && (b == 0x57 || b == 0x4F)) {
                r = 3;
            }
            break;
        case 58:
            if (item == 0x91) {
                r = 2;
            }
            break;
        case 59:
            if (item == 0x92) {
                r = 2;
            }
            break;
        case 64:
            if (item == 0x8F) {
                r = 2;
            }
            break;
        case 66:
            if (item == 0x9F) {
                r = 2;
            }
            break;
        case 68:
            if (item == 0xA1) {
                r = 2;
            }
            break;
        case 69:
            if (item == 0x9B) {
                r = 2;
            }
            break;
        case 71:
            if (item == 0x9D && (a == 0x56 || a == 0x5E) && (b == 0x5E || b == 0x56)) {
                r = 3;
            }
            break;
        case 72:
            if (item == 0x9E) {
                r = 2;
            }
            break;
        case 74:
            if (item == 0xA2) {
                r = 2;
            }
            break;
        case 76:
            if (item == 0xA0) {
                r = 2;
            }
            break;
        case 78:
            if (item == 0x8B) {
                r = 2;
            }
            break;
        case 80:
            if (item == 0x96 && (a == 0x5B || a == 0x6A) && (b == 0x6A || b == 0x5B)) {
                r = 3;
            }
            break;
        case 81:
            if (item == 0x97) {
                r = 2;
            }
            break;
        case 85:
            if (item == 0x84 && (a == 0x0E || a == 0x1B) && (b == 0x1B || b == 0x0E)) {
                r = 3;
            }
            break;
        case 86:
            if (item == 0x85) {
                r = 2;
            }
            break;
        case 87:
            if (item == 0x86) {
                r = 2;
            }
            break;
        case 88:
            if (item == 0x9C) {
                r = 2;
            }
            break;
        case 92:
            if (item == 0x95) {
                r = 2;
            }
            break;
        case 93:
            if (item == 0x7E && (a == 0x3E || a == 0x30) && (b == 0x30 || b == 0x3E)) {
                r = 3;
            }
            break;
        case 95:
            if (item == 0x7F) {
                r = 2;
            }
            break;
        }
        return r;
    }
    return 1;
}

/* Whether a Persona is one of the forty the fusion tables treat specially. */
short FuseSpecialHas(short persona)
{
    int i;

    for (i = 0; i < FUSE_SPECIALS; i++) {
        if (D_800BA0E4[0x208 + i] == persona) {
            return 1;
        }
    }
    return 0;
}

/* The fusion tables' pick for the moon's phase and the second Persona's
   arcana; the fusion keeps it on the result when it is not 0. The first
   Persona plays no part. */
short FuseMoonLookup(short a, short b)
{
    int row;

    row = D_800BA0E4[0x230 + (g_moon & 0xF)];
    return g_fuse_moon_picks[D_800BA0E4[0x240 + row] * 7 +
                             D_800BA0E4[0x243 + g_persona_data[b].arcana * 9 + row]];
}

/* The fusion chart: what each pair of stock Personas fuses into, one cell
   a pair, or the blank cell where the pair gives nothing. */
void FuseChartDraw(void)
{
    int i;
    int j;
    int k;
    int base;
    int cell;

    for (i = 0; i <= PersonaStockCompact(); i++) {
        for (j = 0; j <= PersonaStockCompact(); j++) {
            func_800A1990(g_persona_stock[i], g_persona_stock[j], 0, &g_fuse, 0);
            cell = 0x44B;
            if (g_fuse_bytes[0] != 0) {
                switch (g_fuse_bytes[2]) {
                case 0:
                    k = 0;
                    break;
                case 1:
                    k = 2;
                    break;
                case 2:
                    k = 1;
                    break;
                }
                switch (g_fuse_bytes[4]) {
                case 0:
                    base = 0x448;
                    break;
                case 1:
                    base = 0x451;
                    break;
                case 2:
                    base = 0x44E;
                    break;
                case 3:
                    base = 0x454;
                    break;
                }
                cell = base + k;
            }
            MAP2D(g_tilemap1)[i + 1][j + 22] = cell;
        }
    }
}

/* Fills g_item_list with the bag's items a shop will buy, and says how
   many there are. */
int ItemsListSellable(void)
{
    u_short *src;
    u_short *dst;
    u_short  v;
    int      i;
    int      n;

    ItemsCompact();
    ItemsClearPending();
    i = 0;
    n = 0;
    src = g_items;
    dst = g_item_list;
    for (; i < ITEM_SLOTS; i++) {
        v = *src++;
        if ((v & ITEM_ID) && !(g_item_defs[v & ITEM_ID].unk06 & ITEM_NO_SALE)) {
            *dst++ = v;
            n++;
        }
    }
    return n;
}

void FacilityStep6(void)
{
    switch (g_persona_data_step) {
    case 0:
        PersonaSwapOpen();
        g_persona_data_step++;
        break;
    case 1:
        FacilityMemberPick();
        break;
    case 2:
        func_800A2FF8();
        break;
    case 3:
        func_800A3388();
        break;
    case 4:
        func_800A3984();
        break;
    case 5:
        func_800A3D0C();
        break;
    }
}

/* Lays out the Persona swap screen on the member under the cursor. */
void PersonaSwapOpen(void)
{
    int i;

    SlotClearAll();
    MenuSetLayers(0x1C);
    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap1, 0, MAP_W, 0x40, MAP_W);
    TileMapFillRect(g_tilemap2, 0, MAP_W, 0x40, MAP_W);
    TileMapDrawWindow(AT(g_tilemap0, 0, 6), 0x10, 9, MAP_W);
    TileMapDrawBox(AT(g_tilemap0, 1, 7), 0xE, 7, MAP_W);
    TileMapBlitRle(g_menu_bg_rle, AT(g_tilemap0, 11, 0), MAP_W);
    TileMapWriteRun10(AT(g_tilemap0, 2, 9));
    for (i = 0; i < 3; i++) {
        TileMapWriteBar(g_tilemap0 + 9 + (i + 4) * MAP_W, 10);
        *AT(g_tilemap2, i + 2, 0) = i + 0x418;
    }
    TileMapWriteRow(str_cell_run, AT(g_tilemap2, 1, 3), 0x457, 6);
    BgBoxShow();
    DrawStatusHud();
    DrawPartySlotStatus(0, 0);
    DrawPartySlotStatus(1, 0);
    DrawPartySlotStatus(2, 0);
    DrawPartySlotStatus(3, 0);
    DrawPartySlotStatus(4, 0);
    CharPersonasDraw(g_menu->status_who.cur, g_tilemap2);
    g_cam_y = 0;
    g_map_scroll_y = 0;
    g_header_scroll_y = 0;
    SlotClear(0x2F);
    SlotInitTagged(D_800B1D08, 0x3C, 0x300, 0x18, 0x18);
    SlotInitTagged(D_800B2330, 0x2D, 0x2FF, 0, 0x10);
    SlotSetAnim(0x2D, 0, 0, 0, 0x30, 0x24, 0, 0);
    SlotInitTagged(g_fm_mark_def, 1, 0x42, (g_fm_mark_pos + 1)[g_menu->status_who.cur][0],
                   (g_fm_mark_pos + 1)[g_menu->status_who.cur][1]);
    SlotSetFlicker(1, 1);
}

/* Picks a party member and shows the Personas it carries; A moves on to
   them, B packs the Persona slots and leaves. */
void FacilityMemberPick(void)
{
    u_char *slots;
    u_char  i;
    u_char  j;

    MenuStepMember(&g_menu->status_who.cur, g_party_last);
    SlotSetPos(1, 0x42, (g_fm_mark_pos + 1)[g_menu->status_who.cur][0],
               (g_fm_mark_pos + 1)[g_menu->status_who.cur][1]);
    CharPersonasDraw(g_menu->status_who.cur, g_tilemap2);
    DrawStatusHud();
    if (InputCheckAcceptA(1)) {
        MenuListInit(&g_menu->unk2D0, 0, 0, 3, 0x16);
        func_800A47F8();
        CharPersonasDraw(g_menu->status_who.cur, AT(g_tilemap2, 1, 0));
        FacilityCursorPlace();
        g_map_scroll_y = g_use_top * 12;
        g_persona_data_step++;
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        slots = g_persona_slots;
        for (i = 0; i < 16; i++) {
            if (slots[i] == SLOT_EMPTY) {
                for (j = i + 1; j < 16; j++) {
                    if (slots[j] != SLOT_EMPTY) {
                        slots[i] = slots[j];
                        slots[j] = SLOT_EMPTY;
                        break;
                    }
                }
            }
        }
        g_persona_data_step = 0xFF;
    }
}

/* A party member's name and the three Personas it carries, the one in use
   picked out; empty entries, or all three while the list is blocked, as
   rules. */
void CharPersonasDraw(short slot, short *dst)
{
    Persona *personas;
    Char    *c;
    int      i;

    c = &g_chars[g_party[slot]];
    personas = g_personas;
    TileMapFillRect(dst + 2, 0, 8, 1, MAP_W);
    TileMapFillRect(dst + 2 * MAP_W + 1, 0, 10, 3, MAP_W);
    TileMapWriteRow(c->name, dst + 2, 0, 8);
    for (i = 0; i < 3; i++) {
        if (c->list[i] != SLOT_EMPTY && !c->blocked) {
            DrawPersonaKeyName(personas[c->list[i]].key, dst + (i + 2) * MAP_W + 1,
                               c->entry == i ? 0x285 : 0);
        } else {
            TileMapWriteRow(g_persona_list_rule, dst + (i + 2) * MAP_W + 2, 0xD7, 8);
        }
    }
}

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A2FF8);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A3388);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A3984);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A3D0C);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A43BC);

/* How many of the sixteen Persona slots are filled. */
u_char PersonaSlotsCount(void)
{
    u_char *slots;
    u_char  i;
    u_char  n;

    slots = g_persona_slots;
    i = 0;
    n = 0;
    for (; i < 16; i++) {
        if (slots[i] != SLOT_EMPTY) {
            n++;
        }
    }
    return n;
}

/* Whether a Persona has reached its eighth spell slot. */
u_char PersonaSpellsFull(short n)
{
    return g_personas[n].slots >= 8;
}

/* Puts the facility cursor on its row: three rows 12 apart, the fourth
   further down. */
void FacilityCursorPlace(void)
{
    int n;

    n = g_menu->unk2D0.cur;
    switch (n) {
    case 0:
    case 1:
    case 2:
        SlotSetPos(1, 0x42, 0x48, n * 12 + 0x3C);
        break;
    case 3:
        SlotSetPos(1, 0x42, 0x48, 0x6C);
        break;
    }
}

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A47F8);

/* The Persona slots page: each filled slot's name, and a rule for each
   empty one. */
void PersonaSlotsDraw(void)
{
    Persona *personas;
    int      i;

    personas = g_personas;
    TileMapFillRect(AT(g_tilemap1, 0, 1), 0, 10, 16, MAP_W);
    for (i = 0; i < 16; i++) {
        if (g_persona_slots[i] != SLOT_EMPTY) {
            DrawPersonaKeyName(personas[g_persona_slots[i]].key,
                            AT(g_tilemap1, i, 1), 0);
        } else {
            TileMapWriteRow(g_persona_list_rule, AT(g_tilemap1, i, 2), 0xD7, 8);
        }
    }
}

/* The Persona slots page for one party member: each name dimmed unless the
   member can take it. */
void PersonaSlotsDrawFor(short slot)
{
    Persona *personas;
    short    chr;
    int      i;
    int      n;

    personas = g_personas;
    chr = g_party[slot];
    TileMapFillRect(AT(g_tilemap1, 0, 1), 0, 10, 16, MAP_W);
    for (i = 0; i < 16; i++) {
        n = g_persona_slots[i];
        if (n != SLOT_EMPTY) {
            DrawPersonaKeyName(personas[n].key, AT(g_tilemap1, i, 1),
                               CharCanUsePersona(chr, personas[n].key) ? 0 : 0xD7);
        } else {
            TileMapWriteRow(g_persona_list_rule, AT(g_tilemap1, i, 2), 0xD7, 8);
        }
    }
}

#ifdef NON_MATCHING
/* Whether a character may take a Persona: it must answer them at all, and
   they must be at its level. */
/* 96.14%: the image keeps v0 free and puts the Persona offset in a1;
   this build uses v0 and a0 for the same values. */
short CharCanUsePersona(short chr, short persona)
{
    Char *c;

    c = &g_chars[chr];
    if ((g_persona_defs[persona].bond >> ((c->key - 1) * 2)) & 3) {
        return g_persona_defs[persona].level <= c->unk56;
    }
    return 0;
}
#else
INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", CharCanUsePersona);
#endif

void ShopStep2(void)
{
    switch (g_persona_data_step) {
    case 0:
        func_800A4E7C();
        break;
    case 1:
        CoinShopCountStep();
        break;
    }
}

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A4E7C);

/* The coin counter's count: as at the money counters, but paid in coins. */
void CoinShopCountStep(void)
{
    u_short *item;
    int     *price;

    if (MenuStepCursor(&g_shop_tens)) {
        g_shop_count.cur = g_shop_tens.cur * 10 + g_shop_count.cur % 10;
    } else {
        MenuStepCursor(&g_shop_count);
    }
    if (g_shop_count.cur > g_shop_count.hi) {
        g_shop_count.cur = g_shop_count.hi;
    } else if (g_shop_tens.cur < 0) {
        g_shop_count.cur = 0;
    }
    if (g_shop_count.cur == 0) {
        g_shop_count.cur = 1;
    }
    g_shop_tens.cur = g_shop_count.cur / 10;
    ShopCountRowDraw();
    ShopTotalRedraw(g_item_top + g_menu->unk100.cur, g_shop_count.cur);
    MsgStep();
    if (InputCheckAcceptA(1)) {
        item = g_shop_items + g_item_top + g_menu->unk100.cur;
        price = g_shop_prices + g_item_top + g_menu->unk100.cur;
        CopyShorts(g_items, g_item_list, 0x17F);
        ItemsAddPending(*item, g_shop_count.cur);
        ItemsCommitPending();
        ItemsCompact();
        CoinsSpend(*price * g_shop_count.cur);
        func_800A5560();
        SlotSetFlicker(0, 1);
        g_persona_data_step--;
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        func_800A5560();
        SlotSetFlicker(0, 1);
        g_persona_data_step--;
    }
}

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A5560);

/* Keeps the message running until any button is pressed. */
void MsgStepUntilPress(void)
{
    goto check;
loop:
    MsgStep();
check:
    RunFrame();
    if (g_pad_pressed[0] == 0) goto loop;
    RunFrame();
}

/* How many coins the money buys, at a hundred a coin, up to what the coin
   count can still hold. */
u_int CoinsAffordable(void)
{
    u_int room;
    u_int n;

    n = g_money / 100;
    room = COINS_MAX - g_coins;
    if (room < n) {
        return room;
    }
    return n;
}

void CoinExchangeStep(void)
{
    switch (g_persona_data_step) {
    case 0:
        func_800A5BCC();
        break;
    case 1:
        func_800A6308();
        break;
    }
}

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A5BCC);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A6308);

void FacilityStep3(void)
{
    switch (g_persona_data_step) {
    case 0:
        FacilityOpen3();
        g_persona_data_step++;
        break;
    case 1:
        func_800A6788();
        break;
    case 2:
        func_800A7118();
        break;
    case 3:
        func_800A76F4();
        break;
    case 4:
        FacilityTradeMoonPick();
        break;
    case 5:
        func_800A7D7C();
        break;
    }
}

void FacilityOpen3(void)
{
    MenuTopRedraw();
    SlotSetAnim(0x2D, 0, 0, 0, 0, 0x30, 0, 0);
    MenuWheelOpen2(0, 4);
    D_8007A738(0, 8);
}

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A6788);

/* Thirty-two item names in two columns, dimmed where the item is out. */
void FacilityItemsDraw(void)
{
    short *ids;
    int    i;
    int    dim;

    TileMapFillRect(g_tilemap1, 0, 0x16, 0x20, MAP_W);
    i = 0;
    ids = g_trade_items;
    for (; i < 32; i++) {
        dim = FacilityTradeCount(i) == 0;
        DrawItemName(*ids, &g_tilemap1[i / 2 * MAP_W] + (i & 1) * 12, dim * 0xD7, 0);
        ids++;
    }
}

/* The seven names after the thirty-two, one to a row. */
void FacilityTradeExtrasDraw(void)
{
    int i;
    int dim;

    TileMapFillRect(g_tilemap1, 0, 10, 5, MAP_W);
    for (i = 0; i < 7; i++) {
        dim = FacilityTradeCount(i + TRADE_ITEMS) == 0;
        DrawItemName(g_trade_items[i + TRADE_ITEMS], AT(g_tilemap1, i, 0), dim * 0xD7, 0);
    }
}

/* How many of item n can be had: one of each of its two parts apiece, and
   no more than the bag's 99 still takes. The seven items after the first
   thirty-two (ids 0x13-0x19) have three recipes each, picked by the moon. */
short FacilityTradeCount(short n)
{
    int    room;
    int    have;
    u_char have2;
    int    k;

    room = 99 - ShopHave(g_trade_items[n]);
    if (g_trade_items[n] < 0x13 || g_trade_items[n] > 0x19) {
        have = ShopHave(g_trade_recipes[n * 2]);
        have2 = ShopHave(g_trade_recipes[n * 2 + 1]);
    } else {
        k = n * TRADE_PICKS + D_800BA0E4[0x10 + (g_moon & 0x1F)];
        have = ShopHave(g_trade_moon_recipes[(k - TRADE_ITEMS * TRADE_PICKS) * 2]);
        have2 = ShopHave(g_trade_moon_recipes[(k - TRADE_ITEMS * TRADE_PICKS) * 2 + 1]);
    }
    if (have2 < have) {
        have = have2;
    }
    if (room < have) {
        return room;
    }
    return have;
}

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A7118);

/* The two parts item n costs, each with how many the bag holds. */
void FacilityTradePartsDraw(short n)
{
    short a;
    short b;

    TileMapFillRect(AT(g_tilemap2, 4, 1), 0, 10, 1, MAP_W);
    a = g_trade_recipes[n * 2];
    DrawItemName(a, AT(g_tilemap2, 4, 1), 0, 0);
    TileMapFillRect(AT(g_tilemap2, 5, 12), 0, 2, 1, MAP_W);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap2, 5, 13), GLYPH_DIGIT0,
                       FormatDecimal(ShopHave(a), g_hud_digits, 2));
    *AT(g_tilemap2, 5, 11) = 0xCE;
    TileMapFillRect(AT(g_tilemap2, 6, 1), 0, 10, 1, MAP_W);
    b = g_trade_recipes[n * 2 + 1];
    DrawItemName(b, AT(g_tilemap2, 6, 1), 0, 0);
    TileMapFillRect(AT(g_tilemap2, 7, 12), 0, 2, 1, MAP_W);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap2, 7, 13), GLYPH_DIGIT0,
                       FormatDecimal(ShopHave(b), g_hud_digits, 2));
    *AT(g_tilemap2, 7, 11) = 0xCE;
}

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A76F4);

/* The moon items' list: moving shows each one's stats and parts; A asks
   for a count when any can be had. */
void FacilityTradeMoonPick(void)
{
    short prev;
    short n;

    prev = g_menu->unk1A0.cur;
    MenuStepCursor(&g_menu->unk1A0);
    SlotSetPos(4, 0x42, 0x60, g_menu->unk1A0.cur * 12 + 0x30);
    if (prev != g_menu->unk1A0.cur) {
        n = g_menu->unk1A0.cur;
        prev = g_menu->unk1A0.cur;
        TextItemStatRow(g_trade_items[n + TRADE_ITEMS], 0x42, 0x10);
        FacilityTradeMoonPartsDraw(n);
    }
    MsgStep();
    if (InputCheckAcceptA(1)) {
        g_facility_max = FacilityTradeCount(prev + TRADE_ITEMS);
        if (g_facility_max != 0) {
            *AT(g_tilemap2, 1, 11) = 0xCE;
            TileMapWriteRow(D_800BA648, AT(g_tilemap2, 1, 2), 0, 8);
            FormatDecimal(g_facility_max, g_hud_digits, 2);
            MenuListInit(&g_menu->arcana_row, 0, -1, 10, 0x90);
            MenuListInit(&g_menu->unk100, 0, 0, g_facility_max, 0x50);
            SlotInitTagged(D_800B1124, 1, 0x42, 0xC0, 0x90);
            SlotSetFlicker(1, 1);
            SlotSetFlicker(4, 0);
            g_persona_data_step++;
        }
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        g_persona_data_step = 0;
    }
}

/* The same for one of the seven moon items, under the moon's pick. */
void FacilityTradeMoonPartsDraw(short n)
{
    int   pick;
    short a;
    short b;

    pick = D_800BA0E4[0x10 + (g_moon & 0x1F)];
    TileMapFillRect(AT(g_tilemap2, 3, 1), 0, 10, 1, MAP_W);
    a = g_trade_moon_recipes[(n * TRADE_PICKS + pick) * 2];
    DrawItemName(a, AT(g_tilemap2, 3, 1), 0, 0);
    TileMapFillRect(AT(g_tilemap2, 4, 12), 0, 2, 1, MAP_W);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap2, 4, 13), GLYPH_DIGIT0,
                       FormatDecimal(ShopHave(a), g_hud_digits, 2));
    *AT(g_tilemap2, 4, 11) = 0xCE;
    TileMapFillRect(AT(g_tilemap2, 5, 1), 0, 10, 1, MAP_W);
    b = g_trade_moon_recipes[(n * TRADE_PICKS + pick) * 2 + 1];
    DrawItemName(b, AT(g_tilemap2, 5, 1), 0, 0);
    TileMapFillRect(AT(g_tilemap2, 6, 12), 0, 2, 1, MAP_W);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap2, 6, 13), GLYPH_DIGIT0,
                       FormatDecimal(ShopHave(b), g_hud_digits, 2));
    *AT(g_tilemap2, 6, 11) = 0xCE;
}

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A7D7C);

void ShopStep(void)
{
    switch (g_persona_data_step) {
    case 0:
        ShopScreenOpen();
        g_persona_data_step++;
        break;
    case 1:
        ShopTopStep();
        break;
    case 2:
        func_800A8448();
        break;
    case 3:
        ShopBuyCountStep();
        break;
    case 4:
        func_800A8B88();
        break;
    case 5:
        func_800A91C8();
        break;
    case 6:
        ShopMemberPick();
        break;
    }
}

void ShopScreenOpen(void)
{
    MenuTopRedraw();
    func_80077F8C(3, 1);
    func_8007A62C(3, 8);
}

/* The counter's top: buy, sell, or equip a member. */
void ShopTopStep(void)
{
    DrawStatusHud();
    if (MenuStepCursor(&g_menu->skill_persona)) {
        MenuWheelTurn3(3, 8);
    }
    if (InputCheckAcceptA(2)) {
        SlotSetFlicker(0, 0);
        switch (g_menu->skill_persona.cur) {
        case 0:
            ShopBuyOpen();
            TextItemStatRow(g_shop_items[g_item_top + g_menu->unk100.cur], 0x38, 0xE);
            g_persona_data_step = 2;
            break;
        case 1:
            if (ItemsListShopSell()) {
                MenuListInit(&g_menu->page, 0, 0, 7, 0x14);
                MenuListInit(&g_menu->stock, 0, 0, 1, 0x1A);
                g_use_top = 0;
                ShopSellOpen();
                g_persona_data_step = 4;
            }
            break;
        case 2:
            SlotClearAll();
            SlotInitTagged(D_800B1D08, 0x3C, 0x300, 0x18, 0x18);
            SlotInitTagged(D_800B2330, 0x2D, 0x2FF, 0, 0x10);
            SlotSetAnim(0x2D, 0, 0, 0, 0x90, 0, 0, 0);
            SlotInitTagged(g_fm_mark_def, 1, 0x42, (g_fm_mark_pos + 1)[g_menu->unk050.cur][0],
                           (g_fm_mark_pos + 1)[g_menu->unk050.cur][1]);
            SlotSetFlicker(1, 1);
            g_persona_data_step = 6;
            break;
        }
    } else if (InputCheckAcceptB(2) || g_menu_allow_hold) {
        g_persona_data_step = 0xFF;
    }
}

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A8448);

/* How many to buy: up and down step by one, left and right by ten, and
   never more than the money and the bag allow nor fewer than one. A buys
   them. */
void ShopBuyCountStep(void)
{
    u_short *item;
    int     *price;

    if (MenuStepCursor(&g_shop_tens)) {
        g_shop_count.cur = g_shop_tens.cur * 10 + g_shop_count.cur % 10;
    } else {
        MenuStepCursor(&g_shop_count);
    }
    if (g_shop_count.cur > g_shop_count.hi) {
        g_shop_count.cur = g_shop_count.hi;
    } else if (g_shop_tens.cur < 0) {
        g_shop_count.cur = 0;
    }
    if (g_shop_count.cur == 0) {
        g_shop_count.cur = 1;
    }
    g_shop_tens.cur = g_shop_count.cur / 10;
    ShopCountRowDraw();
    ShopTotalDraw(g_item_top + g_menu->unk100.cur, g_shop_count.cur);
    MsgStep();
    if (InputCheckAcceptA(1)) {
        item = g_shop_items + g_item_top + g_menu->unk100.cur;
        price = g_shop_prices + g_item_top + g_menu->unk100.cur;
        CopyShorts(g_items, g_item_list, 0x17F);
        ItemsAddPending(*item, g_shop_count.cur);
        ItemsCommitPending();
        ItemsCompact();
        MoneySpend(*price * g_shop_count.cur);
        ShopBuyOpen();
        SlotClear(8);
        g_persona_data_step = 2;
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        ShopBuyOpen();
        SlotClear(8);
        g_persona_data_step = 2;
    }
}

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A8B88);

INCLUDE_ASM("adv/nonmatchings/ui/facilitymisc", func_800A91C8);

/* Picks the member to equip from the counter; A opens the equipment
   screen on them, B goes back to the counter's top. */
void ShopMemberPick(void)
{
    short *x;
    short *y;

    DrawStatusHud();
    if (MenuStepMember(&g_menu->unk050.cur, g_party_last)) {
        g_header_scroll_y = 0;
        D_800BB9A8 = 0;
        D_800BC224 = 0;
        g_menu->unk230.cur = 0;
        g_menu->unk240.cur = 0;
    }
    x = &g_fm_mark_pos[1][0];
    SlotSetPos(1, 0x42, (g_fm_mark_pos + 1)[g_menu->unk050.cur][0],
               (y = x + 1)[g_menu->unk050.cur * 2]);
    if (InputCheckAcceptA(1)) {
        EquipScreen(0);
        MenuTopRedraw();
        func_80077F8C(3, 1);
        func_8007A62C(3, 8);
        SlotClearAll();
        SlotInitTagged(D_800B1D08, 0x3C, 0x300, 0x18, 0x18);
        SlotInitTagged(D_800B2330, 0x2D, 0x2FF, 0, 0x10);
        SlotSetAnim(0x2D, 0, 0, 0, 0x90, 0, 0, 0);
        SlotInitTagged(g_fm_mark_def, 1, 0x42, x[g_menu->unk050.cur * 2],
                       y[g_menu->unk050.cur * 2]);
        SlotSetFlicker(1, 1);
        DrawStatusHud();
        g_persona_data_step = 6;
    } else if (InputCheckAcceptB(1) || g_menu_allow_hold) {
        g_persona_data_step = 0;
    }
}

/* The count on the list row under the cursor, with its brackets. */
void ShopCountRowDraw(void)
{
    short row;

    row = g_item_top + g_menu->unk100.cur;
    TileMapFillRect(AT(g_tilemap1, 0, 23), 0, 3, 16, MAP_W);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, row, 25), GLYPH_DIGIT0,
                       FormatDecimal(g_shop_count.cur, g_hud_digits, 2));
    *AT(g_tilemap1, row, 23) = 0xCE;
    *AT(g_tilemap2, 13, 8) = 0xCE;
}

/* The sell count beside the row, and what that many fetch. */
void ShopSellTotalDraw(short row)
{
    TileMapFillRect(AT(g_tilemap2, 11, 9), 0, 2, 1, MAP_W);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap2, 11, 10), GLYPH_DIGIT0,
                       FormatDecimal(g_shop_count.cur, g_hud_digits, 2));
    TileMapFillRect(AT(g_tilemap2, 11, 16), 0, 9, 1, MAP_W);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap2, 11, 24), GLYPH_DIGIT0,
                       FormatDecimal(g_item_defs[g_item_list[row] & ITEM_ID].price * g_shop_count.cur,
                                     g_hud_digits, 9));
}
