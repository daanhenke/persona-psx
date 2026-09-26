/* Persona 1 (JP) - the automap view out of the map pack.
 *
 * Compiled into three overlays rather than called across the boundary:
 *                  DNG         ADV         S2D
 *   MapDrawTurned  0x80096B4C  0x800962F4  0x80086FEC
 *
 * The pack read to 0x801DD000 (see mapname.c) starts with the offset of a
 * byte table and of the tile data; the byte for the current area, added to
 * the map's number, picks the map's entry from the table at +8. Both go to
 * the drawer with the turn the player has the map at.
 */
#include <decomp/types.h>

#define MAP_PACK     0x801DD000
#define MAP_BASES    ((u_char *)0x801DD004)
#define MAP_TILES    (*(u_long *)0x801DD004)
#define MAP_ENTRIES  ((u_long *)0x801DD008)

extern void MapDrawGrid(u_char *map, u_char *tiles, short turn);

void MapDrawTurned(short map, short turn)
{
    u_long *pack;

    pack = (u_long *)MAP_PACK;
    MapDrawGrid((u_char *)pack + MAP_ENTRIES[map + MAP_BASES[pack[0]]],
                (u_char *)pack + MAP_TILES, turn);
}
