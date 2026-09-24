#ifndef PERSONA_COMMON_FONT_H
#define PERSONA_COMMON_FONT_H

/* Persona 1 (JP) - the message font and its one-glyph upload.
 *
 * The font is 1bpp, 16 pixels square: two bytes a row, 32 bytes a glyph.
 * FontUploadGlyph (src/common/gfx/fontglyph.c) expands one into a 4bpp
 * staging cell - the glyph in colour 2 over a colour-1 shadow - and queues it
 * for VRAM.
 */
#include <decomp/types.h>

#define GLYPH_BYTES 32

/* The font's bitmaps, loaded above the overlays and reached by hardcoded
   address. */
#define g_font_bits ((u_char *)0x801E0000)

/* The 4bpp staging cell a glyph is expanded into: 16 rows of 8 bytes. S2D's
   sits 0x20000 higher, which is what WORK_BIAS says. */
#define g_glyph_buf ((u_char *)(0x800EB1CC + WORK_BIAS))

void FontUploadGlyph(short x, short y, u_short glyph);

#endif
