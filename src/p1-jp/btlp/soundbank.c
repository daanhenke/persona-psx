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

typedef struct {
    /* 0x0 */ u_char *vh;           /* VAB header */
    /* 0x4 */ u_char *vb;           /* VAB body, pushed to SPU RAM */
    /* 0x8 */ u_long *seq;          /* SEQ data */
    /* 0xC */ short   nsep;         /* sub-sequences in the SEQ */
} BtlSoundBank;                     /* 0x10 bytes */

#define BTL_SOUND_SLOTS 16

extern short  g_btl_vab[];
extern short  g_btl_seq[];
extern u_char g_btl_seq_count[];

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
