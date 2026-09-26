/* Persona 1 (JP) - the moon on the field screen.  ADV only.
 *   0x80088B8C MoonSetCells   0x80088C68 MoonNone
 *
 * The moon's picture and the name of its phase are cells in the overlay's
 * own data. This points the picture cell at the phase's frame on the moon
 * sheet - eight to a row, 24 pixels apart - and writes the phase's four-glyph
 * name. The new moon (phase 8) has no picture: its cell is sent off the sheet.
 *
 * ovl_adv_entry calls it on the way in and AdvRoomRebuild after a cutscene.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

#define MOON_NEW    8
#define NAME_GLYPHS 4

/* The phase byte, reached by hardcoded address. */
#define MOON_AT ((u_char *)0x801F2B30)

extern GsCELL g_moon_cell;
extern GsCELL g_moon_name_cells[NAME_GLYPHS];
extern u_char g_moon_frames[];              /* by phase: column | row << 3 */
extern u_char g_moon_names[][NAME_GLYPHS];  /* by phase                    */

extern void CellsWriteRow(GsCELL *dst, const u_char *src, u_char page,
                          u_short count);
extern void CellsClear(GsCELL *dst, u_char count);

void MoonSetCells(void)
{
    u_char *moon;
    u_char  phase;
    u_char  f;

    moon = MOON_AT;
    phase = *moon & 0xF;
    if (phase == MOON_NEW) {
        g_moon_cell.u = 0xFF;
        g_moon_cell.v = 0xFF;
    } else {
        f = g_moon_frames[phase];
        g_moon_cell.u = (f & 7) * 24 + 0x28;
        g_moon_cell.v = (f >> 3) * 24 + 0x60;
    }
    CellsClear(g_moon_name_cells, NAME_GLYPHS);
    CellsWriteRow(g_moon_name_cells, g_moon_names[*moon & 0xF], 0, NAME_GLYPHS);
}

void MoonNone(void)
{
}
