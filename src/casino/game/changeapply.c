#include <decomp/types.h>

extern int D_800A96A4[];
extern void func_80090F58(u_char);

void func_80090EEC(int value)
{
    int index = value & 255;
    char *base = (char *)D_800A96A4;
    int *current = (int *)(base + index * 4);
    int *previous = (int *)((char *)(D_800A96A4 + 11) + index * 4);

    if (*current != *previous)
    {
        func_80090F58(index);
        *previous = *current;
    }
}
