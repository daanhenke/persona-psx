/* Persona 1 (JP) - seqrestart1 helper. */
#include <decomp/types.h>

extern short g_casino_seqs[];
extern s8 g_casino_step;
extern int g_casino_timer;
extern u_char sequence_state __asm__("D_800B0AF8+0x3");
extern void SsSeqStop(short);
extern void SsSeqPlay(short, short, short);
void D_80081570(void) {
    SsSeqStop(g_casino_seqs[4]);
    SsSeqPlay(g_casino_seqs[4], 1, 1);
    sequence_state = 27;
    g_casino_step = 122;
    g_casino_timer = -1;
}
