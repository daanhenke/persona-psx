#include <decomp/types.h>

extern u8 bets_present __asm__("D_800AAE9C");
extern void CasinoQueueCluts();

void func_8009019C(u_int* bets)
{
    int count = 0;
    int i = 0;
    int sprite = 64;
    u_int* entry = bets;
    do
    {
        if (*entry != 0)
        {
            CasinoQueueCluts(sprite, 1, 0x7CE5);
            count++;
        }
        else
        {
            CasinoQueueCluts(sprite, 1, 0x7CA5);
        }
        sprite++;
        i++;
        entry++;
    } while (i < 11);
    if (count != 0)
    {
        bets_present = 1;
    }
    else
    {
        bets_present = 0;
    }
}
