#include <libsnd.h>

extern signed char D_800AF834[];
extern short g_casino_seqs[];

void func_80086D40(int index)
{
    switch (*(short *)(D_800AF834 + 0xDA + index * 2))
    {
    case 0:
        D_800AF834[0x104 + index] = 16;
        D_800AF834[0x119 + index] = 32;
        break;
    case 1:
        D_800AF834[0x104 + index] = 8;
        D_800AF834[0x119 + index] = 8;
        break;
    case 2:
        D_800AF834[0x104 + index] = 8;
        D_800AF834[0x119 + index] = -8;
        break;
    case 3:
        D_800AF834[0x104 + index] = 4;
        D_800AF834[0x119 + index] = 4;
        break;
    case 4:
        D_800AF834[0x104 + index] = 4;
        D_800AF834[0x119 + index] = -4;
        break;
    case 5:
        D_800AF834[0x104 + index] = 2;
        D_800AF834[0x119 + index] = 2;
        break;
    case 6:
        D_800AF834[0x104 + index] = 2;
        D_800AF834[0x119 + index] = -2;
        break;
    case 7:
        D_800AF834[0x104 + index] = 1;
        D_800AF834[0x119 + index] = 1;
        break;
    case 8:
        D_800AF834[0x104 + index] = 1;
        D_800AF834[0x119 + index] = -1;
        D_800AF834[0xB0 + index] = 0;
        if (D_800AF834[6] == 0)
            SsSeqStop(g_casino_seqs[8]);
        SsSeqPlay(g_casino_seqs[8],1,1);
        D_800AF834[6]++;
        break;
    }
}

void func_80086EEC(int index)
{
    switch (*(short *)(D_800AF834 + 0xDA + index * 2))
    {
    case 0:
        D_800AF834[0x104 + index] = 8;
        D_800AF834[0x119 + index] = 16;
        break;
    case 1:
        D_800AF834[0x104 + index] = 16;
        D_800AF834[0x119 + index] = 16;
        break;
    case 2:
        D_800AF834[0x104 + index] = 8;
        D_800AF834[0x119 + index] = 8;
        break;
    case 3:
        D_800AF834[0x104 + index] = 8;
        D_800AF834[0x119 + index] = -8;
        break;
    case 4:
        D_800AF834[0x104 + index] = 4;
        D_800AF834[0x119 + index] = 4;
        break;
    case 5:
        D_800AF834[0x104 + index] = 4;
        D_800AF834[0x119 + index] = -4;
        break;
    case 6:
        D_800AF834[0x104 + index] = 2;
        D_800AF834[0x119 + index] = 2;
        break;
    case 7:
        D_800AF834[0x104 + index] = 2;
        D_800AF834[0x119 + index] = -2;
        break;
    case 8:
        D_800AF834[0x104 + index] = 1;
        D_800AF834[0x119 + index] = 1;
        break;
    case 9:
        D_800AF834[0x104 + index] = 1;
        D_800AF834[0x119 + index] = -1;
        D_800AF834[0xB0 + index] = 0;
        if (D_800AF834[5] == 0)
        {
            SsSeqStop(g_casino_seqs[9]);
            SsSeqPlay(g_casino_seqs[9],1,1);
            D_800AF834[5] = 2;
        }
        else if (D_800AF834[5]-- < 0)
            D_800AF834[5] = 0;
        break;
    }
}
