/* Persona 1 (JP) - set an entry state and its associated word. */
#include <decomp/types.h>

extern char D_800B5E28[];
extern char D_800B5E30[];

void func_800999D0(int index, long value)
{
    *(short *)(D_800B5E28 + index * 40) = 2;
    *(long *)(D_800B5E30 + 0x1C + index * 40) = value;
}
