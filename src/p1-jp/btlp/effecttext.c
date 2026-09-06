/* Persona 1 (JP) - an effect's text, drawn where the effect is.  BTLP only.
 *   0x80079128 BtlDrawEffectText
 *
 * The glyph drawer works from a pen of its own rather than from the record, so
 * this sets the pen before handing over: eight pixels right of the effect's
 * origin and four above it, which is where the damage and status words sit
 * relative to whatever they are being drawn over.
 */
#include <types.h>

/* Where the text sits relative to the effect. */
#define EFFECT_TEXT_DX 8
#define EFFECT_TEXT_DY (-4)

/* The text is 0xC into the record. */
#define EFFECT_TEXT 0xC

extern short    g_btl_effect_ox;
extern short    g_btl_effect_oy;
extern short    g_btl_glyph_x;
extern short    g_btl_glyph_y;
extern u_short  g_btl_clut[];

extern int BtlDrawGlyphs(const u_char *text, short clut);

void BtlDrawEffectText(u_char *effect)
{
    g_btl_glyph_x = g_btl_effect_ox + EFFECT_TEXT_DX;
    g_btl_glyph_y = g_btl_effect_oy + EFFECT_TEXT_DY;
    BtlDrawGlyphs(effect + EFFECT_TEXT, g_btl_clut[32]);
}
