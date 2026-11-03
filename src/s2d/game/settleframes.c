/* Persona 1 (JP) - restore saved coordinates and draw four settling frames. */
#include <decomp/types.h>
extern u_short D_800B8FD4, D_800B8FD8, D_800B91E0, D_800B91E4;
extern int D_800B9528;
extern int scene_state __asm__("D_800B94EC+0x8");
extern void func_8008AEA8(void);
extern void S2dDraw(void);

void func_8009BC4C(void)
{
    int i = 0;
    scene_state = 0;
    D_800B9528 = 0;
    D_800B8FD4 = D_800B91E0;
    D_800B8FD8 = D_800B91E4;
    do {
        i++;
        func_8008AEA8();
        S2dDraw();
    } while (i < 4);
}
