/* Persona 1 (JP) - scrolling the view window a tile.  DNG only.
 *   0x80069A7C FieldLoadColumn
 *   0x80069EB4 FieldLoadRow
 *
 * The minimap and the scene both hold an 11 by 11 window round the party
 * that wraps round the floor. A step brings a new column or row into view:
 * its minimap cells are redrawn and its tiles' models placed into the scene
 * objects of the cells they land on.
 *
 * Both return int with nothing returned: v0 live at the exit is what keeps
 * the image's delay slot after the first bounds test empty.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/automap.h>
#include <persona/dng/field.h>

#define WINDOW 11

/* The minimap window's cells, a halfword each. */
#define CELLS ((u_short (*)[WINDOW])(PACK_BASE + 8 + *g_pack_cell_tab))

/* Whether tile (x, y) of the party's room is marked on the automap. */
#define SEEN(x, y)                                                                         \
    (g_map_seen[(y) * MAP_ROW_BYTES + (g_map_base[g_dng->area] + g_dng->room) * MAP_BYTES + \
                (x) / 8] &                                                                 \
     (0x80 >> ((x) % 8)))

#define ICON(x, y) (g_tile_defs[g_floor_grid[y][x]].icon)

/* The column five tiles ahead of the party along x, as a step along x
   brings it into view. */
int FieldLoadColumn(void)
{
    int      i, k;
    int      x, y;
    int      cx, cy; /* the minimap's pass: its own pair keeps them out of s-registers */
    u_short *cell;
    u_int    m;
    int      obj;

    cx = g_dng->pos[POS_X] + g_dir_step[g_dng->walk_dir] * 5;
    cy = g_dng->pos[POS_Y] - 5;
    for (i = 0; i < WINDOW; cy++, i++) {
        cell = &CELLS[(cy % WINDOW + WINDOW) % WINDOW][(cx % WINDOW + WINDOW) % WINDOW];
        if (cy >= 0 && cx >= 0 && cy < FLOOR_W && cx < FLOOR_W) {
            if (g_dng->map_seen_only) {
                if (SEEN(cx, cy)) {
                    *cell = ICON(cx, cy);
                } else {
                    *cell = 0;
                }
            } else {
                *cell = ICON(cx, cy);
            }
        } else {
            *cell = 0;
        }
    }

    x = g_dng->pos[POS_X] + g_dir_step[g_dng->walk_dir] * 5;
    if (x >= 0) {
        if (x >= FLOOR_W) {
            return;
        }
        y = g_dng->pos[POS_Y] - 5;
        for (i = 0; i < WINDOW; y++, i++) {
            if ((u_int)y < FLOOR_W && g_floor_grid[y][x] != 0) {
                for (k = 0; k < 8; k++) {
                    m = g_tile_defs[g_floor_grid[y][x]].models[k];
                    if (m != 0) {
                        obj = ((y % WINDOW) * WINDOW + x % WINDOW) * 8 + k;
                        FieldPlaceObject(obj, m, x, y);
                        D_8009CD50[obj] = 0;
                        D_8009DC70[obj] = 0;
                        D_8009EB90[obj] = 0;
                    }
                }
            }
        }
    }
}

/* The row five tiles ahead of the party along y, as a step along y brings
   it into view (y runs against the step). */
int FieldLoadRow(void)
{
    int      i, k;
    int      x, y, t;
    int      cx, cy;
    u_short *cell;
    u_int    m;
    int      obj;

    cy = g_dng->pos[POS_Y] - g_dir_step[g_dng->walk_dir] * 5;
    cx = g_dng->pos[POS_X] - 5;
    for (i = 0; i < WINDOW; i++, cx++) {
        cell = &CELLS[(cy % WINDOW + WINDOW) % WINDOW][(cx % WINDOW + WINDOW) % WINDOW];
        if (cy >= 0 && cx >= 0 && cy < FLOOR_W && cx < FLOOR_W) {
            if (g_dng->map_seen_only) {
                if (SEEN(cx, cy)) {
                    *cell = ICON(cx, cy);
                } else {
                    *cell = 0;
                }
            } else {
                *cell = ICON(cx, cy);
            }
        } else {
            *cell = 0;
        }
    }

    y = g_dng->pos[POS_Y] - g_dir_step[g_dng->walk_dir] * 5;
    if (y >= 0) {
        if (y >= FLOOR_W) {
            return;
        }
        x = g_dng->pos[POS_X] - 5;
        for (i = 0; i < WINDOW; i++, x++) {
            if (x >= 0 && y < FLOOR_W && (t = g_floor_grid[y][x]) != 0) {
                for (k = 0; k < 8; k++) {
                    m = g_tile_defs[t].models[k];
                    if (m != 0) {
                        obj = ((y % WINDOW) * WINDOW + x % WINDOW) * 8 + k;
                        FieldPlaceObject(obj, m, x, y);
                        D_8009CD50[obj] = 0;
                        D_8009DC70[obj] = 0;
                        D_8009EB90[obj] = 0;
                    }
                }
            }
        }
    }
}
