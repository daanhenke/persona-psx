/* Persona 1 (JP) - store a spacing value in map units. */
#include <decomp/types.h>

extern short D_800A85D0;

void func_80099930(int spacing)
{
    D_800A85D0 = spacing * 192;
}
