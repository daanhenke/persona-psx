#include <decomp/types.h>

extern void CasinoQueueCluts(int, int, u_short);

void func_80085920(u_char active, short end, short start)
{
    int i = start;

    if (active == 1)
    {
        for (; i < end; i++)
        {
            switch (i)
            {
            case 0: case 2: case 5: case 7:
                CasinoQueueCluts(i + 376, 1, 0x7C65);
                break;
            case 1: case 3: case 4: case 6:
                CasinoQueueCluts(i + 376, 1, 0x7CE5);
                break;
            }
        }
    }
    else if (active == 0)
    {
        for (; i > end; i--)
        {
            switch (i - 1)
            {
            case 0: case 2: case 5: case 7:
                CasinoQueueCluts(i + 375, 1, 0x7CA5);
                break;
            case 1: case 3: case 4: case 6:
                CasinoQueueCluts(i + 375, 1, 0x7D25);
                break;
            }
        }
    }
}
