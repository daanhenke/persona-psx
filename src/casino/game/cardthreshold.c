/* Persona 1 (JP) - test whether a card occurs fewer than the required times. */
#include <decomp/types.h>

int func_80082750(u_char *cards, u_char wanted, int length, int minimum)
{
    int count = 0;
    int i;

    for (i = 0; i < length; i++) {
        if (cards[i] == wanted) {
            count++;
        }
    }
    return count < minimum;
}
