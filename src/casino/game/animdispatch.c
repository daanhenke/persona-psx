/* Persona 1 (JP) - dispatch the four supported casino animation modes. */
#include <decomp/types.h>
extern void func_8007F1D0(u_char, int);
extern void CasinoPlayStepAnim(u_char, int);
extern void func_8007F368(u_char, int);

void func_8007F160(u_char mode, u_char index, int value)
{
    switch (mode) {
    case 0x73:
        func_8007F1D0(index, value);
        break;
    case 0x72:
        CasinoPlayStepAnim(index, value);
        break;
    case 0x70:
    case 0x71:
        func_8007F368(index, value);
        break;
    }
}
