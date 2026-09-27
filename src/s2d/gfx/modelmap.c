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

typedef struct {
    u_short n_vert;
    u_short n_normal;
    u_short n_prim;
    u_short pad;
} ModelCounts;

typedef struct {
    /* 0x00 */ u_long     *objs;
    /* 0x04 */ u_long      n_obj;
    /* 0x08 */ ModelCounts counts[16];
} Model;                                /* 0x88 */

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

extern Model g_models[];

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
