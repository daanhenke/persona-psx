/* Persona 1 (JP) - a spell's name in a menu row.
 *
 * Compiled into two overlays rather than called across the boundary:
 *   ADV 0x8007B7E0   S2D 0x8007A660
 *
 * The row is cleared first, so an empty slot is blank unless the caller asks
 * for a rule instead - the same eight long-vowel marks the Persona list draws
 * under its names, inset one cell.
 *
 * The base picks the glyph bank the name is drawn from, which is how a spell
 * that cannot be cast right now comes out greyed.
 */
#include <types.h>
#include <persona/common/spell.h>

#define MAP_W      40
#define NAME_CELLS 10

/* The rule, and where its glyphs live in the font. */
#define RULE_GLYPHS 8
#define RULE_BASE   0xD7

extern u_char g_persona_list_rule[];

extern void TileMapWriteRow(const u_char *src, short *dst, u_short base,
                            u_short count);
extern void TileMapFillRect(short *dst, short value, u_short w, u_short h,
                            u_short stride);

void DrawSpellName(short spell, short *dst, u_short base, short rule)
{
    TileMapFillRect(dst, 0, NAME_CELLS, 1, MAP_W);
    if (spell != 0) {
        TileMapWriteRow(g_spell_data[spell].name, dst, base, NAME_CELLS);
    } else if (rule != 0) {
        TileMapWriteRow(g_persona_list_rule, &dst[1], RULE_BASE, RULE_GLYPHS);
    }
}
