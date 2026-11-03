/* Persona 1 (JP) - map the encoded text ranges to glyph rows. */
#include <decomp/types.h>

void func_8008C724(u_short *dst, u_short code)
{
    if ((u_short)(code - 0x9F82) <= 0x5200) {
        switch (code) {
        case 0xED82:
        {
            int value = 0x1004E;
            *dst = value;
            break;
        }
        case 0xF082:
        case 0xF182:
            *dst = (code >> 8) - 0xA1;
            break;
        default:
            *dst = (code >> 8) - 0x9E;
            break;
        }
    } else if ((u_short)(code - 0x4F82) <= 0x0900) {
        *dst = (code >> 8) + 0x53;
    } else if ((u_short)(code - 0x6082) <= 0x1900) {
        *dst = (code >> 8) + 0x60;
    } else if ((u_short)(code - 0x8082) <= 0x1900) {
        *dst = (code >> 8) - 0x63;
    } else {
        *dst = 0;
    }
}
