/* Persona 1 (JP) - reset three fixed-point scale components and state. */
#include <decomp/types.h>

extern long D_800B5F2C, D_800B5F30, D_800B5F34;
extern char D_800AA73C[];

extern long scale_state __asm__("D_800AA73C + 0x5C1C");
extern long scale_argument __asm__("D_800AA73C + 0x5C20");

void func_8009C308(long value)
{
    D_800B5F2C = 0;
    D_800B5F30 = 0;
    D_800B5F34 = 0;
    scale_state = 1;
    scale_argument = value;
}

void func_8009C33C(void)
{
    D_800B5F2C = 0x1000;
    D_800B5F30 = 0x1000;
    D_800B5F34 = 0x1000;
    *(long *)(D_800AA73C + 0x5C1C) = 3;
}
