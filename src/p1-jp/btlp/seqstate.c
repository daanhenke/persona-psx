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
#include <types.h>
#include <persona/btlp/window.h>

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
extern void BtlDrawFrame(void);
extern void BtlIndicatorClear(void);
extern void BtlQueueVramLoad(const void *src, int x, int y, int w, int h);

extern const u_short g_btl_text_cluts[];

/* The sequencer's own message window record, and the routine both windows
   are stepped through. Not g_btl_seq, which is the SPU sequence handles. */
extern BtlWindow g_btl_seq_window;
extern void      BtlWindowStep(BtlWindow *w, int flag);

void BtlSeqSetState(int state, int mode)
{
    g_btl_seq_window.state = state;
    g_btl_seq_window.timer = mode;
}

int BtlSeqState(void)
{
    return g_btl_seq_window.state;
}

void BtlRunFrames(int frames)
{
    while (frames-- != 0) {
        BtlDrawFrame();
    }
}

void BtlSeqWaitDone(void)
{
    while (BtlSeqState() != 0) {
        BtlUpdateVoices();
        BtlDrawFrame();
    }
    BtlRunFrames(5);
}

/* Puts the sequencer back to rest, but only out of the state that means the
   script has run to its end - anything still playing is left alone. */
void BtlSeqEndIfDone(void)
{
    if (BtlSeqState() == BTL_SEQ_FINISHED) {
        BtlSeqSetState(BTL_SEQ_REST, 0);
        BtlIndicatorClear();
    }
}

/* Set the sequencer going and leave it to run. Callers with something else to
   do meanwhile use this and poll BtlSeqState themselves. */
void BtlSeqStart(void)
{
    BtlSeqSetState(BTL_SEQ_RUNNING, BTL_SEQ_RUNNING);
}

/* Play whatever scripts the caller has just armed on its objects: wait out
   anything already running, set the sequencer going, and wait for that too. */
void BtlSeqRun(void)
{
    BtlSeqWaitDone();
    BtlSeqSetState(BTL_SEQ_RUNNING, BTL_SEQ_RUNNING);
    BtlSeqWaitDone();
}

/* Empties the window and puts it back where it belongs. */
void BtlSeqReset(void)
{
    g_btl_seq_window.dx = BTL_SEQ_X;
    g_btl_seq_window.dy = BTL_SEQ_Y;
    g_btl_seq_window.vram_x = BTL_SEQ_VRAM_X;
    g_btl_seq_window.state = 0;
    g_btl_seq_window.unused = 0;
    g_btl_seq_window.attr = 0;
    g_btl_seq_window.placed = 0;
    g_btl_seq_window.staged = 0;
    g_btl_seq_window.x = 0;
    g_btl_seq_window.y = 0;
    g_btl_seq_window.vram_y = BTL_SEQ_VRAM_Y;
}

/* What the script left behind. Read once, when the sequencer reaches state 10,
   into the negotiation's own record. */
int BtlSeqAnswer(void)
{
    return g_btl_seq_window.answer;
}

/* Calls BtlSeqReset and nothing else. Most of the places that empty the window
   go through this one rather than calling the reset directly. */
void BtlSeqClear(void)
{
    BtlSeqReset();
}

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
void BtlSeqAdvance(void)
{
    BtlWindowStep(&g_btl_seq_window, 1);
}
