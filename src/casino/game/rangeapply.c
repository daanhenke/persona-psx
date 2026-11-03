/* Persona 1 (JP) - apply an operation over an inclusive 16-bit range. */
#include <decomp/types.h>

extern void func_8008CFC0(u_short index);

void func_8008CF68(u_short first, u_short last)
{
    for (; first <= last; first++) {
        func_8008CFC0(first);
    }
}
