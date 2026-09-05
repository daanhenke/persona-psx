/* Persona 1 (JP) - which sides of a tile have walls, seen from a facing.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   DNG 0x80097154   ADV 0x800968FC   S2D 0x800875F4
 *
 * The low byte of a tile's flags holds two nibbles of four sides each, in the
 * order north, south, west, east; bits 8 and 9 are a further pair. A quarter
 * turn sends north to east, east to south, south to west and west to north,
 * which is what each of the masked shifts below does - three per facing,
 * because the four bits do not sit in cyclic order.
 *
 * Tile types wrap at 0x38, so the second bank of types shares the first
 * bank's walls.
 */
#include <types.h>

#define TILE_TYPES 0x38

extern const u_short g_tile_walls[];

u_short TileWallsFacing(short tile, short facing)
{
    short   t;
    u_short v;
    u_short r;

    tile = tile & 0xFF;
    t = tile;
    if (tile >= TILE_TYPES) {
        t = tile - TILE_TYPES;
    }
    switch (facing) {
    case 0:
        r = g_tile_walls[t];
        break;
    case 1:
        v = g_tile_walls[t];
        r = ((v & 0x11) << 3) | ((v & 0x22) << 1);
        r = r | ((v & 0xCC) >> 2);
        r = r | ((v & 0x100) << 1);
        r = r | ((v & 0x200) >> 1);
        break;
    case 2:
        v = g_tile_walls[t];
        r = ((v & 0x55) << 1) | ((v & 0xAA) >> 1);
        r = r | (v & 0x300);
        break;
    case 3:
        v = g_tile_walls[t];
        r = ((v & 0x33) << 2) | ((v & 0x88) >> 3);
        r = r | ((v & 0x44) >> 1);
        r = r | ((v & 0x100) << 1);
        r = r | ((v & 0x200) >> 1);
        break;
    }
    return r;
}
