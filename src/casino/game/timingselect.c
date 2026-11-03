/* Persona 1 (JP) - select timing values for an indexed casino state. */
#include <decomp/types.h>

extern u_char D_800AF834[];

void D_80086AC4(int index)
{
    switch (*(s16 *)(D_800AF834 + 0xDA + index * 2)) {
    case 0:
        D_800AF834[0x104 + index] = 8;
        D_800AF834[0x119 + index] = 8;
        break;
    case 1:
        D_800AF834[0x104 + index] = 12;
        D_800AF834[0x119 + index] = 24;
        break;
    case 2:
        D_800AF834[0x104 + index] = 8;
        D_800AF834[0x119 + index] = 32;
        D_800AF834[0xB0 + index] = 2;
        break;
    }
}

void func_80086B88(int index)
{
    switch (*(s16 *)(D_800AF834 + 0xDA + index * 2)) {
    case 0:
        D_800AF834[0x104 + index] = 4;
        D_800AF834[0x119 + index] = 4;
        break;
    case 1:
        D_800AF834[0x104 + index] = 6;
        D_800AF834[0x119 + index] = 12;
        break;
    case 2:
        D_800AF834[0x104 + index] = 4;
        D_800AF834[0x119 + index] = 16;
        D_800AF834[0xB0 + index] = 2;
        break;
    }
}

void func_80086C4C(int index)
{
    switch (*(s16 *)(D_800AF834 + 0xDA + index * 2)) {
    case 0:
        D_800AF834[0x104 + index] = 8;
        D_800AF834[0x119 + index] = 8;
        break;
    case 1:
        D_800AF834[0x104 + index] = 12;
        D_800AF834[0x119 + index] = 24;
        break;
    case 2:
        D_800AF834[0x104 + index] = 8;
        D_800AF834[0x119 + index] = 32;
        break;
    case 3:
        D_800AF834[0x104 + index] = 4;
        D_800AF834[0x119 + index] = 32;
        D_800AF834[0xB0 + index] = 2;
        break;
    }
}

