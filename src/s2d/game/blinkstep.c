/* Persona 1 (JP) - advance the two flags through a five-duration cycle. */
#include <decomp/types.h>

extern int D_80064F84[];
extern int blink_counter __asm__("D_800B0FB0+0xD7C");
extern u_char D_800B1D30[];

void func_800945E0(void)
{
    int total = 0;
    int i;

    for (i = 0; i < 5; i++) {
        total += D_80064F84[i];
        if ((blink_counter & 0xFF) < total) {
            break;
        }
    }
    D_800B1D30[0] = (4 >> i) & 1;
    D_800B1D30[1] = (0x11 >> i) & 1;
    blink_counter++;
}
