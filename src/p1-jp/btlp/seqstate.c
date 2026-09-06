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

extern short g_btl_seq_state;
extern int   g_btl_seq_mode;

extern void BtlUpdateVoices(void);
extern void BtlDrawFrame(void);

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
