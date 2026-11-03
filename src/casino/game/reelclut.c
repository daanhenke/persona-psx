#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>

extern short D_800999D8[], D_800999F0[];
extern void CasinoQueueCluts(int, int, u_short);

/* Callers supply index 0..7 and active 0 or 1. */
void func_80084D50(signed char index, u_char active)
{
    short x, y;

    switch (active)
    {
    case 1:
        switch (index)
        {
        case 0: case 2: case 5: case 7:
            x = D_800999D8[4];
            y = D_800999D8[5];
            break;
        case 1: case 3: case 4: case 6:
            x = D_800999F0[4];
            y = D_800999F0[5];
            break;
        }
        break;
    case 0:
        switch (index)
        {
        case 0: case 2: case 5: case 7:
            x = 592;
            y = 498;
            break;
        case 1: case 3: case 4: case 6:
            x = 592;
            y = 500;
            break;
        }
        break;
    }
    CasinoQueueCluts(index + 376, 1, GetClut(x, y));
}
