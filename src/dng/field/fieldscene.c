/* Persona 1 (JP) - filling the scene with the floor's objects.  DNG only.
 *   0x8006F510 FieldBuildScene
 *   0x8006F7C8 FieldPlaceObject
 *
 * The scene holds eight objects for each tile of the 11 by 11 window round
 * the party; a tile's definition names up to eight models to put there.
 */
#include <decomp/types.h>
#include <memory.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/dng/field.h>

#define WINDOW 11

/* The pack index's pristine copy, and a save flag the field clears when it
   is built afresh. */
#define PACK_INDEX_COPY ((u_char *)0x801CA000)
#define FIELD_SAVE_FLAGS (*(u_char *)0x801F29EF)

/* Maps the floor's models, unless `reload`, first restoring the pack index,
   then places the objects of every tile of the window. One counter serves
   the mapping loop and the rows; the models are indexed from the second
   entry, and the row's modulo is written inline - loop.c, short of
   registers here, hoists only its division. */
void FieldBuildScene(int reload)
{
    u_int i, col, k;
    int   x, y, base, t;
    u_int m;

    if (!reload) {
        bcopy(PACK_INDEX_COPY, PACK_INDEX, 0x3000);
        FIELD_SAVE_FLAGS &= ~0x10;
    }
    for (i = 0; i < PACK_INDEX[g_pack_sel + 1]; i++) {
        (g_scene->models + 1)[i].tmd = FieldMapTmd((u_long *)(PACK_BASE + g_pack_tmd_tab[i]));
    }
    y = g_dng->pos[POS_Y] - 5;
    for (i = 0; i < WINDOW; y++, i++) {
        if ((u_int)y >= FLOOR_W) {
            continue;
        }
        x = g_dng->pos[POS_X] - 5;
        for (col = 0; col < WINDOW; col++, x++) {
            if ((u_int)x < FLOOR_W && (t = g_floor_grid[y][x]) != 0) {
                base = ((y % WINDOW) * WINDOW + x % WINDOW) * 8;
                for (k = 0; k < 8; k++) {
                    m = g_tile_defs[t].models[k];
                    if (m != 0) {
                        FieldPlaceObject(base + k, m, x, y);
                        D_8009CD50[base + k] = 0;
                        D_8009DC70[base + k] = 0;
                        D_8009EB90[base + k] = 0;
                    }
                }
            }
        }
    }
}

/* Puts model `model` in scene object `obj` on tile (x, y): its TMD (the
   first when the definition names none), its offset from the tile's
   corner and its rotation, given in degrees. */
void FieldPlaceObject(int obj, int model, int x, int y)
{
    ModelDef *d;
    int       tmd;

    tmd = 1;
    if (g_model_defs[model].tmd != 0) {
        tmd = g_model_defs[model].tmd;
    }
    GsLinkObject4((u_long)g_scene->models[tmd].tmd, &g_scene->objs[obj], 0);
    GsInitCoordinate2(WORLD, &g_scene->coords[obj]);
    d = &g_model_defs[model];
    g_scene->rots[obj].vx = (d->rx << 12) / 360;
    g_scene->rots[obj].vy = (d->ry << 12) / 360;
    g_scene->rots[obj].vz = (d->rz << 12) / 360;
    g_scene->objs[obj].coord2 = &g_scene->coords[obj];
    g_scene->objs[obj].attribute = d->attr & ~0xE00;
    g_scene->coords[obj].coord.t[0] = x * STEP_LEN + d->x;
    g_scene->coords[obj].coord.t[1] = d->y;
    g_scene->coords[obj].coord.t[2] = -y * STEP_LEN + d->z;
    CoordSetRot(&g_scene->rots[obj], &g_scene->coords[obj]);
}
