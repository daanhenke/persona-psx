/* Persona 1 (JP) - sequence playback wrappers.  DNG only.
 *   0x80075D38 SoundRestartSeq  0x80075D88 SoundPlaySeq
 *   0x80075E38 SoundOpenSeq
 *
 * The field's copy of ADV's wrappers (src/adv/audio/sound.c). The sound data
 * is one blob the preload leaves at 0x80180000 whose first words are an
 * offset table, so a sequence's data is at 0x80180000 + g_seq_offset[seq].
 * Handles come back from SsSeqOpen and are parked in g_seq_handle, indexed by
 * a caller-chosen slot so several sequences can be in flight at once.
 */
#include <decomp/types.h>

extern void  SsSetNck(short seq);
extern short SsSeqOpen(u_long *addr, short vabid);
extern void  SsSeqSetVol(short seq, short voll, short volr);
extern void  SsSeqPlay(short seq, short mode, short loop);
extern void  SsSeqStop(short seq);

/* All reached by hardcoded address rather than through a linker symbol. */
#define g_seq_handle ((short *)0x801F537C)   /* one open handle per slot */
#define g_vab_id     ((short *)0x801F535C)   /* VAB ids, by bank         */
#define g_seq_offset ((u_long *)0x80180020)  /* offsets into the blob    */
#define SEQ_DATA     0x80180000

/* Stops the sequence in `slot` and starts it again from the top, looping. */
void SoundRestartSeq(u_short slot)
{
    short *handle = &g_seq_handle[slot];

    SsSeqStop(*handle);
    SsSeqPlay(*handle, 1, 1);
}

/* Replaces whatever is in `slot` and starts the new sequence looping at full
   volume (0x7F on both channels). SsSetNck on the outgoing handle stops the
   old sequence before the new one is opened over it. */
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
