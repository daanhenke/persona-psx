/* ADV builds DNG's unit (src/dng/ui/mapstep.c).
 *
 * ADV's MapScreenStep has the compass (common/gfx/compass.c) and the
 * player's marker (common/gfx/mapmarker.c) written into it where DNG calls
 * them: the same bodies, expanded inline.
 */
#include <decomp/types.h>

extern u_char g_compass_cels[];
extern u_char g_map_marker_defs[];
extern short  g_map_view_x;
extern short  g_map_view_y;

extern void SlotInitTagged(void *def, u_char slot, int attr, short x, short y);
extern void SlotSetFlicker(u_char slot, u_char on);
extern void RoomRotatePoint(short from, short x, short y, short to,
                            short *out_x, short *out_y);

static inline void DrawCompassInline(short facing)
{
    SlotInitTagged(&g_compass_cels[((0 - facing) & 3) * 16], 0x24, 0x24, 0xA8,
                   0x3A);
    SlotInitTagged(&g_compass_cels[((2 - facing) & 3) * 16], 0x25, 0x24, 0xA8,
                   0xDA);
    SlotInitTagged(&g_compass_cels[((3 - facing) & 3) * 16], 0x26, 0x24, 0x28,
                   0x90);
    SlotInitTagged(&g_compass_cels[((1 - facing) & 3) * 16], 0x27, 0x24, 0x128,
                   0x90);
}

static inline void MapPlaceMarkerInline(short map_dir, short player_dir,
                                        short x, short y, int unused)
{
    RoomRotatePoint(0, x, y, map_dir, &g_map_view_x, &g_map_view_y);
    SlotInitTagged(&g_map_marker_defs[((map_dir + player_dir) & 3) * 0x10],
                   0x28, 0x10, 0, 0);
    SlotSetFlicker(0x28, 1);
}

#define MAPSTEP_ADV
#define DrawCompass    DrawCompassInline
#define MapPlaceMarker MapPlaceMarkerInline
#include <persona/adv/dngport.h>
#include "../../dng/ui/mapstep.c"
