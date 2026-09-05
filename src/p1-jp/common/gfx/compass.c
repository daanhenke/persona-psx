/* Persona 1 (JP) - the compass rose on the map screen.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   DNG 0x80096374   ADV 0x800959F8   S2D 0x80086808
 *
 * Four markers around a centre at (0xA8, 0x90). Each takes the cel list for
 * its own compass point less the current facing, so the whole rose turns with
 * the room rather than the markers being redrawn.
 */
#include <types.h>

#define CEL_SIZE   16
#define MARK_ATTR  0x24
#define MARK_CX    0xA8
#define MARK_CY    0x90

extern u_char g_compass_cels[];

extern void SlotInitTagged(void *def, u_char slot, int attr, short x, short y);

void DrawCompass(short facing)
{
    SlotInitTagged(&g_compass_cels[((0 - facing) & 3) * CEL_SIZE], 0x24,
                   MARK_ATTR, MARK_CX, 0x3A);
    SlotInitTagged(&g_compass_cels[((2 - facing) & 3) * CEL_SIZE], 0x25,
                   MARK_ATTR, MARK_CX, 0xDA);
    SlotInitTagged(&g_compass_cels[((3 - facing) & 3) * CEL_SIZE], 0x26,
                   MARK_ATTR, 0x28, MARK_CY);
    SlotInitTagged(&g_compass_cels[((1 - facing) & 3) * CEL_SIZE], 0x27,
                   MARK_ATTR, 0x128, MARK_CY);
}
