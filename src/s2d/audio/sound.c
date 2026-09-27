/* Persona 1 (JP) - sequence playback wrappers.  S2D only.
 *   0x80065D9C SoundRestartSeq  0x80065DEC SoundPlaySeq
 *   0x80065E4C SoundOpenSeq
 *
 * DNG's wrappers (src/dng/audio/sound.c) over S2D's sound blob, which the
 * preload leaves at 0x800E0000 with its offset table at +0x20. The menu
 * sounds are opened up front into handles 7 on, so SoundPlaySeq here only
 * restarts one of them: `slot` and `vab` are the shared prototype's and go
 * unused.
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
#define g_seq_offset ((u_long *)0x800E0020)  /* offsets into the blob    */
#define SEQ_DATA     0x800E0000
#define MENU_SEQ     7                       /* first menu sound handle  */

/* Stops the sequence in `slot` and starts it again from the top, looping. */
void SoundRestartSeq(u_short slot)
{
    short *handle = &g_seq_handle[slot];

    SsSeqStop(*handle);
    SsSeqPlay(*handle, 1, 1);
}

/* Restarts menu sound `seq` at full volume. */
void SoundPlaySeq(u_short slot, u_short seq, short vab)
{
    short *handle = &g_seq_handle[MENU_SEQ + seq];

    SsSeqStop(*handle);
    SsSeqSetVol(*handle, 0x7F, 0x7F);
    SsSeqPlay(*handle, 1, 1);
}

/* Opens and records the handle without starting it. */
void SoundOpenSeq(u_short slot, u_short seq, short vab)
{
    g_seq_handle[slot] =
        SsSeqOpen((u_long *)(g_seq_offset[seq] + SEQ_DATA), g_vab_id[vab]);
}
