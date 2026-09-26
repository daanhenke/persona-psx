/* Persona 1 (JP) - the item counters' lists and counts.  ADV only.
 *   0x800AA468 ShopMoneyDraw      0x800AA4DC ShopTotalDraw
 *   0x800AA584 ShopTotalRedraw    0x800AA620 SlotSetUV
 *   0x800AA660 ShopLoadItems      0x800AA6E0 ShopLoadPrices
 *   0x800AA760 ShopLoadItems2     0x800AA7E0 ShopLoadPrices2
 *   0x800AA860 ShopBuyListDraw    0x800AA9C4 ShopBuyListDraw2
 *   0x800AAB40 ShopMemberNames    0x800AABD0 ShopHave
 *   0x800AAC34 ShopHavePending    0x800AAC98 ShopAffordable
 *   0x800AACD4 ShopAffordable2    0x800AAD10 ShopCountInit
 *   0x800AAD50 ShopCountInit2     0x800AAD90 ShopCanBuy
 *   0x800AAE80 ShopCanBuy2        0x800AAF70 ShopSellCountInit
 *   0x800AAFB8 ShopSellCount
 *
 * A counter's stock is copied out of the overlay's tables into the work
 * area when it opens: a run of item ids and their prices, found by the
 * shop's (count, first) pair. The counters of kind 5 have their own tables
 * and are paid for from the second purse. How many of an item can be bought
 * is what the money covers, and never more than takes the bag to 99.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/menulist.h>
#include <persona/common/slot.h>
#include <persona/common/char.h>

/* The counters' digit counts come over as bytes in this unit. */
#define TILEMAP_BYTE_COUNT
#include <persona/common/tilemap.h>

#define g_slots  ((Slot *)0x800DC10C)
#define g_money  (*(u_int *)0x801F2674)
#define g_money2 (*(u_int *)0x801F2678)

/* The open counter, and its stock as copied into the work area. */
#define g_facility     ((u_char *)0x800EB580)
#define g_shop_items   ((u_short *)0x800EB590)
#define g_shop_prices  ((int *)0x800EB5D0)
#define g_items        ((u_short *)0x801F267C)
#define g_item_list    ((u_short *)0x800EAE4C)

#define AT(map, row, col) (&(map)[(row) * MAP_W + (col)])

#define BAG_MAX 99

extern Slot    *g_slot_cur;
extern u_char   g_facility_count;
extern MenuList D_800BB858;
extern u_char   D_800BA0C0[];    /* (count, first) by shop */
extern u_char   D_800BA0E4[];    /* and for the counters of kind 5 */
extern u_short  D_800B9DA0[];
extern int      D_800B9864[];
extern u_short  D_800B9F04[];
extern int      D_800B9B2C[];

extern void  DrawItemName(short id, short *dst, u_short base, int b);
extern short ItemsFind(u_short id);
extern short ItemsFindPending(u_short id);

void ShopMoneyDraw(void)
{
    TileMapFillRect(AT(g_tilemap2, 0, 29), 0, 9, 1, MAP_W);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap2, 0, 37), GLYPH_DIGIT0,
                       FormatDecimal(g_money, g_hud_digits, 9));
}

void ShopTotalDraw(u_char row, u_char count)
{
    int *price = &g_shop_prices[row];

    TileMapFillRect(AT(g_tilemap2, 11, 16), 0, 9, 1, MAP_W);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap2, 11, 24), GLYPH_DIGIT0,
                       FormatDecimal(count * *price, g_hud_digits, 9));
    *AT(g_tilemap2, 2, 12) = 0xD0;
}

void ShopTotalRedraw(u_char row, u_char count)
{
    int *price = &g_shop_prices[row];

    TileMapFillRect(AT(g_tilemap2, 11, 16), 0, 9, 1, MAP_W);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap2, 11, 24), GLYPH_DIGIT0,
                       FormatDecimal(count * *price, g_hud_digits, 9));
}

void SlotSetUV(u_char slot, u_char u, u_char v)
{
    g_slot_cur = &g_slots[slot];
    g_slots[slot].u_add = u;
    g_slot_cur->v_add = v;
}

void ShopLoadItems(short shop)
{
    u_short *dst = g_shop_items;
    u_char   i;
    u_char   n;
    u_char   first;

    n = D_800BA0C0[shop * 2];
    first = D_800BA0C0[shop * 2 + 1];
    for (i = 0; i < n; i++) {
        dst[i] = D_800B9DA0[first + i];
    }
}

void ShopLoadPrices(short shop)
{
    int     *dst = g_shop_prices;
    u_char   i;
    u_char   n;
    u_char   first;

    n = D_800BA0C0[shop * 2];
    first = D_800BA0C0[shop * 2 + 1];
    for (i = 0; i < n; i++) {
        dst[i] = D_800B9864[first + i];
    }
}

void ShopLoadItems2(short shop)
{
    u_short *dst = g_shop_items;
    u_char   i;
    u_char   n;
    u_char   first;

    n = D_800BA0E4[shop * 2];
    first = D_800BA0E4[shop * 2 + 1];
    for (i = 0; i < n; i++) {
        dst[i] = D_800B9F04[first + i];
    }
}

void ShopLoadPrices2(short shop)
{
    int     *dst = g_shop_prices;
    u_char   i;
    u_char   n;
    u_char   first;

    n = D_800BA0E4[shop * 2];
    first = D_800BA0E4[shop * 2 + 1];
    for (i = 0; i < n; i++) {
        dst[i] = D_800B9B2C[first + i];
    }
}

u_char ShopCanBuy(u_char row);
u_char ShopCanBuy2(u_char row);

/* 96.6%: the row pointer and the grey offset share s1 in the image, the
   dim flag s0; this build swaps them. */
#ifdef NON_MATCHING
/* The stock with prices; a row that cannot be bought is greyed. */
void ShopBuyListDraw(void)
{
    u_char *fac = g_facility;
    int    *prices = g_shop_prices;
    u_char  i;
    int     dim;
    int     grey;

    i = 0;
    if (g_facility_count) {
        do {
            dim = ShopCanBuy(i) == 0;
            TileMapFillRect(AT(g_tilemap1, i, 0), 0, 0x16, 1, MAP_W);
            grey = dim * 0xD7;
            DrawItemName(g_shop_items[i], AT(g_tilemap1, i, 0), grey, 0);
            TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, i, 21),
                               grey + GLYPH_DIGIT0,
                               FormatDecimal(prices[i], g_hud_digits, 9));
            *AT(g_tilemap1, i, 12) = grey + 0xD0;
            i++;
        } while (i < fac[2]);
    }
}
#else
INCLUDE_ASM("adv/nonmatchings/ui/shopcount", ShopBuyListDraw);
#endif

/* 83.6%: the image spills the counter pointer to the stack and keeps
   the prices base in s8. */
#ifdef NON_MATCHING
void ShopBuyListDraw2(void)
{
    u_char  i;
    int     dim;
    int     grey;
    u_char *fac;

    i = 0;
    fac = g_facility;
    if (g_facility_count) {
        do {
            dim = ShopCanBuy2(i) == 0;
            TileMapFillRect(AT(g_tilemap1, i, 0), 0, 0xA, 1, MAP_W);
            TileMapFillRect(AT(g_tilemap1, i, 14), 0, 9, 1, MAP_W);
            grey = dim * 0xD7;
            DrawItemName(g_shop_items[i], AT(g_tilemap1, i, 0), grey, 0);
            TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, i, 21),
                               grey + GLYPH_DIGIT0,
                               FormatDecimal(g_shop_prices[i], g_hud_digits, 9));
            i++;
        } while (i < fac[2]);
    }
}
#else
INCLUDE_ASM("adv/nonmatchings/ui/shopcount", ShopBuyListDraw2);
#endif

/* 99.7%: one addu has its operands the other way round. */
#ifdef NON_MATCHING
void ShopMemberNames(void)
{
    Char  *c;
    u_char i;

    i = 0;
    c = g_chars;
    for (; i < 5; i++) {
        TileMapWriteRow(c[i].name, AT(g_tilemap2, i + 2, 3), 0, 8);
    }
}
#else
INCLUDE_ASM("adv/nonmatchings/ui/shopcount", ShopMemberNames);
#endif

/* How many of the item the bag holds. */
inline u_char ShopHave(u_short item)
{
    u_char   n;
    u_short *items;
    short    k;

    n = 0;
    items = g_items;
    k = ItemsFind(item);
    if (k != -1) {
        n = items[k] >> 9;
    }
    return n;
}

u_char ShopHavePending(u_short item)
{
    u_char   n;
    u_short *items;
    short    k;

    n = 0;
    items = g_item_list;
    k = ItemsFindPending(item);
    if (k != -1) {
        n = items[k] >> 9;
    }
    return n;
}

/* How many the money covers, up to 99. */
inline u_char ShopAffordable(u_int price)
{
    u_int n;

    n = g_money / price;
    if (n >= 100) {
        n = BAG_MAX;
    }
    return n;
}

inline u_char ShopAffordable2(u_int price)
{
    u_int n;

    n = g_money2 / price;
    if (n >= 100) {
        n = BAG_MAX;
    }
    return n;
}

void ShopCountInit(u_char row)
{
    MenuListInit(&D_800BB858, 1, 0, ShopCanBuy(row), 0x50);
}

void ShopCountInit2(u_char row)
{
    MenuListInit(&D_800BB858, 1, 0, ShopCanBuy2(row), 0x50);
}

u_char ShopCanBuy(u_char row)
{
    u_short item = g_shop_items[row];
    int   *price = &g_shop_prices[row];
    u_char have;
    u_char room;
    u_char n;

    have = ShopHave(item);
    room = BAG_MAX - have;
    n = ShopAffordable(*price);
    if (n >= 100) {
        n = BAG_MAX;
    }
    if (n < room) {
        room = n;
    }
    return room;
}

u_char ShopCanBuy2(u_char row)
{
    u_short item = g_shop_items[row];
    int   *price = &g_shop_prices[row];
    u_char have;
    u_char room;
    u_char n;

    have = ShopHave(item);
    room = BAG_MAX - have;
    n = ShopAffordable2(*price);
    if (n >= 100) {
        n = BAG_MAX;
    }
    if (n < room) {
        room = n;
    }
    return room;
}

short ShopSellCount(short n);

void ShopSellCountInit(short n)
{
    MenuListInit(&D_800BB858, 1, 0, ShopSellCount(n), 0x50);
}

/* How many of a bag entry there are. */
short ShopSellCount(short n)
{
    return g_item_list[n] >> 9;
}
