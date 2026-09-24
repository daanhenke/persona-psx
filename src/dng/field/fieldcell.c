/* Persona 1 (JP) - the minimap window's cells, and restarting the sound.
 * DNG only.
 *   0x8006E61C FieldRebuildMap
 *   0x8006E988 FieldSetCell
 *   0x8006EA5C FieldResetSound
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libsnd.h>
#include <persona/common/automap.h>
#include <persona/dng/field.h>

/* The minimap shows an 11 by 11 window that wraps round the floor, one
   halfword per cell; tile (x, y) always lands in cell (x % 11, y % 11). */
#define WINDOW 11

/* The maps whose automap rooms are not simply their floors. */
#define MAP_SHARED 0x24 /* every floor draws on area 0 */
#define MAP_SPLIT  10   /* floors from 3 on are rooms 0.. of the same area */
#define MAP_SINGLE 0x11 /* all floors share room 0 */

/* Works out which automap room the party's floor draws on, marks the
   party's tile seen, and redraws every cell of the minimap window around
   it - blank off the floor, and blank where the tile is unseen when the map
   shows seen tiles only. */
/* 92.1%: the mark and the case tree are the image's; in the window loop,
   loop.c hoists all of (y % 11 + 11) % 11 out of the column loop, where the
   image keeps the final division in it and hoists only y % 11 + 11 and its
   sign. */
#ifdef NON_MATCHING
void FieldRebuildMap(void)
{
    u_int    row, col;
    int      x, y;
    u_short *cell;

    FieldSetCell(g_dng->pos[POS_X], g_dng->pos[POS_Y]);
    if (g_dng->map == MAP_SHARED) {
        g_dng->area = 0;
        g_dng->room = g_dng->floor;
    } else if (g_dng->map == MAP_SPLIT && g_dng->floor >= 3) {
        g_dng->area = g_dng->map;
        g_dng->room = g_dng->floor - 3;
    } else if (g_dng->map == MAP_SINGLE) {
        g_dng->room = 0;
        g_dng->area = g_dng->map;
    } else {
        g_dng->area = g_dng->map;
        g_dng->room = g_dng->floor;
    }
    {
        u_char *seen = g_map_seen;
        u_short area = g_dng->area;
        u_short room = g_dng->room;
        u_char  px = g_dng->pos[POS_X];
        u_char  py = g_dng->pos[POS_Y];

        seen[py * MAP_ROW_BYTES + (g_map_base[area] + room) * MAP_BYTES + (px >> 3)] |=
            0x80 >> (px & 7);
    }

    y = g_dng->pos[POS_Y] - 5;
    for (row = 0; row < WINDOW; y++, row++) {
        x = g_dng->pos[POS_X] - 5;
        for (col = 0; col < WINDOW; col++, x++) {
            cell = &((u_short (*)[WINDOW])(PACK_BASE + 8 + *g_pack_cell_tab))
                [(y % WINDOW + WINDOW) % WINDOW][(x % WINDOW + WINDOW) % WINDOW];
            if (y >= 0 && x >= 0 && y < FLOOR_W && x < FLOOR_W &&
                (!g_dng->map_seen_only ||
                 (g_map_seen[(g_map_base[g_dng->area] + g_dng->room) * MAP_BYTES +
                             y * MAP_ROW_BYTES + x / 8] &
                  (0x80 >> (x % 8))))) {
                *cell = g_tile_defs[g_floor_grid[y][x]].icon;
            } else {
                *cell = 0;
            }
        }
    }
}
#else
INCLUDE_ASM("dng/nonmatchings/field/fieldcell", FieldRebuildMap);
#endif

/* Copies tile (x, y)'s minimap icon into its window cell. */
void FieldSetCell(int x, int y)
{
    u_short *cell;

    cell = &((u_short (*)[WINDOW])(PACK_BASE + 8 + *g_pack_cell_tab))[y % WINDOW][x % WINDOW];
    *cell = g_tile_defs[g_floor_grid[y][x]].icon;
}

/* Restarts libsnd and forgets every open sequence. */
void FieldResetSound(void)
{
    int i;

    SsEnd();
    SsQuit();
    SsInit();
    for (i = 0; i < SEQ_HANDLES; i++) {
        g_seq_handles[i] = -1;
    }
}
