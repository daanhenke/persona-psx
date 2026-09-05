/* Persona 1 (JP) - the config screen's labels.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   DNG @ 0x8008FE44   ADV @ 0x8008BC1C   S2D @ 0x80080354
 *
 * The screen's redraw sets the window up and then calls this to fill it in: a
 * title on row 1, five option names down column 0 on every other row from 3,
 * and two more at column 11 beside the first two of them. Every string is
 * 0xFF-terminated, which is why the two columns do not collide even though
 * both are written sixteen cells wide.
 *
 * It is the settings-items list, headed SETTING ITEMS:
 *
 *   0  AUTO MAP            FREE / FIXED
 *   1  SOUND               MONO / STEREO
 *   2  CONTROLLER SETTINGS
 *   3  WINDOW SETTINGS
 *   4  BATTLE SETTINGS
 *
 * The two value labels are the right-column strings, and the last three rows
 * open pages of their own rather than holding a value - which is why only two
 * of them are drawn. src/p1-jp/common/ui/config.c arms and marks them.
 *
 * S2D builds this same source against a layer 0x20000 higher, which is what
 * WORK_BIAS says.
 */
#include <types.h>

#define g_tilemap1 ((short *)(0x800EF580 + WORK_BIAS))

#define MAP_W 40

extern const u_char str_config_title[];
extern const u_char *g_config_labels_left[];
extern const u_char *g_config_labels_right[];

extern void TileMapWriteRow(const u_char *src, short *dst, u_short base,
                            u_short count);

void ConfigDrawLabels(void)
{
    u_char i;

    TileMapWriteRow(str_config_title, &g_tilemap1[MAP_W + 1], 0, 8);
    for (i = 0; i < 5; i++) {
        TileMapWriteRow(g_config_labels_left[i],
                        &g_tilemap1[(i * 2 + 3) * MAP_W], 0, 16);
    }
    for (i = 0; i < 2; i++) {
        TileMapWriteRow(g_config_labels_right[i],
                        &g_tilemap1[(i * 2 + 3) * MAP_W + 11], 0, 16);
    }
}
