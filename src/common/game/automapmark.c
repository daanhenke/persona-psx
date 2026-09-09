/* Persona 1 (JP) - marking an automap tile revealed.  ADV only.
 *   ADV 0x80096AB0
 *
 * Handed the first room of the map already resolved, because the scene keeps
 * that index alongside the room number - which is the difference from
 * MapTileSeen in automap.c, a unit of its own ahead of this one.
 */
#include <decomp/types.h>

#define MAP_ROW_BYTES 3
#define MAP_BYTES     72

/* Reached by hardcoded address rather than through a linker symbol. */
#define g_map_seen ((u_char *)0x801F2B38)

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
