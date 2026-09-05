/* Persona 1 (JP) - the gauge above the menu window.
 *
 * S2D copy; see src/p1-jp/common/gfx/gauge.c for the shared original.
 *   S2D @ 0x8007ECEC
 *
 * Three graphics, each twelve cells of one consecutive run of the sheet, laid
 * down right to left the way TileMapWriteRun12 does it - so the counter, the
 * destination and the first cell are all set before the loop, and the order of
 * those three statements is not free.
 *
 * Two rows go down unconditionally, and `mode` picks what goes beside them:
 * 0 draws a third run and labels both halves of the bar, 1 and 2 label only
 * the middle. The labels come from str_cell_run, which is 00 01 02 03, so the
 * base does all the work.
 */
#include <types.h>

#define g_tilemap0 ((short *)0x8010E180)
#define g_tilemap1 ((short *)0x8010F580)

#define MAP_W 40
#define RUN   12

extern const u_char str_cell_run[];
extern void TileMapWriteRow(const u_char *src, short *dst, int base,
                            u_short count);
extern void TileMapFillRect(short *dst, short cell, u_short w, u_short h,
                            u_short stride);

void DrawGauge(short mode)
{
    short *p;
    short cell;
    int i;
    int i2;
    int i3;

    i = 11;
    p = &g_tilemap0[8 * MAP_W + 14];
    cell = 0x36;
    for (; i >= 0; i--) {
        *p = cell;
        p--;
        cell--;
    }
    i2 = 11;
    p = &g_tilemap0[9 * MAP_W + 14];
    cell = 0x56;
    for (; i2 >= 0; i2--) {
        *p = cell;
        p--;
        cell--;
    }
    TileMapFillRect(&g_tilemap1[1 * MAP_W + 3], 0, 0xC, 1, MAP_W);
    TileMapWriteRow(str_cell_run, &g_tilemap1[1 * MAP_W + 4], 0x39E, 3);
    switch (mode) {
    case 0:
        i3 = 11;
        p = &g_tilemap0[3 * (3 * MAP_W) + 14];
        cell = 0x4A;
        /* The third run's test is also stored into the first run's counter.
           It is dead, and it is what puts the counters in the registers the
           original uses - without it the whole function drops to 97.7%. Do
           not tidy it away. */
        for (; (i = (i3 >= 0)) != 0; i3--) {
            *p = cell;
            p--;
            cell--;
        }
        TileMapWriteRow(str_cell_run, &g_tilemap1[1 * MAP_W + 3], 0x392, 6);
        TileMapWriteRow(str_cell_run, &g_tilemap1[1 * MAP_W + 9], 0x398, 6);
        break;
    case 1:
        TileMapWriteRow(str_cell_run, &g_tilemap1[1 * MAP_W + 8], 0x392, 6);
        break;
    case 2:
        TileMapWriteRow(str_cell_run, &g_tilemap1[1 * MAP_W + 8], 0x398, 6);
        break;
    }
}
