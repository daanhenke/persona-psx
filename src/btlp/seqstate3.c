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
/* Defined in the unit before this one; the prototype is what
   decides how the arguments are converted. */
extern void BtlSeqWaitDone(void);
extern void BtlSeqEndIfDone(void);
extern void BtlSeqRun(void);
extern void BtlSeqStart(void);
extern void BtlSeqSetState(int state, int mode);
extern int BtlSeqAnswer(void);

int BtlSeqState(void)
{
    return g_btl_seq_window.state;
}

/* Empties the window and puts it back where it belongs. */
void BtlSeqReset(void)
{
    g_btl_seq_window.dx = BTL_SEQ_X;
    g_btl_seq_window.dy = BTL_SEQ_Y;
    g_btl_seq_window.vram_x = BTL_SEQ_VRAM_X;
    g_btl_seq_window.state = 0;
    g_btl_seq_window.delay = 0;
    g_btl_seq_window.attr = 0;
    g_btl_seq_window.placed = 0;
    g_btl_seq_window.staged = 0;
    g_btl_seq_window.x = 0;
    g_btl_seq_window.y = 0;
    g_btl_seq_window.vram_y = BTL_SEQ_VRAM_Y;
}

/* What the script left behind. Read once, when the sequencer reaches state 10,
   into the negotiation's own record. */

/* Puts a script into the sequencer's window and sets it running. */
void BtlSeqPlay(const u_char *script)
{
    BtlQueueVramLoad(g_btl_text_cluts, BTL_SEQ_VRAM_X, BTL_SEQ_CLUT_Y,
                     BTL_SEQ_CLUT_W, BTL_SEQ_CLUT_H);
    BtlSeqReset();
    g_btl_seq_window.script = script;
    g_btl_seq_window.state = 1;
}

/* One frame of the sequencer's message window. The second window is stepped
   through the same routine, with a flag of its own where this passes 1. */
void BtlSeqClear(void)
{
    BtlSeqReset();
}
void BtlSeqAdvance(void)
{
    BtlWindowStep(&g_btl_seq_window, 1);
}
