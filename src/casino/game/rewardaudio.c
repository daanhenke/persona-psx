extern short g_casino_seqs[], g_casino_main_vab;
extern void CasinoPlaySeq(short *, unsigned int, short);

void func_80093AD0(unsigned int value)
{
    if (value < 100)
        CasinoPlaySeq(&g_casino_seqs[1], 0x13B9B8, g_casino_main_vab);
    else if (value < 1000)
        CasinoPlaySeq(&g_casino_seqs[1], 0x13B8D8, g_casino_main_vab);
    else if (value < 10000)
        CasinoPlaySeq(&g_casino_seqs[1], 0x13B7A8, g_casino_main_vab);
    else
        CasinoPlaySeq(&g_casino_seqs[1], 0x13B558, g_casino_main_vab);
}
