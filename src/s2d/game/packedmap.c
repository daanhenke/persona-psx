/* Persona 1 (JP) - read and update packed map bytes.
 * The original tests x/2 for nibble selection; preserve that condition.
 */
#include <decomp/types.h>

extern u_char *packed_map __asm__("D_800B1D30+0x4");

void D_80094A34(short x, short y, u_int value)
{
    u_char *cell = packed_map + y * 100 + x / 2;
    u_char byte;

    if (x / 2 == 0) {
        byte = value | (*cell & 0xF0);
    } else {
        byte = (*cell & 0x0F) | (value * 16);
    }
    *cell = byte;
}

int func_80094AA4(short x, short y)
{
    u_char *cell = packed_map + y * 100 + x / 2;

    if (x / 2 == 0) {
        return *cell & 0x0F;
    }
    return *cell >> 4;
}
