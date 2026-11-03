/* Persona 1 (JP) - map the encoded text ranges to glyph rows. */
#include <decomp/types.h>

void func_8008C81C(u_short *dst, u_short code)
{
    if ((u_short)(code - 0x4083) <= 0x3E00) {
        *dst = (code >> 8) + 0x11;
    } else if ((u_short)(code - 0x8083) <= 0x1400) {
        switch (code) {
        case 0x8F83:
            *dst = 0x9E;
            break;
        case 0x9283:
        case 0x9383:
        case 0x9483:
            *dst = (code >> 8) + 0x0D;
            break;
        default:
            *dst = (code >> 8) + 0x10;
            break;
        }
    } else {
        *dst = 0;
    }
}
