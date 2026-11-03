extern short g_casino_seqs[], g_casino_main_vab, g_casino_vab;
extern void CasinoPlaySeq(short *, int, short);
void func_8007FC94(unsigned char value, short channel)
{
    switch (value)
    {
    case 4:
        CasinoPlaySeq(&g_casino_seqs[channel], 0x1389A4, g_casino_main_vab);
        break;
    case 5:
        CasinoPlaySeq(&g_casino_seqs[channel], 0x1389E0, g_casino_main_vab);
        break;
    case 6:
        CasinoPlaySeq(&g_casino_seqs[channel], 0x138A1C, g_casino_main_vab);
        break;
    case 7:
        CasinoPlaySeq(&g_casino_seqs[channel], 0x138A58, g_casino_main_vab);
        break;
    case 8:
        CasinoPlaySeq(&g_casino_seqs[channel], 0x138A94, g_casino_main_vab);
        break;
    case 9:
        CasinoPlaySeq(&g_casino_seqs[channel], 0x138AD0, g_casino_main_vab);
        break;
    case 10:
        CasinoPlaySeq(&g_casino_seqs[channel], 0x1388F0, g_casino_main_vab);
        break;
    case 11:
        CasinoPlaySeq(&g_casino_seqs[channel], 0x13892C, g_casino_main_vab);
        break;
    case 12:
        CasinoPlaySeq(&g_casino_seqs[channel], 0x138968, g_casino_main_vab);
        break;
    case 13:
        CasinoPlaySeq(&g_casino_seqs[channel], 0x138668, g_casino_vab);
        break;
    case 14:
        CasinoPlaySeq(&g_casino_seqs[channel], 0x13853C, g_casino_vab);
        break;
    case 15:
        CasinoPlaySeq(&g_casino_seqs[channel], 0x138578, g_casino_vab);
        break;
    case 16:
        CasinoPlaySeq(&g_casino_seqs[channel], 0x1385B4, g_casino_vab);
        break;
    case 17:
        CasinoPlaySeq(&g_casino_seqs[channel], 0x1385F0, g_casino_vab);
        break;
    case 18:
        CasinoPlaySeq(&g_casino_seqs[channel], 0x13862C, g_casino_vab);
        break;
    case 19:
        CasinoPlaySeq(&g_casino_seqs[channel], 0x138500, g_casino_vab);
        break;
    case 20:
        CasinoPlaySeq(&g_casino_seqs[channel], 0x1386A4, g_casino_vab);
        break;
    case 21:
        CasinoPlaySeq(&g_casino_seqs[channel], 0x138758, g_casino_main_vab);
        break;
    }
}
