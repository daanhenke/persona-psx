/* Persona 1 (JP) - classify special pairs and ace/high-card combinations. */
#include <decomp/types.h>

int func_80081DC0(u_char *cards)
{
    u_char rank;

    if ((cards[0] == 0 && cards[1] == 10) ||
        (cards[0] == 10 && cards[1] == 0)) {
        return 4;
    }
    if ((cards[0] == 13 && cards[1] == 23) ||
        (cards[0] == 23 && cards[1] == 13)) {
        return 5;
    }
    if ((cards[0] == 26 && cards[1] == 36) ||
        (cards[0] == 36 && cards[1] == 26)) {
        return 6;
    }
    if ((cards[0] == 39 && cards[1] == 49) ||
        (cards[0] == 49 && cards[1] == 39)) {
        return 7;
    }
    rank = cards[0] % 13;
    if ((rank >= 9 && (u_char)(cards[1] % 13) == 0) ||
        (rank == 0 && (u_char)(cards[1] % 13) >= 9)) {
        return 8;
    }
    return 1;
}
