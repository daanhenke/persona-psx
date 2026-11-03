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
extern short CasinoOpenSeq(u_long* seq, short vab, short voll, short volr);
extern void  CasinoCloseSeq(short* h);

/* One sequence in the list. */
typedef struct
{
    u_char  bank; /* 1: the field's bank, 2: the casino's */
    u_char  pad[3];
    u_long* seq;
} CasinoTestSeq;

/* Each game's list, in the overlay's data. */
extern u_char D_80093C0C[];

/* Array view keeps the byte cursor address live across read/modify/write. */
extern s8 sound_pick[1] __asm__("g_casino_soundtest");

void CasinoSoundTestStep(CasinoTestSeq* t);

void CasinoSoundTest(u_char game)
{
    switch (game)
    {
        case 1:
            CasinoSoundTestStep((CasinoTestSeq*)(D_80093C0C + 0x8));
            break;
        case 2:
            CasinoSoundTestStep((CasinoTestSeq*)(D_80093C0C + 0x120));
            break;
        case 3:
            CasinoSoundTestStep((CasinoTestSeq*)(D_80093C0C + 0x268));
            break;
        case 4:
            CasinoSoundTestStep((CasinoTestSeq*)(D_80093C0C + 0x3A0));
            break;
        case 5:
            CasinoSoundTestStep((CasinoTestSeq*)(D_80093C0C + 0x468));
            break;
    }
}

/* The bank is loaded as an unsigned halfword, then narrowed by the sequence
 * API inside this shared player. Keep each bank's player call in its branch. */
static __inline__ void CasinoTestPlay(u_long* seq, int bank)
{
    CasinoCloseSeq(&g_casino_seqs[14]);
    g_casino_seqs[14] = CasinoOpenSeq(seq, bank, 0x7F, 0x7F);
    SsSeqPlay(g_casino_seqs[14], 1, 1);
}

void CasinoSoundTestStep(CasinoTestSeq* t)
{
    u_long* seq;
    u_short vab;

    if (g_casino_pad_trig & PAD_UP)
    {
        sound_pick[0]++;
    }
    else if (g_casino_pad_trig & PAD_DOWN)
    {
        sound_pick[0]--;
    }
    else if (g_casino_pad_trig & 0x20)
    {
        if (t[sound_pick[0]].bank != 1)
        {
            if (t[sound_pick[0]].bank != 2)
                goto check;
            seq = t[sound_pick[0]].seq;
            vab = g_casino_vab;
            CasinoTestPlay(seq, vab);
        }
        else
        {
            seq = t[sound_pick[0]].seq;
            vab = g_casino_main_vab;
            CasinoTestPlay(seq, vab);
        }
    }
check:
    {
        s8* pick = sound_pick;
        if (t[*pick].seq == (u_long*)-1)
        {
            *pick = *pick ? *pick - 1 : 1;
        }
    }
}
