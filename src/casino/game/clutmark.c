/* Persona 1 (JP) - clutmark helper. */
#include <decomp/types.h>

extern s8 D_800A9424[][3];
extern void CasinoQueueCluts();
void func_8008A974(int row, int column) {
    s8 *base=(s8*)D_800A9424;
    s8 *entry=base+row*3+column;
    if (*entry == 1) {
        *entry = 0;
        CasinoQueueCluts((column * 3 + row) * 2 + 26, 2, 0x7E24);
    }
}
