/* Persona 1 (JP) - the player's marker on the automap.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   DNG @ 0x8009645C   ADV @ 0x80095AE0   S2D @ 0x800868F0
 *
 * The map redraw calls DrawCompass and then this. The player's position is
 * turned into the map's current facing - the map can be rotated - and the
 * marker gets one of four animations picked by the two facings added together,
 * so the arrow points the right way whichever way the map is turned. The slot
 * flickers, which is what makes the marker blink.
 */
#include <types.h>

/* Where the rotated position lands; the map's own draw reads it back. */
extern short g_map_view_x;
extern short g_map_view_y;

/* Four slot animation definitions, 0x10 bytes each, one per facing. */
extern u_char g_map_marker_defs[];

#define MARKER_SLOT 0x28
#define MARKER_Z    0x10
#define DEF_SIZE    0x10

extern void RoomRotatePoint(short from, short x, short y, short to,
                            short *out_x, short *out_y);
extern void SlotInitTagged(void *def, u_char slot, int attr, short x, short y);
extern void SlotSetFlicker(u_char slot, u_char on);

void MapPlaceMarker(short map_dir, short player_dir, short x, short y)
{
    RoomRotatePoint(0, x, y, map_dir, &g_map_view_x, &g_map_view_y);
    SlotInitTagged(&g_map_marker_defs[((map_dir + player_dir) & 3) * DEF_SIZE],
                   MARKER_SLOT, MARKER_Z, 0, 0);
    SlotSetFlicker(MARKER_SLOT, 1);
}
