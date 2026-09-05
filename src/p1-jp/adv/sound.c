/* Persona 1 (JP) - sequence playback wrappers.  ADV @ 0x800662FC / 0x800663B0.
 *
 * DNG (0x80075D88) and S2D (0x80065DEC) carry their own SoundPlaySeq against
 * their own work areas; only ADV has SoundOpenSeq.
 *
 * The sound data is one blob loaded at 0x80118000 whose first words are an
 * offset table, so a sequence's data is at 0x80118000 + g_seq_offset[seq].
 * Handles come back from SsSeqOpen and are parked in g_seq_handle, indexed by
 * a caller-chosen slot so several sequences can be in flight at once.
 */
#include <types.h>

extern void  SsSetNck(short seq);
extern short SsSeqOpen(u_long *addr, short vabid);
extern void  SsSeqSetVol(short seq, short voll, short volr);
extern void  SsSeqPlay(short seq, short mode, short loop);

/* Literal addresses: the indexed accesses assemble to `addu $at,rX,$at`, the
   numeric-base form. */
#define g_seq_handle ((short *)0x801F537C)   /* one open handle per slot */
#define g_vab_id     ((short *)0x801F535C)   /* VAB ids, by bank         */
#define g_seq_offset ((u_long *)0x80118020)  /* offsets into the blob    */
#define SEQ_DATA     0x80118000

/* Replaces whatever is in `slot` and starts the new sequence looping at full
   volume.
 *
 * Both pointer locals are computed before the SsSetNck call on purpose: the
 * original keeps them in saved registers across it, and the assignment order
 * decides which of the two literals is materialised first. Indexing the macros
 * at each use instead rebuilds the second address after the call. */
void SoundPlaySeq(u_short slot, u_short seq, short vab)
{
    short  *handle;
    u_long *offset;
    short   h;

    offset = &g_seq_offset[seq];
    handle = &g_seq_handle[slot];
    SsSetNck(*handle);
    h = SsSeqOpen((u_long *)(*offset + SEQ_DATA), g_vab_id[vab]);
    *handle = h;
    SsSeqSetVol(h, 0x7F, 0x7F);
    SsSeqPlay(*handle, 1, 1);
}

/* Opens and records the handle without starting it - used where the caller
   sets the volume and start point itself. */
void SoundOpenSeq(u_short slot, u_short seq, short vab)
{
    g_seq_handle[slot] =
        SsSeqOpen((u_long *)(g_seq_offset[seq] + SEQ_DATA), g_vab_id[vab]);
}
