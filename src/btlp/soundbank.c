/* Persona 1 (JP) - BTLP overlay @ 0x80081558, 0x8008168C, 0x800817EC
 *
 * Battle sound comes out of up to sixteen banks, each a VAB plus the SEQ that
 * plays against it. A bank is described on disc by four words - the VAB
 * header, the VAB body, the SEQ, and how many sub-sequences the SEQ holds -
 * and the three parallel arrays below hold what the SPU handed back for each
 * open slot. A free slot is marked by a negative id in both arrays.
 *
 * The opening fanfare's mark callback is here too, since it is the one thing
 * outside BtlSePlay that drives a sequence directly.
 *
 * BtlSoundOpen takes a slot of -1 to mean "any free one" and returns the slot
 * it used, so callers that do not care can still find their bank afterwards.
 * Slot 4 is the battle BGM, which is why BtlWaitBgmEnd reads that one entry on
 * its own.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libsnd.h>
#include <persona/btlp/sound.h>

#ifdef NON_MATCHING
/* A slot of -1 means "any free one", and the slot in hand and the answer are
   one variable, so a caller that did not care still learns which was taken.
   A bank with no VAB header is not a bank: nothing is opened and the answer
   is zero rather than the slot. */
int BtlSoundOpen(const BtlSoundBank *banks, int slot, int index)
{
    short              *p;
    int                 i;
    int                 used;

    used = 0;
    if (banks[index].vh != 0) {
        used = slot;
        if (slot < 0) {
            p = g_btl_vab;
            for (i = 0; i < BTL_SOUND_SLOTS; i++) {
                if (*p < 0) {
                    used = i;
                    break;
                }
                p++;
            }
        }

        SsVabTransCompleted(1);

        g_btl_vab[used] = SsVabOpenHead(banks[index].vh, -1);
        /* The body goes up in whatever pieces the SPU will take, so the call
           is repeated until it stops answering with a negative. */
        while (SsVabTransBody(banks[index].vb, g_btl_vab[used]) < 0) {
            ;
        }

        g_btl_seq[used] = SsSepOpen(banks[index].seq, g_btl_vab[used],
                                    banks[index].nsep);
        g_btl_seq_count[used] = banks[index].nsep;
    }
    return used;
}
#else
INCLUDE_ASM("btlp/nonmatchings/soundbank", BtlSoundOpen);
#endif


void BtlSoundClose(int slot)
{
    short *seq;
    short *vab;
    int i;

    if (g_btl_seq[slot] < 0 || g_btl_vab[slot] < 0) {
        return;
    }

    for (i = 0; i < g_btl_seq_count[slot]; i++) {
        SsSepStop(g_btl_seq[slot], i);
    }
    SsSepClose(g_btl_seq[slot]);

    /* Taking the two addresses in this order is load-bearing: `seq` is used
       first below, but assigning it first puts the pair in the wrong
       registers. */
    vab = &g_btl_vab[slot];
    seq = &g_btl_seq[slot];
    SsVabClose(*vab);
    *seq = -1;
    *vab = -1;
    g_btl_seq_count[slot] = 0;
}

/* How the battle plays a sound: 44 call sites reach this, from the menu
   cursor to the spell effects. Stopping first is what makes retriggering a
   sound that is already playing restart it rather than do nothing. */
void BtlSePlay(int slot, int seq)
{
    SsSepStop(g_btl_seq[slot], seq);
    SsSepPlay(g_btl_seq[slot], seq, SSPLAY_PLAY, 1);
}

/* Called by libsnd when the opening sequence reaches a mark. The fanfare is
   two sub-sequences: the first ends on BTL_BGM_MARK_NEXT, which is where the
   hit goes and where the callback is re-armed on the second, and the second
   ends on BTL_BGM_MARK_END, which disarms it and lets the battle start. A mark
   that is neither is left alone, and there are others in the data. */
void BtlIntroBgmMark(short seq, short sep, short mark)
{
    switch (mark) {
    case BTL_BGM_MARK_NEXT:
        SsSepStop(g_btl_seq[0], 0);
        BtlSePlay(0, 1);
        SsSetMarkCallback(g_btl_seq[0], 1, BtlIntroBgmMark);
        break;
    case BTL_BGM_MARK_END:
        SsSepStop(g_btl_seq[0], 1);
        SsSetMarkCallback(g_btl_seq[0], 1, 0);
        g_btl_intro_bgm_done = 1;
        break;
    }
}
