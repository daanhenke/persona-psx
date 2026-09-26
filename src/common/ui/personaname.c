/* Persona 1 (JP) - a Persona's name in a menu row.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   ADV @ 0x8007B754   S2D @ 0x8007A5D4   DNG @ 0x8008A18C
 *
 * The row is cleared first, so an id of 0 leaves it blank rather than writing
 * anything - which is how an empty stock slot is drawn. The names live in main
 * RAM with the rest of the Persona table, ten cells apiece.
 */
#include <decomp/types.h>
#include <persona/common/persona.h>

#define MAP_W 40
#define NAME_CELLS 10

extern void TileMapFillRect(short *dst, short value, u_short w, u_short h,
                            u_short stride);

#ifdef NAME_KR
/* DNG's copy is defined old-style: the id and the glyph base come in as ints
   and are narrowed here, and the base goes on to the writer as an int. */
extern void TileMapWriteRow(const u_char *src, short *dst, int base,
                            u_short count);

void DrawPersonaName(persona, dst, base)
    short  persona;
    short *dst;
    short  base;
#else
extern void TileMapWriteRow(const u_char *src, short *dst, u_short base,
                            u_short count);

void DrawPersonaName(short persona, short *dst, u_short base)
#endif
{
    TileMapFillRect(dst, 0, NAME_CELLS, 1, MAP_W);
    if (persona != 0) {
        TileMapWriteRow(g_persona_data[persona].name, dst, base, NAME_CELLS);
    }
}
