#include <decomp/types.h>

#include <libsnd.h>
extern short g_casino_seqs[];
typedef struct
{
    u8 pad00[0x98];
    double value98;
    u8 padA0[0x18];
    double valueB8;
    u8 padC0[0x21];
    u8 flag;
    u8 padE2[6];
    u8 pending;
} CasinoReverse;
void func_80092394(CasinoReverse *value)
{
    if (value->pending == 1)
    {
        SsSeqStop(g_casino_seqs[10]);
        SsSeqPlay(g_casino_seqs[10], 1, 1);
        value->valueB8 = -value->valueB8;
        value->value98 = -value->value98;
        value->flag = 1;
        value->pending = 0;
    }
}
