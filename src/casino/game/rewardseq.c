extern unsigned char D_800A9724[];
extern short g_casino_seqs[],g_casino_vab,g_casino_main_vab;
extern void CasinoPlaySeq(short*,unsigned int,short);
void func_80093950(void)
{
    switch(D_800A9724[2])
    {
    case 2:
        CasinoPlaySeq(&g_casino_seqs[1],1290248,g_casino_vab);
        break;
    case 3:
        CasinoPlaySeq(&g_casino_seqs[1],1290188,g_casino_vab);
        break;
    case 4:
        CasinoPlaySeq(&g_casino_seqs[1],1291248,g_casino_main_vab);
        break;
    case 5:
        CasinoPlaySeq(&g_casino_seqs[1],1291308,g_casino_main_vab);
        break;
    case 6:
        CasinoPlaySeq(&g_casino_seqs[1],1291368,g_casino_main_vab);
        break;
    case 7:
        CasinoPlaySeq(&g_casino_seqs[1],1291428,g_casino_main_vab);
        break;
    case 8:
        CasinoPlaySeq(&g_casino_seqs[1],1291488,g_casino_main_vab);
        break;
    case 9:
        CasinoPlaySeq(&g_casino_seqs[1],1291548,g_casino_main_vab);
        break;
    case 10:
        CasinoPlaySeq(&g_casino_seqs[1],1291068,g_casino_main_vab);
        break;
    case 11:
        CasinoPlaySeq(&g_casino_seqs[1],1291128,g_casino_main_vab);
        break;
    case 12:
        CasinoPlaySeq(&g_casino_seqs[1],1291188,g_casino_main_vab);
        break;
    }
}
