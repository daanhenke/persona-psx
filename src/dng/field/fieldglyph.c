/* Persona 1 (JP) - primitive palettes, glyph decoding and the moon icon.
 * DNG only.
 *   0x8006FE8C FieldSetPrimClut
 *   0x8006FED0 FieldDecodeGlyph
 *   0x8006FFF4 FieldSetMoonIcon
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/dng/field.h>

/* Points every primitive of a packet list at palette `clut`: the list's
   count is its third halfword, and each 8-byte entry keeps the palette in
   the low five bits of its fourth halfword, from the eighth halfword on. */
void FieldSetPrimClut(u_short *list, u_short clut)
{
    u_short n, i;

    list += 2;
    n = *list;
    list += 5;
    for (i = 0; i < n; i++, list += 4) {
        *list = clut | (*list & 0xFFE0);
    }
}

/* The field's glyphs are 16 by 16 at one bit a pixel, 32 bytes each. */
#define GLYPHS ((u_char *)0x801E0000)

/* Expands glyph `n` into the scene's scratch image at four bits a pixel,
   bottom row first, and drops a shadow one pixel right and down: each row,
   shifted five bits, is or'ed into the row below it. The scene pointer is
   read once, the two words are filled by index, and the low word's carry
   into the high one is read before either shadow store. */
void FieldDecodeGlyph(u_short n)
{
    u_char    *src;
    int        row, half, k;
    u_long     words[2];
    u_int      bits;
    DngScene  *sc;

    src = GLYPHS + 31 + n * 32;
    sc = g_scene;
    for (row = 15; row >= 0; row--) {
        words[1] = 0;
        words[0] = 0;
        for (half = 1; half >= 0; half--) {
            bits = *src--;
            for (k = 7; k >= 0; k--) {
                words[half] |= (bits & 1) << (k * 4);
                bits >>= 1;
            }
        }
        sc->glyph[row][0] = words[0];
        sc->glyph[row][1] = words[1];
        if (row < 15) {
            u_long c = sc->glyph[row][0];

            sc->glyph[row + 1][0] = (c << 5) | sc->glyph[row + 1][0];
            sc->glyph[row + 1][1] = (sc->glyph[row][1] << 5) | (c >> 27) | sc->glyph[row + 1][1];
        }
    }
}

/* The moon icon is sprite MOON_SPRITE; the table gives each phase's cell as
   row * 10 + column, 24 pixels a cell. */
#define MOON_SPRITE 68
#define MOON_CELL   24

void FieldSetMoonIcon(void)
{
    u_char *phase;

    phase = &MOON_PHASE;
    g_scene->sprites[MOON_SPRITE].u = (u_char)(g_moon_cells[*phase] % 10) * MOON_CELL;
    g_scene->sprites[MOON_SPRITE].v = (u_char)(g_moon_cells[*phase] / 10) * MOON_CELL - 0x70;
}
