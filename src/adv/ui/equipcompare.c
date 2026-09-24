/* Persona 1 (JP) - the equipment page's comparison.  ADV only.
 *   ADV 0x80090D14
 *
 * Draws a member's name, the seven things they have equipped, and every number
 * a change of equipment moves, taken from `preview` - a copy of the member's
 * record with the candidate item already in it. Each number is drawn in the
 * font's plain bank when it would stay the same, in the next bank when it
 * would fall and in the one after when it would rise, with an arrow beside it
 * saying which.
 *
 *      col  5      name, then the seven equipment rows below it
 *      col 22      the five stats: arrow, then two digits
 *      col 32      the six derived values: arrow, then three digits
 */
#include <decomp/types.h>
#include <persona/common/char.h>
#include <persona/common/item.h>
#include <persona/common/tilemap.h>

#define NAME_AT     5
#define EQUIP_AT    (2 * MAP_W + NAME_AT)
#define EQUIP_W     10
#define NOTHING_W   7
#define STAT_ARROW  (3 * MAP_W + 22)
#define VALUE_ARROW (2 * MAP_W + 32)

/* The glyph banks are GLYPH_BANK apart: plain, lower, higher. */
#define GLYPH_BANK  0xD7
#define SAME        0
#define LOWER       1
#define HIGHER      2

extern const u_char str_nothing[];

/* Which bank a number goes in: how the preview's value compares with the
   member's own. */
#define COMPARE(d, cur, new)                                                   \
    d = (cur) > (new);                                                         \
    if ((cur) < (new)) {                                                       \
        d = HIGHER;                                                            \
    }

#define EQUIP_ROW(k)                                                           \
    if ((short)c->equip[k] != 0) {                                             \
        TileMapWriteRow(g_item_defs[(short)c->equip[k]].name,                  \
                        &g_tilemap1[EQUIP_AT + (k) * MAP_W], 0, EQUIP_W);      \
    } else {                                                                   \
        TileMapWriteRow(str_nothing, &g_tilemap1[EQUIP_AT + (k) * MAP_W + 1],  \
                        GLYPH_BANK, NOTHING_W);                                \
    }

#define STAT_ROW(k)                                                            \
    COMPARE(d, c->stat[k], preview->stat[k]);                                  \
    TileMapWriteRowRev(g_hud_digits,                                           \
                       &g_tilemap1[STAT_ARROW + (k) * MAP_W + 3],              \
                       (u_short)(GLYPH_DIGIT0 + d * GLYPH_BANK),                \
                       FormatDecimal(preview->stat[k], g_hud_digits, 2));      \
    g_tilemap1[STAT_ARROW + (k) * MAP_W] = arrow[d];                           \
    g_tilemap1[STAT_ARROW + (k) * MAP_W + 1] = arrow[d] + 1;

#define VALUE_ROW(k, field)                                                    \
    COMPARE(d, c->field, preview->field);                                      \
    TileMapWriteRowRev(g_hud_digits,                                           \
                       &g_tilemap1[VALUE_ARROW + (k) * MAP_W + 4],             \
                       (u_short)(GLYPH_DIGIT0 + d * GLYPH_BANK),                \
                       FormatDecimal(preview->field, g_hud_digits, 3));        \
    g_tilemap1[VALUE_ARROW + (k) * MAP_W] = arrow[d];                          \
    g_tilemap1[VALUE_ARROW + (k) * MAP_W + 1] = arrow[d] + 1;

void EquipDrawCompare(short member, Char *preview)
{
    Char   *c = &g_chars[member];
    /* Two cells each: level, down, up. */
    u_short arrow[3] = { 0x360, 0x35E, 0x35C };
    int     d;

    TileMapFillRect(&g_tilemap1[NAME_AT], 0, 8, 1, MAP_W);
    TileMapWriteRow(c->name, &g_tilemap1[NAME_AT], 0, 8);

    TileMapFillRect(&g_tilemap1[EQUIP_AT], 0, EQUIP_W, CHAR_EQUIP, MAP_W);
    EQUIP_ROW(0);
    EQUIP_ROW(1);
    EQUIP_ROW(2);
    EQUIP_ROW(3);
    EQUIP_ROW(4);
    EQUIP_ROW(5);
    EQUIP_ROW(6);

    TileMapFillRect(&g_tilemap1[STAT_ARROW + 2], 0, 2, CHAR_STATS, MAP_W);
    STAT_ROW(0);
    STAT_ROW(1);
    STAT_ROW(2);
    STAT_ROW(3);
    STAT_ROW(4);

    TileMapFillRect(&g_tilemap1[VALUE_ARROW + 2], 0, 3, 6, MAP_W);
    VALUE_ROW(0, melee_atk);
    VALUE_ROW(1, melee_hit);
    VALUE_ROW(2, gun_atk);
    VALUE_ROW(3, gun_hit);
    VALUE_ROW(4, defence);
    VALUE_ROW(5, evade);
}
