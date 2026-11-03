/* Persona 1 (JP) - choose an animation step by side and event. */
#include <decomp/types.h>
extern void CasinoAnimStep0(void);
extern void CasinoAnimStep1(void);

void func_8007F368(u_char side, int event)
{
    switch (side) {
    case 0:
        if (event == 0) CasinoAnimStep0();
        if (event == 2) CasinoAnimStep1();
        break;
    case 1:
        if (event == 0) CasinoAnimStep1();
        if (event == 2) CasinoAnimStep0();
        break;
    }
}
