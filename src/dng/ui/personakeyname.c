/* Persona 1 (JP) - a Persona's name by its key.  DNG only.
 *   0x8008A0F4 DrawPersonaKeyName
 *
 * DrawPersonaName's sibling for the fixed definitions: the row is cleared,
 * and a key of 0 leaves it blank. The status pages draw the header's Persona
 * with it. ADV has the same routine at 0x8007B6C0, still asm. Defined
 * old-style, like DNG's DrawPersonaName.
 */
#include <decomp/types.h>
#include <persona/common/persona.h>

#define MAP_W      40
#define NAME_CELLS 10

extern void TileMapWriteRow(const u_char *src, short *dst, int base,
                            u_short count);
extern void TileMapFillRect(short *dst, short value, u_short w, u_short h,
                            u_short stride);

void DrawPersonaKeyName(key, dst, base)
    short  key;
    short *dst;
    short  base;
{
    TileMapFillRect(dst, 0, NAME_CELLS, 1, MAP_W);
    if (key != 0) {
        TileMapWriteRow(g_persona_defs[key].name, dst, base, NAME_CELLS);
    }
}
