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

/* Keep the aggregate initializer: individual stores move the saved ra. */
void CasinoStartAnim(CasinoObj *o, short frames, short rx, short ry, short rz, long tx, long ty, long tz, long sx,
                     long sy, long sz)
{
    CasinoAnimArg a = {
        { { rx, ry, rz, 0 }, { tx, ty, tz, 0 }, { sx, sy, sz, 0 } },
        o,
        frames
    };

    CasinoAddAnim(a);
}

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

void CasinoStepAnims(void)
{
    CasinoObj   *o;
    CasinoXform *x;
    CasinoXform *d;
    int          i;
    int          k;

    for (i = 0; i < g_casino_anims.n; i++) {
        k = i;
        o = g_casino_anims.obj[i];
        d = &g_casino_anims.delta[k];
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
