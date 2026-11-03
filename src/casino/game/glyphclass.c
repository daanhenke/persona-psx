#include <decomp/types.h>

/* Classify the encoded text code after its 16-bit argument conversion. */
void func_8008C6E4(u_short *dst, int code)
{
    code &= 0xFFFF;
    if (code == 0x5181)
    {
        *dst = 0;
    }
    else if (code < 0x5182)
    {
        *dst = 0;
    }
    else if (code == 0x5B81 || code == 0x7C81)
    {
        *dst = 0xAC;
    }
    else
    {
        *dst = 0;
    }
}
