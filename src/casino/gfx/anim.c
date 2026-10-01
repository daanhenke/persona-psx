/* Persona 1 (JP) - CASINO's 3D object animations.
 *   0x8006B4B8 CasinoStartAnim
 *   0x8006B57C CasinoAddAnim
 *   0x8006B8B0 CasinoStepAnims
 *
 * An object is turned, moved and scaled by a total spread over a number of
 * frames. While it runs the object is busy, and each frame its transform
 * takes another step and the object is drawn with it.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/casino/casino.h>

extern void CasinoSetXform(CasinoXform *x);
extern void CasinoDrawObj(CasinoObj *o);

void CasinoAddAnim(CasinoAnimArg a);

#ifdef NON_MATCHING
void CasinoStartAnim(CasinoObj *o, short frames, short rx, short ry, short rz, long tx, long ty, long tz, long sx,
                     long sy, long sz)
{
    volatile CasinoAnimArg a;

    a.d.rot.vx = rx;
    a.d.rot.vy = ry;
    a.d.rot.pad = 0;
    a.d.rot.vz = rz;
    a.d.trans.vx = tx;
    a.d.trans.vy = ty;
    a.d.trans.vz = tz;
    a.d.trans.pad = 0;
    a.d.scale.vx = sx;
    a.d.scale.vy = sy;
    a.d.scale.vz = sz;
    a.d.scale.pad = 0;
    a.o = o;
    a.frames = frames;
    CasinoAddAnim(*(CasinoAnimArg *)&a);
}
#else
/* 95.92%: every instruction matches but the image keeps `sw ra` right after
   the frame adjust, ahead of the parameter loads; sched2 here moves the
   loads above it (the VramClearRect residual). The volatile request struct
   is what keeps the stores in field order. */
INCLUDE_ASM("casino/nonmatchings/gfx/anim", CasinoStartAnim);
#endif

void CasinoAddAnim(CasinoAnimArg a)
{
    CasinoXform  cur;
    CasinoXform *d;
    CasinoObj   *o;
    short        n;

    n = g_casino_anims.n;
    cur = *a.o->xform;
    o = a.o;
    d = &g_casino_anims.delta[n];
    d->rot.vx = a.d.rot.vx / a.frames;
    d->rot.vy = a.d.rot.vy / a.frames;
    d->rot.vz = a.d.rot.vz / a.frames;
    d->trans.vx = a.d.trans.vx / a.frames;
    d->trans.vy = a.d.trans.vy / a.frames;
    d->trans.vz = a.d.trans.vz / a.frames;
    d->scale.vx = a.d.scale.vx / a.frames;
    d->scale.vy = a.d.scale.vy / a.frames;
    d->scale.vz = a.d.scale.vz / a.frames;
    g_casino_anims.obj[n] = o;
    g_casino_anims.obj[n]->busy = 1;
    g_casino_anims.left[n] = a.frames - 1;
    g_casino_anims.n++;
}

#ifdef NON_MATCHING
void CasinoStepAnims(void)
{
    CasinoObj   *o;
    CasinoXform *x;
    CasinoXform *d;
    int          i;
    int          k;

    for (i = 0; i < g_casino_anims.n; i++) {
        d = &g_casino_anims.delta[i];
        o = g_casino_anims.obj[i];
        x = o->xform;
        x->rot.vx += d->rot.vx;
        x->rot.vy += d->rot.vy;
        x->rot.vz += d->rot.vz;
        x->trans.vx += d->trans.vx;
        x->trans.vy += d->trans.vy;
        x->trans.vz += d->trans.vz;
        x->scale.vx += d->scale.vx;
        x->scale.vy += d->scale.vy;
        x->scale.vz += d->scale.vz;
        if (--g_casino_anims.left[i] == -1) {
            o->busy = 0;
            for (k = i; k < g_casino_anims.n; k++) {
                g_casino_anims.delta[k] = g_casino_anims.delta[k + 1];
                g_casino_anims.obj[k] = g_casino_anims.obj[k + 1];
                g_casino_anims.left[k] = g_casino_anims.left[k + 1];
            }
            g_casino_anims.n--;
            i--;
        }
        CasinoSetXform(x);
        CasinoDrawObj(o);
    }
}
#else
/* 94.16%: the image starts the removal loop's pointer givs from the outer
   loop's i*4 and i*40 registers (cse2 reuses them); here cse2 recomputes
   them in the preheader, which shifts the registers that follow. */
INCLUDE_ASM("casino/nonmatchings/gfx/anim", CasinoStepAnims);
#endif
