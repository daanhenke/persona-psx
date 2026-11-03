/* Persona 1 (JP) - clutbits helper. */
#include <decomp/types.h>

extern u_short D_800A86FC;
extern void CasinoQueueCluts();
void func_8008B428(void)
{
    int bit = 0;
    do {
        if ((D_800A86FC >> bit) & 1) {
            CasinoQueueCluts(bit + 6, 1, 0x7FA4);
        } else {
            CasinoQueueCluts(bit + 6, 1, 0x7F64);
        }
        bit++;
    } while (bit < 9);
}
