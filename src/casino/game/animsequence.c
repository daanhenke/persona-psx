/* Persona 1 (JP) - step a four-event animation sequence in either direction. */
#include <decomp/types.h>
extern void CasinoAnimStep0(void);
extern void CasinoAnimStep1(void);
extern void CasinoAnimStep2(void);
extern void func_8007F578(void);

void func_8007F1D0(u_char side, int event)
{
    switch (side) {
    case 0:
        if (event == 0) CasinoAnimStep0();
        if (event == 2) CasinoAnimStep1();
        if (event == 4) CasinoAnimStep2();
        if (event == 6) func_8007F578();
        break;
    case 1:
        if (event == 0) func_8007F578();
        if (event == 2) CasinoAnimStep2();
        if (event == 4) CasinoAnimStep1();
        if (event == 6) CasinoAnimStep0();
        break;
    }
}
