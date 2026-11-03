/* Observe the actual coin-shop C on Linux with synthetic menu/game data.
   The source's fixed PS1 data addresses are backed by a private test mapping. */
#define SKIP_ASM
#define WORK_BIAS 0
#include "../../src/adv/ui/facilitymisc.c"
extern int printf(const char*, ...);
extern void abort(void);
extern int fflush(void*);
extern void* memset(void*, int, unsigned long);
extern void* mmap(void*, unsigned long, int, int, int, long);
extern int munmap(void*, unsigned long);
static MenuCtx menu;
MenuCtx* g_menu = &menu;
MenuList g_shop_tens;
Slot* g_slot_cur;
short g_item_top, g_map_scroll_y;
u_char g_facility_count, g_hud_digits[16];
const u_char str_cell_run[16] = {0};
u_char D_800BA640[8], D_800B1EB8[8], D_800B1D08[8], D_800B2330[8];
u_char g_pdata_cursor_def[8], g_pdata_mark_up_def[8], g_pdata_mark_down_def[8];
static int checks, cases, fills, bars, tags, formats, revs, counts, selected;
static int mutate, initialCursor;
static u_int budgets[] = {0, 1, 99999999};
#define CHECK(c) do { checks++; if (!(c)) { printf("coin shop: case %d line %d failed\n", cases, __LINE__); fflush(0); abort(); } } while (0)
void MenuSetLayers(int layers)
{
    CHECK(layers == 0x14);
    /* Selection is captured before this callback; later UI reads stay live. */
    if (mutate) { g_item_top = (g_item_top + 1) % 9; g_menu->unk100.cur = (initialCursor + 1) % 8; }
}
void TileMapFillRect(short* dst, short value, u_short w, u_short h, u_short stride)
{
    int i;
    CHECK(stride == 40);
    if (fills < 3) {
        CHECK(dst == (fills == 0 ? g_tilemap0 : fills == 1 ? g_tilemap1 : g_tilemap2));
        CHECK(value == 0 && w == 40 && h == 64);
        for (i = 0; i < w * h; i++) dst[i] = 0;
    } else {
        static const int pos[] = {93,105,442,456,536};
        static const int widths[] = {2,1,28,9,9};
        static const int heights[] = {8,8,4,1,1};
        int j = fills - 3;
        CHECK(j < 5);
        CHECK(dst == (j < 3 ? g_tilemap0 : g_tilemap2) + pos[j]);
        CHECK(w == widths[j] && h == heights[j] && value == (j < 3 ? 0x17 : 0));
        /* Observe the separator and untouched neighbouring cells. */
        if (j == 0) for (i = 0; i < 64; i++) {
            CHECK(g_tilemap1[i * 40 + 12] == (i < 16 ? 0x37F : 0));
            CHECK(g_tilemap1[i * 40 + 11] == 0 && g_tilemap1[i * 40 + 13] == 0);
        }
    }
    fills++;
}
void TileMapDrawWindow(short* dst, u_char w, u_char h, u_char stride)
{ CHECK(fills == 3 && dst == g_tilemap0 && w == 32 && h == 17 && stride == 40); }
void TileMapDrawBox(short* dst, u_short w, short h, u_short stride)
{ CHECK(dst == g_tilemap0 + 41 && w == 30 && h == 15 && stride == 40); }
void TileMapWriteBar(short* dst, u_char width)
{
    if (bars < 24) {
        static const int columns[] = {3,15,26};
        CHECK(dst == g_tilemap0 + (bars / 3 + 2) * 40 + columns[bars % 3]);
        CHECK(width == (bars % 3 == 2 ? 3 : 10));
    } else {
        static const int pos[] = {498,578,571};
        CHECK(bars < 27 && dst == g_tilemap0 + pos[bars - 24]);
        CHECK(width == (bars == 26 ? 3 : 10));
    }
    bars++;
}
void TileMapWriteRow(const u_char* src, short* dst, int base, u_short count)
{
    if (src == str_cell_run) {
        CHECK((dst == g_tilemap2 + 14 && base == 0x46E && count == 4) ||
              (dst == g_tilemap2 + 413 && base == 0x38D && count == 5) ||
              (dst == g_tilemap2 + 493 && base == 0x389 && count == 4));
    } else CHECK(src == D_800BA640 && dst == g_tilemap2 + 520 && base == 0 && count == 7);
}
void TextItemStatRow(short item, short x, short y)
{ CHECK(item == (g_shop_items[selected] & 0x1FF) && x == 56 && y == 14 && bars == 27); }
void ShopCountInit2(u_char row) { CHECK(row == selected); counts++; }
u_char ShopHave(short item) { CHECK((u_short)item == g_shop_items[selected]); return 37; }
short FormatDecimal(u_int value, u_char* dst, u_short width)
{
    CHECK(dst == g_hud_digits && formats < 3);
    CHECK(value == (formats == 0 ? (u_int)g_shop_prices[selected] : formats == 1 ? 37 : g_coins));
    CHECK(width == (formats == 1 ? 2 : 9));
    formats++; return formats + 1;
}
void TileMapWriteRowRev(const u_char* src, short* dst, int base, u_short count)
{
    static const int pos[] = {464,530,544};
    CHECK(revs < 3 && src == g_hud_digits && dst == g_tilemap2 + pos[revs]);
    CHECK(base == 0xC0 && count == revs + 2 && formats == revs + 1);
    revs++;
}
void SlotClearAll(void) { CHECK(revs == 3); }
void SlotInitTagged(void* def, u_char slot, int attr, short x, short y)
{
    static const int slots[] = {46,60,45,4,32,33};
    static const int attrs[] = {36,0x300,0x2FF,0x42,0x42,0x42};
    static const int xs[] = {54,24,0,72,152,152};
    void* defs[] = {D_800B1EB8,D_800B1D08,D_800B2330,g_pdata_cursor_def,g_pdata_mark_up_def,g_pdata_mark_down_def};
    int expectedY = tags == 0 ? 12 : tags == 1 ? 24 : tags == 2 ? 16 : tags == 3 ? g_menu->unk100.cur * 12 + 48 : tags == 4 ? 48 : 132;
    CHECK(tags < 6 && slot == slots[tags] && attr == attrs[tags] && x == xs[tags] && y == expectedY && def == defs[tags]);
    /* Preserve unrelated attributes when changing the arrow visibility. */
    g_slots[slot].attr = 0x12348001;
    tags++;
}
void SlotSetAnim(short slot, short a, short b, u_char page, u_char u, u_char v, u_short cx, u_short cy)
{ CHECK(slot == 45 && a == 0 && b == 0 && page == 0 && u == 0 && v == 72 && cx == 0 && cy == 0); }
void SlotSetFlicker(u_char slot, u_char on) { CHECK(slot == 4 && on == 1 && tags == 4); }
void MenuListInit(MenuList* m, int cur, int lo, int hi, u_short flags)
{ CHECK(m == &g_shop_tens && cur == 0 && lo == -1 && hi == 10 && flags == 0x90); }
void ShopBuyListDraw2(void) { CHECK(counts == 2 && tags == 6); }
int main(void)
{
    int top, cursor, budget, i;
    /* MAP_FIXED_NOREPLACE rejects an occupied range instead of replacing it. */
    void* ram = mmap((void*)0x80000000UL, 0x200000, 3, 0x100022, -1, 0);
    if (ram != (void*)0x80000000UL) {
        printf("coin shop: fixed test address unavailable (for example, ASan shadow memory); skipped\n");
        return 77;
    }
    for (mutate = 0; mutate < 2; mutate++) for (top = 0; top <= 8; top++)
    for (cursor = 0; cursor < 8; cursor++) for (budget = 0; budget < 3; budget++) {
        memset(ram, 0xA5, 0x200000); memset(&menu, 0, sizeof(menu));
        g_item_top = top; menu.unk100.cur = initialCursor = cursor;
        g_facility_count = 16; g_coins = budgets[budget]; selected = top + cursor;
        for (i = 0; i < 16; i++) { g_shop_items[i] = 0x8200 + i; g_shop_prices[i] = 1103 * i; }
        fills = bars = tags = formats = revs = counts = 0;
        CoinShopOpen();
        CHECK(fills == 8 && bars == 27 && tags == 6 && formats == 3 && revs == 3 && counts == 2);
        CHECK(g_map_scroll_y == g_item_top * 12);
        CHECK(g_slots[32].attr == (g_item_top == 0 ? 0x12348001 : 0x12340001));
        CHECK(g_slots[33].attr == (g_item_top == 8 ? 0x12348001 : 0x12340001));
        CHECK(g_slot_cur == &g_slots[33]);
        CHECK(g_tilemap2[455] == 0x37F && g_tilemap2[528] == 0xCE && g_tilemap2[535] == 0x37F);
        cases++;
    }
    CHECK(munmap(ram, 0x200000) == 0);
    printf("coin shop: %d cases, %d checks passed\n", cases, checks);
    return 0;
}
