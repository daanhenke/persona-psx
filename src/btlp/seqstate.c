/* Persona 1 (JP) - the battle sequencer's run state.
 *
 *   BTLP @ 0x8007BF00, 0x8007C53C, 0x8007BE24, 0x80079FE0
 *         0x8007C54C BtlSeqReset, 0x8007C5BC BtlSeqPlay
 *
 * A battle animation is driven by a script the sequencer walks one frame at a
 * time, and the sequencer owns one of the overlay's two message windows. Its
 * run state is that window's own state field, and the value BtlSeqSetState
 * takes alongside it is the window's timer.
 *
 * The state is not a flag. 0, 4, 6 and 8 are all set, and the callers that care
 * which one wait for a particular value - the turn-start path pumps frames
 * until it reads 8, then sets 8/4 itself. Zero still means nothing is playing,
 * which is what BtlSeqWaitDone waits for before running five more frames so the
 * last one is actually on the screen.
 */
#include <decomp/types.h>
#include <persona/btlp/window.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/text.h>

/* Three of the four values seen: nothing playing, running, and the one
   BtlSeqEndIfDone treats as "the script reached its end". */
#define BTL_SEQ_REST     0
#define BTL_SEQ_RUNNING  4
#define BTL_SEQ_FINISHED 8

/* Where this window sits, and where its glyphs are staged. Unlike the second
   window it has a fixed corner of VRAM rather than one of two pages. */
#define BTL_SEQ_X      0x20
#define BTL_SEQ_Y      0xAC
#define BTL_SEQ_VRAM_X 0x380
#define BTL_SEQ_VRAM_Y 0x100

/* Its palettes go beside the glyphs, four rows of sixteen colours. */
#define BTL_SEQ_CLUT_Y 0x180
#define BTL_SEQ_CLUT_W 0x10
#define BTL_SEQ_CLUT_H 4

extern void BtlUpdateVoices(void);

/* The sequencer's own message window record, and the routine both windows
   are stepped through. Not g_btl_seq, which is the SPU sequence handles. */
extern BtlWindow g_btl_seq_window;

void BtlRunFrames(int frames)
{
    while (frames-- != 0) {
        BtlDrawFrame();
    }
}
