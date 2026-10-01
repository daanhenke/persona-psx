/* Persona 1 (JP) - CASINO's sound test, left over from development.
 *   0x8006D06C CasinoSoundTest
 *   0x8006D100 CasinoSoundTestStep
 *
 * Nothing calls it. Up and down move through the game's list of sequences,
 * and the button plays the one picked (from the field's bank or the
 * casino's) in handle 14. The list ends at a sequence of -1, and the pick
 * steps back off it.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/casino/casino.h>

extern void  SsSeqPlay(short seq, char mode, short loop);
extern short CasinoOpenSeq(u_long *seq, short vab, short voll, short volr);
extern void  CasinoCloseSeq(short *h);

/* One sequence in the list. */
typedef struct {
    u_char  bank;   /* 1: the field's bank, 2: the casino's */
    u_char  pad[3];
    u_long *seq;
} CasinoTestSeq;

/* Each game's list, in the overlay's data. */
extern u_char D_80093C0C[];

extern s8 g_casino_soundtest;

void CasinoSoundTestStep(CasinoTestSeq *t);

void CasinoSoundTest(u_char game)
{
    switch (game) {
    case 1:
        CasinoSoundTestStep((CasinoTestSeq *)(D_80093C0C + 0x8));
        break;
    case 2:
        CasinoSoundTestStep((CasinoTestSeq *)(D_80093C0C + 0x120));
        break;
    case 3:
        CasinoSoundTestStep((CasinoTestSeq *)(D_80093C0C + 0x268));
        break;
    case 4:
        CasinoSoundTestStep((CasinoTestSeq *)(D_80093C0C + 0x3A0));
        break;
    case 5:
        CasinoSoundTestStep((CasinoTestSeq *)(D_80093C0C + 0x468));
        break;
    }
}

#ifdef NON_MATCHING
void CasinoSoundTestStep(CasinoTestSeq *t)
{
    u_long *seq;
    short   vab;

    if (g_casino_pad_trig & PAD_UP) {
        g_casino_soundtest++;
    } else if (g_casino_pad_trig & PAD_DOWN) {
        g_casino_soundtest--;
    } else if (g_casino_pad_trig & 0x20) {
        if (t[g_casino_soundtest].bank == 1) {
            seq = t[g_casino_soundtest].seq;
            vab = g_casino_main_vab;
        } else if (t[g_casino_soundtest].bank == 2) {
            seq = t[g_casino_soundtest].seq;
            vab = g_casino_vab;
        } else {
            goto check;
        }
        CasinoCloseSeq(&g_casino_seqs[14]);
        g_casino_seqs[14] = CasinoOpenSeq(seq, vab, 0x7F, 0x7F);
        SsSeqPlay(g_casino_seqs[14], 1, 1);
    }
check:
    if (t[g_casino_soundtest].seq == (u_long *)-1) {
        g_casino_soundtest = g_casino_soundtest ? g_casino_soundtest - 1 : 1;
    }
}
#else
/* 62%: debug code, not worked through - the image keeps the pick's address
   in a register for every read-modify-write and tests the bank another way. */
INCLUDE_ASM("casino/nonmatchings/debug/soundtest", CasinoSoundTestStep);
#endif
