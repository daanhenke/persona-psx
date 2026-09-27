/* Persona 1 (JP) - the automap's cell layers out of a room's tile map.
 *
 * Compiled into three overlays rather than called across the boundary:
 *   DNG 0x80096BB4   ADV 0x8009635C   S2D 0x80087054
 *
 * A room is 24 by 24 tiles, one byte each, a tile type. The drawer first
 * clears a 42-wide scratch grid and the two 42-wide cell layers (walls and
 * floor), then copies the room into the grid turned the way the player holds
 * the map, blanking the tiles the automap has not revealed. Last it walks the
 * grid one cell further than the room each way and picks, per cell, a floor
 * cell from the tile's kind and a wall cell from the walls it shares with the
 * tiles above and to its left, both placed 4 rows and 7 columns in.
 */
#include <decomp/types.h>
#include <persona/common/automap.h>

#define ROOM_W 24
#define GRID_W 42
#define GRID_H 32
#define VIEW_W (ROOM_W + 1)
#define VIEW_Y 4
#define VIEW_X 7

/* The first cell of each layer: a blank wall and a blank floor. */
#define WALL_CELL0 0x3DC
#define FLOOR_BLANK 0x403

/* Floor kinds below this are one cell whatever the turn. */
#define FLOOR_PLAIN 13

/* Reached by hardcoded address; S2D's sit WORK_BIAS higher. */
#define g_map_grid  ((u_char (*)[GRID_W])(0x800EAEFB + WORK_BIAS))
#define g_map_walls ((short (*)[GRID_W])(0x800EF580 + WORK_BIAS))
#define g_map_floor ((short (*)[GRID_W])(0x800F0980 + WORK_BIAS))

extern short g_map_view_area;
extern short g_map_view_room;

extern const u_short g_map_floor_cells[];
/* Four cells a kind, one per turn, for the kinds from FLOOR_PLAIN on. */
extern const u_short g_map_turned_cells[];
/* Sixteen cells a set (vertical by horizontal wall, four each); set 4 is
   the plain one. */
extern const u_char  g_map_wall_cells[];

void MapDrawGrid(u_char *map, MapTile *tiles, short turn)
{
    int     x;
    int     y;
    int     v;
    u_short cur;
    u_char  up;
    u_char  left;
    u_int   kind;
    int     vert;
    int     horz;
    u_short cw;
    u_short uw;
    u_short lw;
    u_short cell;
    u_char (*grid)[GRID_W];

    /* The grid is held in a variable (spilled) for the turns and the walk;
       the clear addresses it by number. */
    grid = g_map_grid;
    for (y = 0; y < GRID_H; y++) {
        for (x = 0; x < GRID_W; x++) {
            grid[y][x]  = 0;
            g_map_walls[y][x] = WALL_CELL0;
            g_map_floor[y][x] = FLOOR_BLANK;
        }
    }

    switch (turn) {
    case 0:
        for (y = 0; y < ROOM_W; y++) {
            for (x = 0; x < ROOM_W; x++) {
                v = *(map + y * ROOM_W + x);
                if (!MapTileSeen(g_map_view_area, g_map_view_room, x, y)) {
                    v = 0;
                }
                grid[y][x] = v;
            }
        }
        break;
    case 1:
        for (y = 0; y < ROOM_W; y++) {
            for (x = 0; x < ROOM_W; x++) {
                v = *(map + (y + (ROOM_W - 1) * ROOM_W) - x * ROOM_W);
                if (!MapTileSeen(g_map_view_area, g_map_view_room, y,
                                 ROOM_W - 1 - x)) {
                    v = 0;
                }
                grid[y][x] = v;
            }
        }
        break;
    case 2:
        for (y = 0; y < ROOM_W; y++) {
            for (x = 0; x < ROOM_W; x++) {
                v = *(map + (ROOM_W - 1 - y) * ROOM_W - x + (ROOM_W - 1));
                if (!MapTileSeen(g_map_view_area, g_map_view_room,
                                 ROOM_W - 1 - x, ROOM_W - 1 - y)) {
                    v = 0;
                }
                grid[y][x] = v;
            }
        }
        break;
    case 3:
        for (y = 0; y < ROOM_W; y++) {
            for (x = 0; x < ROOM_W; x++) {
                v = *(map + (ROOM_W - 1 - y) + x * ROOM_W);
                if (!MapTileSeen(g_map_view_area, g_map_view_room,
                                 ROOM_W - 1 - y, x)) {
                    v = 0;
                }
                grid[y][x] = v;
            }
        }
        break;
    }

    for (y = 0; y < VIEW_W; y++) {
        for (x = 0; x < VIEW_W; x++) {
            v   = grid[y][x];
            cur = tiles[v].flags;
            v   = grid[y - 1][x];
            if (y != 0) {
                up = tiles[v].flags;
            } else {
                up = 0;
            }
            v = grid[y][x - 1];
            if (x != 0) {
                left = tiles[v].flags;
            } else {
                left = 0;
            }

            kind = cur >> 8;
            if (kind >= FLOOR_PLAIN) {
                int k = kind - FLOOR_PLAIN;
                v = (k / 4 * 4 + (k & 3)) * 4 + turn;
                cell = g_map_turned_cells[v];
            } else {
                cell = g_map_floor_cells[kind];
            }
            g_map_floor[y + VIEW_Y][x + VIEW_X] = cell;

            cw = TileWallsFacing(cur, turn);
            uw = TileWallsFacing(up, turn);
            lw = TileWallsFacing(left, turn);

            if ((uw & 2) || (cw & 1)) {
                vert = 1;
            } else {
                vert = 0;
                if (uw & 0x44) {
                    if (!(cw & 0x11)) {
                        vert = 2;
                    }
                }
            }
            if ((lw & 8) || (cw & 4)) {
                horz = 1;
            } else {
                horz = 0;
                if (lw & 0x11) {
                    if (!(cw & 0x44)) {
                        horz = 2;
                    }
                }
            }
            if ((uw & 0x20) || (cw & 0x10)) {
                vert = 3;
            }
            if ((lw & 0x80) || (cw & 0x40)) {
                horz = 3;
            }
            /* The wall set, in `cur` again: a variable of its own takes v1
               where the image keeps it in a0. */
            cur = (cw & 0x100) ? 5 : 0;
            if (cw & 0x200) {
                cur = 6;
            }
            if (cur == 0) {
                g_map_walls[y + VIEW_Y][x + VIEW_X] =
                    g_map_wall_cells[horz + (vert << 2) + 64] + WALL_CELL0;
            } else {
                /* An integer base, added after the row and before the
                   set: the order the image adds them in. */
                u_long cells = (u_long)g_map_wall_cells;
                u_char *p = (u_char *)(horz + ((vert << 2) + cells));
                g_map_walls[y + VIEW_Y][x + VIEW_X] = p[cur << 4] + WALL_CELL0;
            }
        }
    }
}
