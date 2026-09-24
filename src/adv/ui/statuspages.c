/* Persona 1 (JP) - the status screen's pages.  ADV only.
 *   ADV 0x8008F1A8 StatusDrawMember  0x8008F848 StatusDrawName
 *       0x8008F8C0 StatusDrawPersona
 *
 * The member page: name, level and ailment on the top rows, then hp and sp
 * each over its maximum, three experience figures, the seven things equipped,
 * the five stats and the eight numbers the equipment and stats come to. The
 * Persona page is the same for one of the Personas: its name, level, five
 * stats and the pair a contact is weighed with.
 *
 * Every number is right-aligned into its field with FormatDecimal and
 * TileMapWriteRowRev; each field is cleared first, since a shorter number
 * would otherwise leave digits of the last one showing.
 */
#include <decomp/types.h>
#include <persona/common/char.h>
#include <persona/common/itemname.h>
#include <persona/common/persona.h>
#include <persona/common/tilemap.h>

/* The ailment labels, six cells each, by Char.status. */
extern u_char *g_status_labels[];
#define STATUS_LABEL_BASE 0x285

#define AT(row, col) (&g_tilemap1[(row) * MAP_W + (col)])

/* One number, right-aligned so its last digit lands in `last`. */
#define NUMBER(value, width, row, last)                                        \
    TileMapWriteRowRev(g_hud_digits, AT(row, last), GLYPH_DIGIT0,              \
                       FormatDecimal(value, g_hud_digits, width))

void StatusDrawMember(short member)
{
    Char *c;

    c = &g_chars[member];
    TileMapFillRect(AT(0, 14), 0, CHAR_NAME_CELLS, 1, MAP_W);
    TileMapWriteRow(c->name, AT(0, 14), 0, CHAR_NAME_CELLS);

    TileMapFillRect(AT(2, 16), 0, 8, 1, MAP_W);
    NUMBER(g_chars[member].level, 2, 2, 17);
    TileMapWriteRow(g_status_labels[g_chars[member].status], AT(2, 18),
                    STATUS_LABEL_BASE, 6);

    TileMapFillRect(AT(3, 16), 0, 4, 2, MAP_W);
    TileMapFillRect(AT(3, 21), 0, 3, 2, MAP_W);
    NUMBER(c->hp, 3, 3, 19);
    NUMBER(g_chars[member].hp_max, 3, 3, 23);
    NUMBER(g_chars[member].sp, 3, 4, 19);
    NUMBER(g_chars[member].sp_max, 3, 4, 23);

    TileMapFillRect(AT(6, 17), 0, 7, 3, MAP_W);
    NUMBER(g_chars[member].unk10, 7, 6, 23);
    NUMBER(g_chars[member].unk18, 7, 7, 23);
    NUMBER(g_chars[member].unk1C, 7, 8, 23);

    TileMapFillRect(AT(9, 17), 0, 2, 1, MAP_W);
    NUMBER(g_chars[member].unk56, 7, 9, 18);

    TileMapFillRect(AT(0, 28), 0, 10, CHAR_EQUIP + 1, MAP_W);
    DrawItemName(g_chars[member].equip[0], AT(0, 28), 0, 1);
    DrawItemName(g_chars[member].equip[1], AT(1, 28), 0, 1);
    DrawItemName(g_chars[member].equip[2], AT(2, 28), 0, 1);
    DrawItemName(g_chars[member].equip[3], AT(3, 28), 0, 1);
    DrawItemName(g_chars[member].equip[4], AT(4, 28), 0, 1);
    DrawItemName(g_chars[member].equip[5], AT(5, 28), 0, 1);
    DrawItemName(g_chars[member].equip[6], AT(6, 28), 0, 1);

    TileMapFillRect(AT(12, 4), 0, 2, CHAR_STATS, MAP_W);
    NUMBER(g_chars[member].stat[0], 2, 12, 5);
    NUMBER(g_chars[member].stat[1], 2, 13, 5);
    NUMBER(g_chars[member].stat[2], 2, 14, 5);
    NUMBER(g_chars[member].stat[3], 2, 15, 5);
    NUMBER(g_chars[member].stat[4], 2, 16, 5);

    TileMapFillRect(AT(8, 35), 0, 3, 8, MAP_W);
    NUMBER(g_chars[member].melee_atk, 3, 8, 37);
    NUMBER(g_chars[member].melee_hit, 3, 9, 37);
    NUMBER(g_chars[member].gun_atk, 3, 10, 37);
    NUMBER(g_chars[member].gun_hit, 3, 11, 37);
    NUMBER(g_chars[member].defence, 3, 12, 37);
    NUMBER(g_chars[member].evade, 3, 13, 37);
    NUMBER(g_chars[member].unk3A, 3, 14, 37);
    NUMBER(g_chars[member].unk3C, 3, 15, 37);
}

void StatusDrawName(short member)
{
    Char *c;

    c = &g_chars[member];
    TileMapFillRect(AT(0, 4), 0, CHAR_NAME_CELLS, 1, MAP_W);
    TileMapWriteRow(c->name, AT(0, 4), 0, CHAR_NAME_CELLS);
}

void StatusDrawPersona(short persona)
{
    Persona *p;

    p = &g_personas[persona];
    TileMapFillRect(AT(0, 17), 0, 10, 1, MAP_W);
    TileMapWriteRow(p->name, AT(0, 17), 0, 10);

    TileMapFillRect(AT(1, 19), 0, 2, 1, MAP_W);
    NUMBER(g_personas[persona].level, 2, 1, 20);

    TileMapFillRect(AT(12, 4), 0, 2, PERSONA_STATS, MAP_W);
    NUMBER(g_personas[persona].stat[0], 2, 12, 5);
    NUMBER(g_personas[persona].stat[1], 2, 13, 5);
    NUMBER(g_personas[persona].stat[2], 2, 14, 5);
    NUMBER(g_personas[persona].stat[3], 2, 15, 5);
    NUMBER(g_personas[persona].stat[4], 2, 16, 5);

    TileMapFillRect(AT(8, 35), 0, 3, 2, MAP_W);
    NUMBER(g_personas[persona].unk10, 3, 8, 37);
    NUMBER(g_personas[persona].unk12, 3, 9, 37);
}
