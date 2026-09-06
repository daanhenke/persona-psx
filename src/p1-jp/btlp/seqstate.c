/* Persona 1 (JP) - the battle sequencer's run state.
 *
 *   BTLP @ 0x8007BF00, 0x8007C53C, 0x8007BE24, 0x80079FE0
 *
 * A battle animation is driven by a script the sequencer walks one frame at a
 * time. g_btl_seq_state says where that script has got to and g_btl_seq_mode
 * says which script it is.
 *
 * The state is not a flag. 0, 4, 6 and 8 are all set, and the callers that care
 * which one wait for a particular value - the turn-start path pumps frames
 * until it reads 8, then sets 8/4 itself. Zero still means nothing is playing,
 * which is what BtlSeqWaitDone waits for before running five more frames so the
 * last one is actually on the screen.
 */
#include <types.h>

/* Three of the four values seen: nothing playing, running, and the one
   BtlSeqEndIfDone treats as "the script reached its end". */
#define BTL_SEQ_REST     0
#define BTL_SEQ_RUNNING  4
#define BTL_SEQ_FINISHED 8

extern short g_btl_seq_state;
extern int   g_btl_seq_mode;

extern void BtlUpdateVoices(void);
extern void BtlDrawFrame(void);
extern void BtlIndicatorClear(void);

void BtlSeqSetState(int state, int mode)
{
    g_btl_seq_state = state;
    g_btl_seq_mode = mode;
}

int BtlSeqState(void)
{
    return g_btl_seq_state;
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
