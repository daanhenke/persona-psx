/* Persona 1 (JP) - BTLP overlay @ 0x80081558, 0x8008168C
 *
 * Battle sound comes out of up to sixteen banks, each a VAB plus the SEQ that
 * plays against it. A bank is described on disc by four words - the VAB
 * header, the VAB body, the SEQ, and how many sub-sequences the SEQ holds -
 * and the three parallel arrays below hold what the SPU handed back for each
 * open slot. A free slot is marked by a negative id in both arrays.
 *
 * BtlSoundOpen takes a slot of -1 to mean "any free one" and returns the slot
 * it used, so callers that do not care can still find their bank afterwards.
 * Slot 4 is the battle BGM, which is why BtlWaitBgmEnd reads that one entry on
 * its own.
 */
#include <types.h>
#include <libsnd.h>
#include <persona/btlp/sound.h>

int BtlSoundOpen(const BtlSoundBank *banks, int slot, int index)
{
    const BtlSoundBank *b;
    short *p;
    short *vab;
    int i;
    int used;

    used = 0;
    if (banks[index].vh != 0) {
        if (slot < 0) {
            p = g_btl_vab;
            for (i = 0; i < BTL_SOUND_SLOTS; i++) {
                if (*p < 0) {
                    slot = i;
                    break;
                }
                p++;
            }
        }

        SsVabTransCompleted(1);

        b = &banks[index];
        vab = &g_btl_vab[slot];
        *vab = SsVabOpenHead(b->vh, -1);
        while (SsVabTransBody(b->vb, *vab) < 0) {
            ;
        }

        b = &banks[index];
        g_btl_seq[slot] = SsSepOpen(b->seq, g_btl_vab[slot], b->nsep);
        g_btl_seq_count[slot] = b->nsep;
        used = slot;
    }
    return used;
}

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
void BtlSePlay(int slot, short seq)
{
    SsSepStop(g_btl_seq[slot], seq);
    SsSepPlay(g_btl_seq[slot], seq, SSPLAY_PLAY, 1);
}
