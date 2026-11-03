/* Persona 1 (JP) - select a casino card sequence. */
#include <decomp/types.h>
extern short g_casino_main_vab;
extern short g_casino_seqs[];
extern void CasinoPlaySeq(short *, u_int, short);
void func_8007FFAC(s32 arg0, s32 arg1) {
    s32 temp_a0;

    temp_a0 = arg0 & 0xFF;
    switch (temp_a0) {
    case 3:
        CasinoPlaySeq(&g_casino_seqs[(short)arg1], 0x138E40, g_casino_main_vab);
        break;
    case 4:
    case 9:
        CasinoPlaySeq(&g_casino_seqs[(short)arg1], 0x138C6C, g_casino_main_vab);
        break;
    case 5:
    case 6:
    case 7:
    case 10:
        CasinoPlaySeq(&g_casino_seqs[(short)arg1], 0x138B8C, g_casino_main_vab);
        break;
    case 8:
    case 11:
        CasinoPlaySeq(&g_casino_seqs[(short)arg1], 0x139090, g_casino_main_vab);
        break;
    }
}
