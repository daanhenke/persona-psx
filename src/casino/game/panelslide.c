/* Persona 1 (JP) - slide three panels and their associated 3D object. */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/casino/casino.h>
extern CasinoLayoutDef D_800A1764;
extern CasinoLayoutDef D_800A17B8;
extern CasinoLayoutDef D_800A1620;
extern CasinoObj D_800A16F8;
/* The original caller promotes the signed distance before negating it. */
extern void CasinoTween(CasinoLayout *, int, short, short, short, short);
extern void CasinoStartAnim(CasinoObj *, short, short, short, short, long, long, long, long, long, long);
void func_80087F48(u_char active)
{
    short distance;
    do {
        distance = 320;
        if (active == 0) distance = -320;
    } while (0);
    CasinoTween(D_800A1764.l, -distance, 0, 0, 0, 32);
    CasinoTween(D_800A17B8.l, distance, 0, 0, 0, 32);
    CasinoTween(D_800A1620.l, -distance, 0, 0, 0, 32);
    CasinoStartAnim(&D_800A16F8, 32, 0, 0, 0, distance, 0, 0, 0, 0, 0);
}
