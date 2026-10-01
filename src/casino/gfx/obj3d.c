/* Persona 1 (JP) - CASINO's 3D objects: flat faces drawn as sprites.
 *   0x8006A480 CasinoObjSetOn
 *   0x8006A4D4 CasinoDrawObj
 *   0x8006A768 CasinoBuildObj
 *   0x8006A994 CasinoTexObj
 *   0x8006AA54 CasinoSetXform
 *
 * Each face of an object owns one sprite. Drawing puts the face through
 * the GTE with the current matrix and queues the four screen corners for
 * its sprite; a face that is off or turned away switches its sprite off.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/casino/casino.h>

extern void CasinoQueueQuad(short x0, short y0, short x1, short y1, short x2, short y2, short x3, short y3, int spr);
extern void CasinoQueueUV(short u, short v, short w, short h, u_short tpage, u_short clut, int spr);

/* The depth range a sorted face is mapped into. */
#define OBJ_FAR 0x400

void CasinoObjSetOn(CasinoObj *o, short first, short n, u_char on)
{
    int i;

    for (i = 0; i < n; i++) {
        o->face[i + first].on = on;
    }
}

void CasinoDrawObj(CasinoObj *o)
{
    long    unused[2];
    SVECTOR s[4];
    VECTOR  t[4];
    long    p;
    long    otz;
    long    flag;
    int     i;

    for (i = 0; i < o->n; i++) {
        if (!o->face[i].on
            || RotAverageNclip4(&o->face[i].v[0], &o->face[i].v[1], &o->face[i].v[2], &o->face[i].v[3],
                                (long *)&s[0], (long *)&s[1], (long *)&s[2], (long *)&s[3], &p, &otz, &flag) <= 0) {
            g_casino_sprites.on[i + o->first] = 0;
            continue;
        }
        g_casino_sprites.on[i + o->first] = 1;
        if (o->flat == 0) {
            RotTrans(&o->face[i].v[0], &t[0], &flag);
            RotTrans(&o->face[i].v[1], &t[1], &flag);
            RotTrans(&o->face[i].v[2], &t[2], &flag);
            RotTrans(&o->face[i].v[3], &t[3], &flag);
            s[0].vx = t[0].vx;
            s[0].vy = t[0].vy;
            s[1].vx = t[1].vx;
            s[1].vy = t[1].vy;
            s[2].vx = t[2].vx;
            s[2].vy = t[2].vy;
            s[3].vx = t[3].vx;
            s[3].vy = t[3].vy;
        }
        CasinoQueueQuad(s[0].vx, s[0].vy, s[1].vx, s[1].vy, s[2].vx, s[2].vy, s[3].vx, s[3].vy, o->first + i);
        if (o->zsort == 1) {
            if (otz < OBJ_FAR) {
                g_casino_sprites.z[i + o->first] = OBJ_FAR - otz;
            } else {
                g_casino_sprites.z[i + o->first] = 0;
            }
        } else {
            g_casino_sprites.z[i + o->first] = o->face[i].z;
        }
    }
}

/* Lays each face out from its definition: axis 1 runs left to right,
   axis 0 is the same face mirrored. */
void CasinoBuildObj(CasinoModel *m)
{
    CasinoFaceDef *defs;
    CasinoFace    *f;
    short         *idx;
    SVECTOR        o;
    short          w;
    short          h;
    int            i;

    defs = m->def;
    f = m->obj->face;
    idx = m->def_idx;
    for (i = 0; i < m->obj->n; i++) {
        o = defs[idx[i]].origin;
        w = defs[idx[i]].w;
        h = defs[idx[i]].h;
        f[i].z = defs[idx[i]].z;
        f[i].on = defs[idx[i]].on;
        if (defs[idx[i]].axis == 1) {
            f[i].v[0].vx = o.vx;
            f[i].v[0].vy = o.vy;
            f[i].v[0].vz = o.vz;
            f[i].v[1].vx = o.vx + w;
            f[i].v[1].vy = o.vy;
            f[i].v[1].vz = o.vz;
            f[i].v[2].vx = o.vx;
            f[i].v[2].vy = o.vy + h;
            f[i].v[2].vz = o.vz;
            f[i].v[3].vx = o.vx + w;
            f[i].v[3].vy = o.vy + h;
            f[i].v[3].vz = o.vz;
        } else if (defs[idx[i]].axis == 0) {
            f[i].v[0].vx = o.vx + w;
            f[i].v[0].vy = o.vy;
            f[i].v[0].vz = o.vz;
            f[i].v[1].vx = o.vx;
            f[i].v[1].vy = o.vy;
            f[i].v[1].vz = o.vz;
            f[i].v[2].vx = o.vx + w;
            f[i].v[2].vy = o.vy + h;
            f[i].v[2].vz = o.vz;
            f[i].v[3].vx = o.vx;
            f[i].v[3].vy = o.vy + h;
            f[i].v[3].vz = o.vz;
        }
    }
}

void CasinoTexObj(CasinoModel *m)
{
    u_long     tex;
    CasinoTex *t;
    short     *idx;
    short      first;
    short      n;
    int        i;
    int        k;
    u_short    c;

    n = m->obj->n;
    first = m->obj->first;
    tex = (u_long)m->tex;
    idx = m->tex_idx;
    for (i = 0; i < n; i++) {
        t = (CasinoTex *)(idx[i] * sizeof(CasinoTex) + tex);
        k = i + first;
        c = t->clut;
        CasinoQueueUV(t->u, t->v, t->w, t->h, t->tpage, c, k);
    }
}

void CasinoSetXform(CasinoXform *x)
{
    MATRIX m;

    RotMatrix(&x->rot, &m);
    TransMatrix(&m, &x->trans);
    ScaleMatrix(&m, &x->scale);
    SetTransMatrix(&m);
    SetRotMatrix(&m);
}
