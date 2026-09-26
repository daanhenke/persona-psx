/* Persona 1 (JP) - a Persona's name in a menu row.
 *
 * Compiled into three overlays rather than called across the boundary:
 *                        DNG         ADV         S2D
 *   DrawPersonaKeyName   0x8008A0F4  0x8007B6C0  0x8007A540
 *   DrawPersonaName      0x8008A18C  0x8007B754  0x8007A5D4
 *
 * The row is cleared first, so an id of 0 leaves it blank rather than writing
 * anything - which is how an empty stock slot is drawn. DrawPersonaName takes
 * the name from the stock's Persona table in main RAM, DrawPersonaKeyName from
 * the fixed definitions; the status pages draw the header's Persona with the
 * latter. Both are ten cells.
 */
#include <decomp/types.h>
#include <persona/common/persona.h>

#define MAP_W 40
#define NAME_CELLS 10

extern void TileMapFillRect(short *dst, short value, u_short w, u_short h,
                            u_short stride);

#ifdef NAME_KR
/* DNG's copies are defined old-style: the id and the glyph base come in as
   ints and are narrowed here, and the base goes on to the writer as an int. */
extern void TileMapWriteRow(const u_char *src, short *dst, int base,
                            u_short count);

void DrawPersonaKeyName(key, dst, base)
    short  key;
    short *dst;
    short  base;
#else
extern void TileMapWriteRow(const u_char *src, short *dst, u_short base,
                            u_short count);

void DrawPersonaKeyName(short key, short *dst, u_short base)
#endif
{
    TileMapFillRect(dst, 0, NAME_CELLS, 1, MAP_W);
    if (key != 0) {
        TileMapWriteRow(g_persona_defs[key].name, dst, base, NAME_CELLS);
    }
}

#ifdef NAME_KR
void DrawPersonaName(persona, dst, base)
    short  persona;
    short *dst;
    short  base;
#else
void DrawPersonaName(short persona, short *dst, u_short base)
#endif
{
    TileMapFillRect(dst, 0, NAME_CELLS, 1, MAP_W);
    if (persona != 0) {
        TileMapWriteRow(g_persona_data[persona].name, dst, base, NAME_CELLS);
    }
}
