/* Persona 1 (JP) - the status screen's main page.  DNG's copy.
 *   DNG 0x8008A990
 *
 * The field's build of ADV's page (src/adv/ui/statusmain.c, which describes
 * it), against the field's formatter and writers.
 */
#define TILEMAP_INT_COUNT
#define NAME_KR
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

extern void DrawPersonaKeyName();

/* Clear a field and draw one number into it, its last digit at `last`. */
#define FIELD(value, width, row, col, last)                                    \
    TileMapFillRect(AT(row, col), 0, width, 1, MAP_W);                         \
    TileMapWriteRowRev(g_hud_digits, AT(row, last), GLYPH_DIGIT0,              \
                       FormatDecimal(value, g_hud_digits, width))

#define NUMBER(value, width, row, last)                                        \
    TileMapWriteRowRev(g_hud_digits, AT(row, last), GLYPH_DIGIT0,              \
                       FormatDecimal(value, g_hud_digits, width))

#ifdef NON_MATCHING
void StatusDrawMain(u_char slot)
{
    int      member = g_party[slot];   /* then each digit count, then a row */
    Char    *c = &g_chars[member];
    Persona *personas = g_personas;
    int      next;
    int      bank;
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
    FIELD(g_chars[member].mag_atk, 3, 23, 24, 26);
    FIELD(g_chars[member].mag_def, 3, 24, 24, 26);

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
    member = FormatDecimal(c->hp, g_hud_digits, 3);
    bank = 0;
    if (!(c->hp_max / 4 < c->hp)) {
        bank = BANK_DANGER;
    }
    TileMapWriteRowRev(g_hud_digits, AT(26, 22),
                       GLYPH_DIGIT0 + bank * GLYPH_BANK, member);
    NUMBER(c->hp_max, 3, 26, 26);
    member = FormatDecimal(c->sp, g_hud_digits, 3);
    bank = 0;
    if (!(c->sp_max / 4 < c->sp)) {
        bank = BANK_DANGER;
    }
    TileMapWriteRowRev(g_hud_digits, AT(27, 22),
                       GLYPH_DIGIT0 + bank * GLYPH_BANK, member);
    NUMBER(c->sp_max, 3, 27, 26);

    TileMapFillRect(AT(28, 18), 0, 10, 1, MAP_W);
    if (c->status != 0) switch (c->status) {
    case STATUS_D:
        TileMapWriteRow(g_status_names[STATUS_D], AT(28, 18), LABEL_BASE, 6);
        break;
    case STATUS_10:
        TileMapWriteRow(g_status_names[STATUS_10], AT(28, 18), LABEL_BASE, 4);
        break;
    }

    TileMapFillRect(AT(26, 3), 0, 10, CHAR_LIST_N, MAP_W);
    for (member = 0; member < CHAR_LIST_N; member++) {
        bank = c->list[member];
        if (bank != 0xFF && !c->blocked) {
            key = personas[bank].key;
            base = c->entry == member ? LABEL_BASE : 0;
            DrawPersonaKeyName(key, AT(26 + member, 3), base);
        } else {
            TileMapWriteRow(g_persona_list_rule, AT(26 + member, 4), GLYPH_BANK, 8);
        }
    }

    TileMapFillRect(g_tilemap2, 0, 8, 1, MAP_W);
    TileMapWriteRow(c->name, g_tilemap2, 0, 8);
}
#else
INCLUDE_ASM("dng/nonmatchings/ui/statusmain", StatusDrawMain);
#endif
