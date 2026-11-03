/* Initialize and draw the two preview objects from shared transform storage.
 * Slots 0/1 and 6/7 are the current/previous transforms for each object;
 * the intervening storage contains other state and is not accessed here.
 * Passing the vectors by value preserves their snapshots during object writes.
 */
#include <persona/casino/casino.h>

extern char        D_800A2BC8[];
extern CasinoXform D_800B0720[];
extern void        CasinoTexObj(CasinoModel*);
extern void        CasinoSetXform(CasinoXform*);
extern void        CasinoDrawObj(CasinoObj*);

static inline void DrawVectors(CasinoModel* m, SVECTOR r, VECTOR t, VECTOR s)
{
    m->obj->xform->rot.vx   = r.vx;
    m->obj->xform->rot.vy   = r.vy;
    m->obj->xform->rot.vz   = r.vz;
    m->obj->xform->trans.vx = t.vx;
    m->obj->xform->trans.vy = t.vy;
    m->obj->xform->trans.vz = t.vz;
    m->obj->xform->scale.vx = s.vx;
    m->obj->xform->scale.vy = s.vy;
    m->obj->xform->scale.vz = s.vz;
    CasinoTexObj(m);
    CasinoSetXform(m->obj->xform);
    CasinoDrawObj(m->obj);
}
void func_80093178(void)
{

    D_800B0720[0].rot.vx = D_800B0720[1].rot.vx = 2048;
    D_800B0720[0].trans.vx = D_800B0720[1].trans.vx = 24;
    D_800B0720[0].rot.vy = D_800B0720[1].rot.vy = 0;
    D_800B0720[0].rot.vz = D_800B0720[1].rot.vz = 0;
    D_800B0720[0].trans.vy = D_800B0720[1].trans.vy = -40;
    D_800B0720[0].trans.vz = D_800B0720[1].trans.vz = 512;
    D_800B0720[0].scale.vx = D_800B0720[1].scale.vx = 4096;
    D_800B0720[0].scale.vy = D_800B0720[1].scale.vy = 4096;
    D_800B0720[0].scale.vz = D_800B0720[1].scale.vz = 4096;
    D_800B0720[6].rot.vx = D_800B0720[7].rot.vx = 0;
    D_800B0720[6].rot.vy = D_800B0720[7].rot.vy = 0;
    D_800B0720[6].rot.vz = D_800B0720[7].rot.vz = 0;
    D_800B0720[6].trans.vx = D_800B0720[7].trans.vx = -24;
    D_800B0720[6].trans.vy = D_800B0720[7].trans.vy = -40;
    D_800B0720[6].trans.vz = D_800B0720[7].trans.vz = 512;
    D_800B0720[6].scale.vx = D_800B0720[7].scale.vx = 4096;
    D_800B0720[6].scale.vy = D_800B0720[7].scale.vy = 4096;
    D_800B0720[6].scale.vz = D_800B0720[7].scale.vz = 4096;
    DrawVectors((CasinoModel*)(D_800A2BC8 + 14816), D_800B0720[0].rot, D_800B0720[0].trans, D_800B0720[0].scale);
    DrawVectors((CasinoModel*)(D_800A2BC8 + 14836), D_800B0720[6].rot, D_800B0720[6].trans, D_800B0720[6].scale);
}
