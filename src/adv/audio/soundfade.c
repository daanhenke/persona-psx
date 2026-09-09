/* Persona 1 (JP) - fading a sequence out while the frame loop keeps running.
 *   ADV 0x800858DC
 *
 * Spins the frame loop while the fade happens, so the screen keeps updating
 * through it rather than freezing. A unit of its own, far from the playback
 * wrappers it shares a source with; see sound.c.
 */
#include <decomp/types.h>

extern void SsSeqSetDecrescendo(short seq, short vol, short time);
extern void AdvRunFrame(void);

/* Reached by hardcoded address rather than through a linker symbol. */
#define g_seq_handle ((short *)0x801F537C)   /* one open handle per slot */

/* Fades a sequence out and spins the frame loop while it happens, so the
   screen keeps updating through the fade rather than freezing on it. */
void SoundFadeOutSeq(u_char slot, u_char vol, short time, short frames)
{
    SsSeqSetDecrescendo(g_seq_handle[slot], vol, time);
    while (frames != 0) {
        AdvRunFrame();
        frames--;
    }
}
