/* Persona 1 (JP) - registering a TMD.  S2D only.
 *   0x8008D8EC ModelMap
 *
 * Maps a TMD in place and records, in model slot `slot`, where its object
 * table starts, how many objects it has and each object's vertex, normal and
 * primitive counts.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/s2d/model.h>

/* One entry of a TMD's object table. */
typedef struct {
    u_long *vert_top;
    u_long  n_vert;
    u_long *normal_top;
    u_long  n_normal;
    u_long *prim_top;
    u_long  n_prim;
    long    scale;
} TmdObject;                            /* 0x1C */


void ModelMap(u_long *tmd, int slot)
{
    Model     *m;
    TmdObject *o;
    u_long     i;
    u_long     n;

    m = &g_models[slot];
    GsMapModelingData(tmd + 1);
    n = tmd[2];
    m->objs = tmd + 3;
    m->n_obj = n;
    o = (TmdObject *)(tmd + 3);
    for (i = 0; i < n; i++) {
        m->counts[i].n_vert = o->n_vert;
        m->counts[i].n_normal = o->n_normal;
        m->counts[i].n_prim = o->n_prim;
        o++;
    }
}
