#include <persona/casino/bounce.h>

extern short g_casino_seqs[], g_casino_vab;
extern void CasinoPlaySeq(short*, u_int, short);

s32 func_800917F0(CasinoBounce* value)
{
    s32 position;
    if (value->inhibit == 0)
    {
        value->valueC0 = -0.9800000000000002 + value->valueC0;
    }
    value->value78 = 0.1 * value->valueC0 + value->value78;
    position = 762.0 - value->value78;
    if (position >= 762)
    {
        if (value->mode == 0)
        {
            CasinoPlaySeq(g_casino_seqs, 0x13AF54, g_casino_vab);
        }
        func_8009190C(value);
        value->mode = (u8)value->mode + 1;
    }
    return position;
}
