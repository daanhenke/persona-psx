/* Persona 1 (JP) - test whether the indexed word is nonzero. */
#include <decomp/types.h>

int func_800915E4(u_char index, u_long *entries)
{
    return entries[index] != 0;
}
