/* Persona 1 (JP) - expire an idle state after 600 ticks. */
#include <decomp/types.h>

extern u_char D_800AAE9C;
extern u_char idle_state __asm__("D_800AAE94 + 0x2");
extern short D_800AAE98;
extern u_char idle_extra __asm__("D_800AAE98 + 0x2");

extern u_char idle_previous __asm__("D_800AAE9C+0x1");
extern u_int g_casino_lamps;

void func_800905CC(void)
{
    if (D_800AAE9C != idle_previous) {
        if (D_800AAE9C) {
            u_int *lamps = &g_casino_lamps;
            idle_extra = 1;
            *lamps |= 0x14;
        } else {
            u_int *lamps = &g_casino_lamps;
            idle_state = 1;
            D_800AAE98 = 0;
            *lamps &= ~0x10;
            *lamps &= ~0x04;
        }
        idle_previous = D_800AAE9C;
    }
}

void func_8009065C(void)
{
    if (!D_800AAE9C && idle_state == 1) {
        if (++D_800AAE98 >= 600) {
            D_800AAE98 = 0;
            idle_state = 0;
            idle_extra = 0;
        }
    }
}
