/* Persona 1 (JP) - blanking a run of text sprite cells.
 *   DNG 0x8009300C   ADV 0x8008EF6C   S2D 0x80083520
 *
 * The row writer that fills them is a unit of its own further along in every
 * overlay, in cellsrow.c.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

#define GLYPH_W    8
#define GLYPH_H    12
#define ATLAS_COLS 31
#define CELL_FLAG  0x3C0
#define CELL_TPAGE 0x1A0

/* Blanks a run of cells. 0xFF in u and v is the same "no cell here" code the
   row writers stop on, so a bar or a string that shrinks erases its tail. */
void CellsClear(GsCELL *dst, u_char count)
{
    while (count != 0) {
        dst->u = 0xFF;
        dst->v = 0xFF;
        dst++;
        count--;
    }
}
