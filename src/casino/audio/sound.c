/* Persona 1 (JP) - CASINO's sound.
 *   0x8006C890 CasinoInitSound
 *   0x8006C8F8 CasinoGame1LoadSound .. 0x8006CD20 CasinoGame5LoadSound
 *   0x8006CE74 CasinoOpenVab
 *   0x8006CEC8 CasinoPlaySeq
 *   0x8006CF38 CasinoOpenSeq
 *   0x8006CFA0 CasinoCloseSeq
 *   0x8006CFF4 CasinoStopSeqs
 *
 * The casino keeps up to sixteen sequences open in g_casino_seqs (0xFF is a
 * free slot); the last slot holds the field's music, which it inherits. Each
 * game opens its own sound bank and its effects when it starts, some from
 * the bank the field left open and some from its own.
 */
#include <decomp/types.h>
#include <persona/casino/casino.h>

extern short SsVabOpenHead(u_char *addr, short vabid);
extern short SsVabTransBody(u_char *addr, short vabid);
extern short SsVabTransCompleted(short immediate);
extern short SsSeqOpen(u_long *addr, short vabid);
extern void  SsSeqSetVol(short seq, short voll, short volr);
extern void  SsSeqPlay(short seq, char mode, short loop);
extern void  SsSeqStop(short seq);
extern void  SsSetNck(short seq);

extern u_char g_casino_wrapped;
extern u_char D_800B06B4;

#define SEQ_FREE 0xFF

void  CasinoOpenVab(u_long *head, u_long *body, short *vab);
short CasinoOpenSeq(u_long *seq, short vab, short voll, short volr);
void  CasinoCloseSeq(short *h);

void CasinoInitSound(void)
{
    int i;
    int v;

    v = SEQ_FREE;
    for (i = 15; i >= 0; i--) {
        g_casino_seqs[i] = v;
    }
    g_casino_main_vab = g_vab_id[0];
    D_800B06B4 = 1;
    g_casino_wrapped = 0;
    g_bgm_ready = 1;
    g_casino_seqs[15] = g_seq_handle[0];
}

void CasinoGame1LoadSound(void)
{
    CasinoOpenVab((u_long *)0x1180BC, (u_long *)0x1190DC, &g_casino_vab);
    g_casino_seqs[4] = CasinoOpenSeq((u_long *)0x13A4FC, g_casino_main_vab, 0x7F, 0x7F);
    g_casino_seqs[5] = CasinoOpenSeq((u_long *)0x13A4D4, g_casino_main_vab, 0x7F, 0x7F);
    g_casino_seqs[6] = CasinoOpenSeq((u_long *)0x13A4AC, g_casino_main_vab, 0x7F, 0x7F);
    g_casino_seqs[7] = CasinoOpenSeq((u_long *)0x139E70, g_casino_main_vab, 0x7F, 0x7F);
}

void CasinoGame2LoadSound(void)
{
    CasinoOpenVab((u_long *)0x1180DC, (u_long *)0x118EFC, &g_casino_vab);
    g_casino_seqs[4] = CasinoOpenSeq((u_long *)0x138E14, g_casino_main_vab, 0x7F, 0x7F);
    g_casino_seqs[5] = CasinoOpenSeq((u_long *)0x138DEC, g_casino_main_vab, 0x7F, 0x7F);
    g_casino_seqs[6] = CasinoOpenSeq((u_long *)0x138D9C, g_casino_main_vab, 0x7F, 0x7F);
    g_casino_seqs[7] = CasinoOpenSeq((u_long *)0x13880C, g_casino_main_vab, 0x7F, 0x7F);
}

void CasinoGame3LoadSound(void)
{
    short *vab;

    vab = &g_casino_vab;
    CasinoOpenVab((u_long *)0x11815C, (u_long *)0x11977C, vab);
    g_casino_seqs[4] = CasinoOpenSeq((u_long *)0x1265C4, g_casino_main_vab, 0x7F, 0x7F);
    g_casino_seqs[5] = CasinoOpenSeq((u_long *)0x12634C, g_casino_main_vab, 0x7F, 0x7F);
    g_casino_seqs[6] = CasinoOpenSeq((u_long *)0x126324, g_casino_main_vab, 0x7F, 0x7F);
    g_casino_seqs[7] = CasinoOpenSeq((u_long *)0x125F08, g_casino_main_vab, 0x7F, 0x7F);
    g_casino_seqs[8] = CasinoOpenSeq((u_long *)0x125B7C, *vab, 0x7F, 0x7F);
    g_casino_seqs[9] = CasinoOpenSeq((u_long *)0x125BA8, *vab, 0x7F, 0x7F);
    g_casino_seqs[10] = CasinoOpenSeq((u_long *)0x125EE0, *vab, 0x7F, 0x7F);
    g_casino_seqs[11] = CasinoOpenSeq((u_long *)0x125B4C, *vab, 0x7F, 0x7F);
    g_casino_seqs[12] = CasinoOpenSeq((u_long *)0x125AFC, *vab, 0x7F, 0x7F);
    g_casino_seqs[13] = CasinoOpenSeq((u_long *)0x125B24, *vab, 0x7F, 0x7F);
}

void CasinoGame4LoadSound(void)
{
    short *vab;

    vab = &g_casino_vab;
    CasinoOpenVab((u_long *)0x1180A0, (u_long *)0x1198C0, vab);
    g_casino_seqs[4] = CasinoOpenSeq((u_long *)0x12F5C0, g_casino_main_vab, 0x7F, 0x7F);
    g_casino_seqs[5] = CasinoOpenSeq((u_long *)0x12F598, g_casino_main_vab, 0x7F, 0x7F);
    g_casino_seqs[8] = CasinoOpenSeq((u_long *)0x12F4F0, g_casino_main_vab, 0x7F, 0x7F);
    g_casino_seqs[6] = CasinoOpenSeq((u_long *)0x12F5C0, g_casino_main_vab, 0x7F, 0x7F);
    g_casino_seqs[9] = CasinoOpenSeq((u_long *)0x12E84C, *vab, 0x7F, 0x7F);
    g_casino_seqs[10] = CasinoOpenSeq((u_long *)0x12E874, *vab, 0x7F, 0x7F);
}

void CasinoGame5LoadSound(void)
{
    short *vab;

    vab = &g_casino_vab;
    CasinoOpenVab((u_long *)0x118108, (u_long *)0x119D28, vab);
    g_casino_seqs[4] = CasinoOpenSeq((u_long *)0x13B260, g_casino_main_vab, 0x7F, 0x7F);
    g_casino_seqs[5] = CasinoOpenSeq((u_long *)0x13B1D0, g_casino_main_vab, 0x7F, 0x7F);
    g_casino_seqs[6] = CasinoOpenSeq((u_long *)0x13B120, g_casino_main_vab, 0x7F, 0x7F);
    g_casino_seqs[7] = CasinoOpenSeq((u_long *)0x13B148, g_casino_main_vab, 0x7F, 0x7F);
    g_casino_seqs[11] = CasinoOpenSeq((u_long *)0x13B1A8, g_casino_main_vab, 0x7F, 0x7F);
    g_casino_seqs[8] = CasinoOpenSeq((u_long *)0x13AF2C, *vab, 0x7F, 0x7F);
    g_casino_seqs[9] = CasinoOpenSeq((u_long *)0x13AF7C, *vab, 0x7F, 0x7F);
    g_casino_seqs[10] = CasinoOpenSeq((u_long *)0x13B044, *vab, 0x7F, 0x7F);
}

void CasinoOpenVab(u_long *head, u_long *body, short *vab)
{
    *vab = SsVabOpenHead((u_char *)head, -1);
    SsVabTransBody((u_char *)body, *vab);
    SsVabTransCompleted(1);
}

/* Replaces whatever is in the slot and starts the new sequence looping. */
void CasinoPlaySeq(short *h, u_long *seq, short vab)
{
    CasinoCloseSeq(h);
    *h = CasinoOpenSeq(seq, vab, 0x7F, 0x7F);
    SsSeqPlay(*h, 1, 1);
}

short CasinoOpenSeq(u_long *seq, short vab, short voll, short volr)
{
    short h;

    h = SsSeqOpen(seq, vab);
    SsSeqSetVol(h, voll, volr);
    return h;
}

void CasinoCloseSeq(short *h)
{
    if ((u_short)*h != SEQ_FREE) {
        SsSeqStop(*h);
        SsSetNck(*h);
        *h = SEQ_FREE;
    }
}

/* Everything but the field's music. */
void CasinoStopSeqs(void)
{
    int i;

    for (i = 0; i < 15; i++) {
        if ((u_short)g_casino_seqs[i] != SEQ_FREE) {
            SsSeqStop(g_casino_seqs[i]);
            SsSetNck(g_casino_seqs[i]);
            g_casino_seqs[i] = SEQ_FREE;
        }
    }
}
