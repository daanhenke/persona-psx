#include <decomp/types.h>

u8 func_80082340(u8 *cards)
{
    int count = 0;
    int i;
    for (i = 0; i < 2; i++) {
        u8 rank = (u32)cards[i] % 13;
        if (rank == 0) count++;
    }
    return count;
}
