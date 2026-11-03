/* Persona 1 (JP) - choose the palette for two runs of a player's sprites. */
#include <decomp/types.h>
extern void CasinoQueueCluts();

void func_8008AA64(int player, int active)
{
    short first;
    if (active & 255) {
        first = (player & 255) * 25;
        CasinoQueueCluts(first + 86, 2, 0x7EE4);
        CasinoQueueCluts(first + 91, 2, 0x7EE4);
    } else {
        first = (player & 255) * 25;
        CasinoQueueCluts(first + 86, 2, 0x7EA4);
        CasinoQueueCluts(first + 91, 2, 0x7EA4);
    }
}
