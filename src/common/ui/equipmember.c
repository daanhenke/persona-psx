/* Persona 1 (JP) - the equipment screen's member panel.
 *
 * Compiled into two overlays rather than called across the boundary:
 *                      DNG         ADV
 *   EquipDrawMember    0x8008DF8C  0x800936D4
 *
 * A unit of its own in ADV, after the item row; DNG's equipment screen
 * (src/dng/ui/equipscreen.c) includes it at its end.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/adv/personapage.h>
#include <persona/common/itemname.h>
#include <persona/common/char.h>
#include <persona/common/status.h>

/* A number right-aligned so its last digit lands in `last`. */
#define MEMBER_NUMBER(value, width, row, last)                                     TileMapWriteRowRev(g_hud_digits, AT(g_tilemap1, row, last), GLYPH_DIGIT0,                         FormatDecimal(value, g_hud_digits, width))

/* The two cells of the "unchanged" arrow. */
#define ARROW_SAME 0x360

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
