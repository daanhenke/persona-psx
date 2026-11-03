/* Persona 1 (JP) - convert a card's rank to its blackjack value. */
#include <decomp/types.h>

int func_800823A0(u_char card)
{
    u_char rank = card % 13;

    switch (rank) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
        return (u_char)(rank + 1);
    case 10:
    case 11:
    case 12:
        return 10;
    default:
        return 0xFF;
    }
}
