/* Persona 1 (JP) - graphics laid out as one run of the tile sheet.
 *
 * Compiled into more than one overlay rather than called across the boundary:
 *                       DNG         ADV         S2D
 *   TileMapWriteRun12   0x80085AA4  0x80076F28  0x80075EE0
 *   TileMapWriteRun10   0x80085ACC  0x80076F50
 *
 * A graphic wider than one tile is drawn by giving it consecutive ids on the
 * sheet, so writing it is a countdown rather than a table lookup. Both runs
 * finish on 0x56 and differ only in where they start, which is what makes them
 * two rows of the same picture. DrawGauge lays down the same kind of run.
 *
 * The counter is set before the destination and the destination before the
 * first cell; the three statements do not commute here.
 */
#include <types.h>

void TileMapWriteRun12(short *dst)
{
    short *p;
    short  cell;
    int    i;

    i = 11;
    p = &dst[11];
    cell = 0x56;
    for (; i >= 0; i--) {
        *p = cell;
        p--;
        cell--;
    }
}

void TileMapWriteRun10(short *dst)
{
    short *p;
    short  cell;
    int    i;

    i = 9;
    p = &dst[9];
    cell = 0x56;
    for (; i >= 0; i--) {
        *p = cell;
        p--;
        cell--;
    }
}
