/* Persona 1 (JP) - move two stored coordinate pairs by eleven units. */
#include <decomp/types.h>

extern u_short D_800B0720;
extern u_short pair1_y __asm__("D_800B0720 + 0x2");
extern u_short D_800B0810;
extern u_short pair2_y __asm__("D_800B0810 + 0x2");

void func_8008FEC4(void)
{
    u_short *first = &D_800B0720;

    *first += 11;
    pair1_y += 11;
    D_800B0810 += 11;
    pair2_y += 11;
}
