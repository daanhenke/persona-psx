/* Persona 1 (JP) - an item's name, a bag row, and the tactics marker's cels.
 *
 * Compiled into two overlays rather than called across the boundary:
 *                        ADV         DNG
 *   DrawItemName         0x8008FB18  0x80093BB8
 *   DrawBagItemRow       0x8008FB7C  0x80093C1C
 *   TacticsSetCels       0x8008FC78  0x80093D18
 *
 * All three are defined old-style: every narrow argument is narrowed here,
 * whatever the caller handed over.
 */
#define NAME_KR
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/item.h>
#include <persona/common/itemname.h>

#define MAP_W        40
#define NAME_CELLS   10
#define ROW_CELLS    0xC
#define ROW_COUNT_AT 11

/* The bag in the save game: an item id in the low nine bits, the count
   above them. */
#define g_items    ((u_short *)0x801F267C)
#define ITEM_SHIFT 9

/* The definitions the bag row reads its names from, reached by address. */
#define BAG_DEFS ((ItemDef *)0x80220000)

/* The "nothing" line: the dimmed bank, one cell in. */
#define NOTHING_BANK  0xD7
#define NOTHING_CELLS 7

/* The tactics byte of the save-game options: the row in its upper bits, the
   column in the low three. The marker's cels are picked from it. */
#define TACTICS ((u_char *)0x801F2AC4)
#define TACTICS_MARKS 8
extern u_char D_801F2AC6;

/* Two of the cinema's cel lists, seen with their cels: eight behind the
   first head, one behind the second (the head itself is cel.h's CelHead). */
typedef struct {
    u_char  head[8];
    GsCELL  cel[TACTICS_MARKS];
} TacticCels;
typedef struct {
    u_char  head[8];
    GsCELL  cel[1];
} TacticCel;
extern TacticCels g_cinema_cels0;
extern TacticCel  g_cinema_cels6;

extern u_char str_nothing[];
extern u_char g_hud_digits[];

extern void  TileMapWriteRow(const u_char *src, short *dst, u_short base,
                             u_short count);
extern void  TileMapWriteRowRev(const u_char *src, short *dst, u_short base,
                                u_short count);
extern void  TileMapFillRect(short *dst, short value, u_short w, u_short h,
                             u_short stride);
extern short FormatDecimal(u_int value, u_char *dst, u_short width);

/* An item's name; for an id of 0 the "nothing" line when asked, else
   nothing at all. */
void DrawItemName(id, dst, base, nothing)
    short   id;
    short  *dst;
    u_short base;
    short   nothing;
{
    if (id != 0) {
        TileMapWriteRow(g_item_defs[id].name, dst, base, NAME_CELLS);
    } else if (nothing != 0) {
        TileMapWriteRow(str_nothing, dst + 1, NOTHING_BANK, NOTHING_CELLS);
    }
}

/* One row of the bag: the name, and the count two digits wide. */
void DrawBagItemRow(slot, dst, base, digits)
    short   slot;
    short  *dst;
    u_short base;
    u_short digits;
{
    u_short *entry = &g_items[slot];
    ItemDef *defs = &BAG_DEFS[slot];
    int      id;

    TileMapFillRect(dst, 0, ROW_CELLS, 1, MAP_W);
    id = *entry & ITEM_ID;
    if (id != 0 && (*entry >> ITEM_SHIFT) != 0) {
        TileMapWriteRow(defs[id].name, dst, base, NAME_CELLS);
        TileMapWriteRowRev(g_hud_digits, dst + ROW_COUNT_AT, digits,
                           FormatDecimal(*entry >> ITEM_SHIFT, g_hud_digits,
                                         2));
    }
}

/* Stores the tactics byte and points the eight marker cels, and the
   cinema's single one, at the part of the page it picks. */
void TacticsSetCels(tactics)
    u_char tactics;
{
    u_char *t = TACTICS;
    u_char  i;

    D_801F2AC6 = tactics;
    for (i = 0; i < TACTICS_MARKS; i++) {
        g_cinema_cels0.cel[i].u = (t[2] & 7) << 5;
        g_cinema_cels0.cel[i].v = (t[2] >> 3) * 56 - 56;
        g_cinema_cels0.cel[i].tpage = t[2] + 0x1F8;
    }
    g_cinema_cels6.cel[0].u = (t[2] & 7) << 5;
    g_cinema_cels6.cel[0].v = (t[2] >> 3) * 56 - 56;
    g_cinema_cels6.cel[0].tpage = t[2] + 0x1F8;
}
