/* Persona 1 (JP) - start the one- or two-digit display animation. */
#include <decomp/types.h>
extern void *D_800A2BC8[];
extern void CasinoStartAnim();

void func_80091544(u_char value)
{
    if (value < 10) {
        CasinoStartAnim(D_800A2BC8[0x3464 / 4], 16, -0x400, 0, 0, 0, 0, 0, 0, 0, 0);
    } else {
        CasinoStartAnim(D_800A2BC8[0x3478 / 4], 16, -0x400, 0, 0, 0, 0, 0, 0, 0, 0);
        CasinoStartAnim(D_800A2BC8[0x348C / 4], 16, -0x400, 0, 0, 0, 0, 0, 0, 0, 0);
    }
}
