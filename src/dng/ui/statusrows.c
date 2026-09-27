/* Persona 1 (JP) - rows the status and formation screens redraw.  DNG only.
 *   0x80090374 SaveRowsDraw  0x800903B4 SaveRowDraw
 *   0x80090464 StatusDrawPersonaRows  0x80090688 StatusDrawPersonaNames
 *
 * The member's three Persona entries - name, level, spell slots, the
 * equipped one in the highlight bank - and a column of their names; and a
 * two-column list of sixteen records from the save area, each blank or
 * marked empty.
 */
#define TILEMAP_INT_COUNT
#include <decomp/types.h>
#include <persona/common/char.h>
#include <persona/common/persona.h>
#include <persona/common/status.h>
#include <persona/common/tilemap.h>

#define AT(map, row, col) (&(map)[(row) * MAP_W + (col)])

/* Sixteen four-byte records in the save area; 0xFF in the first byte marks
   an empty one. */
#define g_save_2AF0 ((u_char *)0x801F2AF0)

/* The highlight bank the equipped Persona is drawn from. */
#define BANK_EQUIPPED 0x285

extern const u_char str_empty[];
extern const u_char g_persona_list_rule[];
extern void DrawPersonaKeyName();

void SaveRowDraw(u_char n);

void SaveRowsDraw(void)
{
    u_char i;

    for (i = 0; i < 16; i++) {
        SaveRowDraw(i);
    }
}

void SaveRowDraw(u_char n)
{
    u_char *rec = &g_save_2AF0[n * 4];
    short  *dst;

    dst = g_tilemap2 + (n / 2) * MAP_W + (n & 1) * 14 + 6;
    TileMapFillRect(dst, 0, 10, 1, MAP_W);
    if (*rec == 0xFF) {
        TileMapWriteRow(str_empty, dst, 0xD7, 9);
    }
}

void StatusDrawPersonaRows(short slot)
{
    Char    *c;
    u_long   personas;   /* added index-first: an integer */
    Persona *p;
    int      i;
    int      k;
    int      hl;
    int      bank;

    /* `i` holds the party index first, then counts the rows. */
    i = g_party[slot];
    c = &g_chars[i];
    personas = (u_long)g_personas;
    TileMapFillRect(AT(g_tilemap2, 1, 1), 0, 21, 3, MAP_W);
    for (i = 0; i < 3; i++) {
        k = c->list[i];
        if (k != 0xFF && !c->blocked) {
            p = (Persona *)(k * sizeof(Persona) + personas);
            hl = c->entry == i ? BANK_EQUIPPED : 0;
            DrawPersonaKeyName(p->key, AT(g_tilemap2, 1 + i, 1), hl);
            bank = hl + GLYPH_DIGIT0;
            TileMapWriteRowRev(g_hud_digits, AT(g_tilemap2, 1 + i, 19), bank,
                               FormatDecimal(p->level, g_hud_digits, 2));
            FormatDecimal(p->slots, g_hud_digits, 1);
            TileMapWriteRowRev(g_hud_digits, AT(g_tilemap2, 1 + i, 21), bank,
                               1);
            *AT(g_tilemap2, 1 + i, 20) = hl + 0xCD;
            TileMapWriteRow(str_cell_run, AT(g_tilemap2, 1 + i, 12), 0x36F, 5);
        } else {
            TileMapWriteRow(g_persona_list_rule, AT(g_tilemap2, 1 + i, 2),
                            0xD7, 8);
            TileMapWriteRow(str_cell_run, AT(g_tilemap2, 1 + i, 12), 0x36F, 5);
            TileMapWriteRow(g_persona_list_rule, AT(g_tilemap2, 1 + i, 18),
                            0xD7, 4);
        }
    }
}

void StatusDrawPersonaNames(u_char member)
{
    Char    *c;
    Persona *p;
    u_char   i;

    c = &g_chars[member];
    TileMapFillRect(AT(g_tilemap1, 3, 6), 0, 10, 3, MAP_W);
    for (i = 0; i < 3; i++) {
        p = &g_personas[c->list[i]];
        if (g_personas[c->list[i]].key != 0) {
            TileMapWriteRow(p->name, AT(g_tilemap1, 3 + i, 6), 0, 10);
        }
    }
}
