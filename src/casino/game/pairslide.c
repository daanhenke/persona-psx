/* Persona 1 (JP) - slide either pair of objects in opposite directions. */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/casino/casino.h>
extern CasinoObj D_8009C294[2];
extern CasinoObj D_8009C2B4[2];
extern void CasinoStartAnim();
void func_8007FBD8(int side)
{
    CasinoObj *pair;
    switch (side & 255) {
    case 0: pair = D_8009C294; break;
    case 1: pair = D_8009C2B4; break;
    default: return;
    }
    CasinoStartAnim(pair, 32, 0, 0, 0, -320, 0, 0, 0, 0, 0);
    CasinoStartAnim(pair + 1, 32, 0, 0, 0, 320, 0, 0, 0, 0, 0);
}
