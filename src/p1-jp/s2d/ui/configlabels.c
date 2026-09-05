/* Persona 1 (JP) - the config screen's labels.
 *
 * S2D copy; see src/p1-jp/common/ui/configlabels.c for the shared original.
 *   S2D @ 0x80080354
 *
 * The screen's redraw sets the window up and then calls this to fill it in: a
 * title on row 1, five option names down column 0 on every other row from 3,
 * and two more at column 11 beside the first two of them. Every string is
 * 0xFF-terminated, which is why the two columns do not collide even though
 * both are written sixteen cells wide.
 *
 * The text is Japanese and outside the range tools/glyphs.py decodes, so the
 * names say where the strings go rather than what they read.
 *
 * The layer sits 0x20000 above the DNG and ADV copies.
 */
#include <types.h>

#define g_tilemap1 ((short *)0x8010F580)

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
