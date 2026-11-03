/* Persona 1 (JP) - decrement reel positions with wrap and look up symbols. */
#include <decomp/types.h>

extern u_char *D_800B3E28[];

void func_800870B4(u_char *symbols, s8 *positions, s8 *previous)
{
    int i;

    for (i = 0; i < 5; i++) {
        if (positions[i] == 0) {
            previous[i] = 27;
        } else {
            previous[i] = positions[i] - 1;
        }
        symbols[i] = D_800B3E28[i][previous[i]];
    }
}

void func_80087124(s8 group, u_char *symbols, s8 *positions, s8 *previous)
{
    int i;

    for (i = group * 4 + 5; i < group * 4 + 9; i++) {
        if (positions[i] == 0) {
            previous[i] = 27;
        } else {
            previous[i] = positions[i] - 1;
        }
        symbols[i] = D_800B3E28[i][previous[i]];
    }
}
