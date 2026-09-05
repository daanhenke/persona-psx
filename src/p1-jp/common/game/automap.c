/* Persona 1 (JP) - which tiles the automap has revealed.
 *
 * MapTileSeen is compiled into three overlays rather than called across the
 * boundary:
 *   DNG 0x80097290   ADV 0x80096A38   S2D 0x80087730
 * MapMarkTile (0x80096AB0) is ADV's alone.
 *
 * Every room owns 72 bytes: a 24 by 24 bit grid, three bytes to a row, with
 * the most significant bit of a byte at the lower x. Rooms are numbered
 * globally, and g_map_base turns a map id into the index of its first room -
 * MapMarkTile is handed that index already resolved, because the scene keeps
 * it alongside the room number.
 */
#include <types.h>

#define MAP_ROW_BYTES 3
#define MAP_BYTES     72

/* Reached by hardcoded address rather than through a linker symbol. */
#define g_map_seen ((u_char *)0x801F2B38)

extern const u_char g_map_base[];

u_char MapTileSeen(short map, short room, short x, short y)
{
    int n;
    int mask;
    int i;

    n = g_map_base[map] + room;
    mask = 0x80 >> (x & 7);
    i = n * MAP_BYTES + y * MAP_ROW_BYTES + x / 8;
    return g_map_seen[i] & mask;
}

void MapMarkTile(short base, short room, short x, short y)
{
    int n;
    int mask;
    int i;

    n = base + room;
    mask = 0x80 >> (x & 7);
    i = n * MAP_BYTES + y * MAP_ROW_BYTES + x / 8;
    g_map_seen[i] = mask | g_map_seen[i];
}
