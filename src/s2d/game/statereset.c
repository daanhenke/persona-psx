/* Persona 1 (JP) - S2D state reset.  S2D only.
 *   0x80096174 S2dResetState
 */
#include <decomp/types.h>

extern int D_800B94EC[];
extern int D_800A4CE4;

void S2dResetState(void)
{
    D_800B94EC[15] = 0;
    D_800B94EC[2] = 0;
    D_800A4CE4 = 0x80;
}
