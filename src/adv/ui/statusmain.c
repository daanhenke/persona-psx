/* Persona 1 (JP) - the status screen's main page.  ADV only.
 *   ADV 0x8007C38C
 *
 * Everything the screen shows about one party member at once: the stat bars,
 * level and the three experience figures with what the next level still
 * needs, the eight derived numbers, the seven things equipped, the five
 * stats, hp and sp over their maxima - each drawn in the warning bank once it
 * has fallen to a quarter of its maximum - the ailment if it is one of the two
 * the screen names, the three Personas the member carries with the one in use
 * picked out, and the member's name on the header layer.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/common/char.h>
#include <persona/common/itemname.h>
#include <persona/common/persona.h>
#include <persona/common/status.h>
#include <persona/common/tilemap.h>

#define AT(row, col) (&g_tilemap1[(row) * MAP_W + (col)])

#define GLYPH_BANK   0xD7
#define BANK_DANGER  3
#define LEVEL_MAX    99

/* The two ailments with a label on this page, and their labels. */
#define STATUS_D     0xD
#define STATUS_10    0x10
#define LABEL_BASE   0x285
extern u_char g_status_names[][8];

extern const u_char g_persona_list_rule[];

/* Clear a field and draw one number into it, its last digit at `last`. */
#define FIELD(value, width, row, col, last)                                    \
    TileMapFillRect(AT(row, col), 0, width, 1, MAP_W);                         \
    TileMapWriteRowRev(g_hud_digits, AT(row, last), GLYPH_DIGIT0,              \
                       FormatDecimal(value, g_hud_digits, width))

#define NUMBER(value, width, row, last)                                        \
    TileMapWriteRowRev(g_hud_digits, AT(row, last), GLYPH_DIGIT0,              \
                       FormatDecimal(value, g_hud_digits, width))

/* 98.54%: the member index and the record offset sit one saved register over
   from the original's; the permuter is on it. */
#ifdef NON_MATCHING
void StatusDrawMain(u_char slot)
{
    int      member = g_party[slot];
    Char    *c = &g_chars[member];
    Persona *personas = g_personas;
    int      next;
    int      n;
    int      bank;
    int      i;
    int      key;
    u_short  base;

    DrawCharStatBars(c);

    FIELD(g_chars[member].level, 2, 3, 21, 22);
    FIELD(g_chars[member].unk10, 7, 4, 20, 26);
    FIELD(g_chars[member].unk18, 7, 5, 20, 26);
    FIELD(g_chars[member].unk56, 2, 7, 23, 24);

    TileMapFillRect(AT(8, 20), 0, 7, 2, MAP_W);
    NUMBER(g_chars[member].unk1C, 7, 8, 26);
    next = ExpToLevel(g_chars[member].unk56, g_party[slot],
                      g_chars[member].unk1C);
    if (g_chars[member].unk56 == LEVEL_MAX) {
        next = 0;
    }
    NUMBER(next, 7, 9, 26);

    FIELD(g_chars[member].melee_atk, 3, 17, 24, 26);
    FIELD(g_chars[member].melee_hit, 3, 18, 24, 26);
    FIELD(g_chars[member].gun_atk, 3, 19, 24, 26);
    FIELD(g_chars[member].gun_hit, 3, 20, 24, 26);
    FIELD(g_chars[member].defence, 3, 21, 24, 26);
    FIELD(g_chars[member].evade, 3, 22, 24, 26);
    FIELD(g_chars[member].unk3A, 3, 23, 24, 26);
    FIELD(g_chars[member].unk3C, 3, 24, 24, 26);

    TileMapFillRect(AT(17, 4), 0, 10, CHAR_EQUIP, MAP_W);
    DrawItemName(g_chars[member].equip[0], AT(17, 4), 0, 1);
    DrawItemName(g_chars[member].equip[1], AT(18, 4), 0, 1);
    DrawItemName(g_chars[member].equip[2], AT(19, 4), 0, 1);
    DrawItemName(g_chars[member].equip[3], AT(20, 4), 0, 1);
    DrawItemName(g_chars[member].equip[4], AT(21, 4), 0, 1);
    DrawItemName(g_chars[member].equip[5], AT(22, 4), 0, 1);
    DrawItemName(g_chars[member].equip[6], AT(23, 4), 0, 1);

    TileMapFillRect(AT(11, 16), 0, 2, CHAR_STATS, MAP_W);
    NUMBER(g_chars[member].stat[0], 2, 11, 17);
    NUMBER(g_chars[member].stat[1], 2, 12, 17);
    NUMBER(g_chars[member].stat[2], 2, 13, 17);
    NUMBER(g_chars[member].stat[3], 2, 14, 17);
    NUMBER(g_chars[member].stat[4], 2, 15, 17);

    TileMapFillRect(AT(26, 20), 0, 3, 2, MAP_W);
    TileMapFillRect(AT(26, 24), 0, 3, 2, MAP_W);
    n = FormatDecimal(c->hp, g_hud_digits, 3);
    bank = 0;
    if (!(g_chars[member].hp_max / 4 < c->hp)) {
        bank = BANK_DANGER;
    }
    TileMapWriteRowRev(g_hud_digits, AT(26, 22),
                       (u_short)(GLYPH_DIGIT0 + bank * GLYPH_BANK), (u_short)n);
    NUMBER(g_chars[member].hp_max, 3, 26, 26);
    n = FormatDecimal(g_chars[member].sp, g_hud_digits, 3);
    bank = 0;
    if (!(g_chars[member].sp_max / 4 < g_chars[member].sp)) {
        bank = BANK_DANGER;
    }
    TileMapWriteRowRev(g_hud_digits, AT(27, 22),
                       (u_short)(GLYPH_DIGIT0 + bank * GLYPH_BANK), (u_short)n);
    NUMBER(g_chars[member].sp_max, 3, 27, 26);

    TileMapFillRect(AT(28, 18), 0, 10, 1, MAP_W);
    if (g_chars[member].status != 0) switch (g_chars[member].status) {
    case STATUS_D:
        TileMapWriteRow(g_status_names[STATUS_D], AT(28, 18), LABEL_BASE, 6);
        break;
    case STATUS_10:
        TileMapWriteRow(g_status_names[STATUS_10], AT(28, 18), LABEL_BASE, 4);
        break;
    }

    TileMapFillRect(AT(26, 3), 0, 10, CHAR_LIST_N, MAP_W);
    for (i = 0; i < CHAR_LIST_N; i++) {
        if (c->list[i] != 0xFF && !c->blocked) {
            key = personas[c->list[i]].key;
            base = c->entry == i ? LABEL_BASE : 0;
            TileMapFillRect(AT(26, 3) + i * MAP_W, 0, 10, 1, MAP_W);
            if (key != 0) {
                TileMapWriteRow(g_persona_defs[key].unk08,
                                AT(26, 3) + i * MAP_W, base, 10);
            }
        } else {
            TileMapWriteRow(g_persona_list_rule, AT(26 + i, 4), GLYPH_BANK, 8);
        }
    }

    TileMapFillRect(g_tilemap2, 0, 8, 1, MAP_W);
    TileMapWriteRow(c->name, g_tilemap2, 0, 8);
}
#else
INCLUDE_ASM("adv/nonmatchings/ui/statusmain", StatusDrawMain);
#endif
