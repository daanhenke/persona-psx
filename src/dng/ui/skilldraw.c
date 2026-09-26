/* Persona 1 (JP) - the skills screen's layers.  DNG only.
 *   0x80089AF4 SkillScreenLayout   0x80089CCC SkillPersonasDraw
 *   0x80089F8C SkillSpellsDraw
 *
 * The layout, the member's SP and Persona list, and the chosen Persona's
 * seven spells. A Persona whose cost the member's SP cannot cover, and a
 * spell that cannot be cast here or now, come out in the greyed glyph bank.
 * ADV has the same three at 0x8007AF78, 0x8007B288 and 0x8007B554 (asm).
 */
#define NAME_KR
#define PERSONAPAGE_DNG
#define ITEM_USABLE_INT
#define TILEMAP_INT_COUNT
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/char.h>
#include <persona/common/persona.h>
#include <persona/common/formation.h>
#include <persona/common/spell.h>
#include <persona/common/status.h>
#include <persona/adv/personapage.h>

/* The glyph between SP and its maximum, and the bank a low count is drawn
   from (a quarter or less of the maximum). */
#define GLYPH_SLASH 0xCA
#define BANK_LOW    3
/* SpellData.kind: the spell can be cast outside battle. */
#define SPELL_FIELD 0x40

extern u_short g_menu_bg_rle[];
extern u_char  g_persona_list_rule[];

extern void TileMapWriteRun12(short *dst);
extern void BgBoxShow(void);
extern void DrawStatusHud(void);
extern void DrawPersonaKeyName();

void SkillScreenLayout(void)
{
    short i;

    TileMapFillRect(g_tilemap0, 0, MAP_W, 0x40, MAP_W);
    TileMapDrawWindow(AT(g_tilemap0, 0, 8), 0x1D, 0xB, MAP_W);
    TileMapDrawBox(AT(g_tilemap0, 1, 9), 0x1B, 9, MAP_W);
    TileMapBlitRle(g_menu_bg_rle, AT(g_tilemap0, 11, 0), MAP_W);
    TileMapWriteRun12(AT(g_tilemap0, 2, 11));
    for (i = 0; i < 3; i++) {
        *AT(g_tilemap2, 5 + i, 0) = 0x418 + i;
        TileMapWriteBar(AT(g_tilemap0, 6 + i, 10), 1);
        TileMapWriteBar(AT(g_tilemap0, 6 + i, 11), 10);
        TileMapWriteBar(AT(g_tilemap0, 6 + i, 21), 3);
    }
    for (i = 0; i < 7; i++) {
        TileMapWriteBar(AT(g_tilemap0, 2 + i, 25), 10);
    }
    TileMapWriteRow(str_cell_run, AT(g_tilemap2, 2, 3), 0x37A, 2);
    TileMapWriteRow(str_cell_run, AT(g_tilemap2, 4, 3), 0x457, 6);
    TileMapWriteRow(str_cell_run, AT(g_tilemap2, 0, 17), 0x45D, 4);
    BgBoxShow();
    DrawStatusHud();
}

/* The member's name and SP, then their three Personas with each one's cost;
   an empty entry, or all three while the list is blocked, shows the rule. */
void SkillPersonasDraw(member)
    short member;
{
    int      i = g_party_at[member];   /* the member, a count, a Persona */
    Char    *c = &g_chars[i];
    Persona *personas = g_personas;
    int      row;
    int      bank;

    TileMapFillRect(AT(g_tilemap2, 1, 3), 0, 8, 1, MAP_W);
    TileMapWriteRow(c->name, AT(g_tilemap2, 1, 3), 0, 8);
    TileMapFillRect(AT(g_tilemap2, 2, 7), 0, 7, 1, MAP_W);
    *AT(g_tilemap2, 2, 10) = GLYPH_SLASH;
    row = 0;
    i = FormatDecimal(c->sp, g_hud_digits, 3);
    if (c->sp_max / 4 >= c->sp) {
        row = BANK_LOW;
    }
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap2, 2, 9),
                       row * 0xD7 + GLYPH_DIGIT0, i);
    TileMapWriteRowRev(g_hud_digits, AT(g_tilemap2, 2, 13), GLYPH_DIGIT0,
                       FormatDecimal(c->sp_max, g_hud_digits, 3));
    TileMapFillRect(AT(g_tilemap2, 5, 1), 0, 0xD, 3, MAP_W);
    for (row = 0; row < 3; row++) {
        i = c->list[row];
        if (i != 0xFF && !c->blocked) {
            bank = c->sp < personas[i].sp_cost;
            DrawPersonaKeyName(personas[i].key, AT(g_tilemap2, 5 + row, 1),
                               bank * 0xD7);
            TileMapWriteRowRev(g_hud_digits, AT(g_tilemap2, 5 + row, 13),
                               bank * 0xD7 + GLYPH_DIGIT0,
                               FormatDecimal(personas[i].sp_cost, g_hud_digits,
                                             3));
        } else {
            TileMapWriteRow(g_persona_list_rule, AT(g_tilemap2, 5 + row, 2),
                            0xD7, 8);
            TileMapWriteRow(g_persona_list_rule, AT(g_tilemap2, 5 + row, 11),
                            0xD7, 3);
        }
    }
}

/* The Persona's seven spells, greyed while the member cannot pay its cost,
   while the list is blocked, or where the spell does nothing out here.

   The party index and then the Persona id go through one block-scoped
   local: kept local to the entry block, it ranks below the two record
   bases in local-alloc, as the original's does. */
void SkillSpellsDraw(member, persona)
    short member;
    short persona;
{
    Char    *c;
    Persona *p;
    int      i;
    int      grey;

    {
        int k = g_party_at[member];

        c = &g_chars[k];
        k = c->list[persona];
        p = &g_personas[k];
    }

    for (i = 0; i < PERSONA_SPELLS; i++) {
        grey = 0;
        if (c->sp < p->sp_cost || c->blocked ||
            !(g_spell_data[p->spell[i]].kind & SPELL_FIELD) ||
            !ItemUsableAny(p->spell[i])) {
            grey = 1;
        }
        if (c->blocked) {
            DrawSpellName(0, AT(g_tilemap2, 1 + i, 15), grey * 0xD7, 1);
        } else {
            DrawSpellName(p->spell[i], AT(g_tilemap2, 1 + i, 15), grey * 0xD7, 1);
        }
    }
}
