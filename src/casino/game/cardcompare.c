/* Persona 1 (JP) - compare the ranks of two cards. */
#include <decomp/types.h>

int func_800827EC(u_char first, u_char second)
{
    int ranks[2] = { first, second };
    int i;

    for (i = 0; i < 2; i++) {
        if (ranks[i] == 52) {
            ranks[i] = 14;
        } else if (ranks[i] == 0 || ranks[i] == 13
                   || ranks[i] == 26 || ranks[i] == 39) {
            ranks[i] = 13;
        } else {
            ranks[i] %= 13;
        }
    }
    ranks[0] = ranks[1] - ranks[0];
    if (ranks[0] > 0) {
        return 22;
    } else if (ranks[0] < 0) {
        return 23;
    } else if (ranks[0] == 0) {
        return 25;
    }
}
