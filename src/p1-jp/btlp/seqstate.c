/* Persona 1 (JP) - the battle sequencer's run state.
 *
 *   BTLP @ 0x8007BF00, 0x8007C53C, 0x8007BE24, 0x80079FE0
 *
 * A battle animation is driven by a script the sequencer walks one frame at a
 * time. g_btl_seq_active is what the rest of the overlay tests to know whether
 * anything is still playing, and g_btl_seq_mode says which script.
 *
 * Callers that need the screen quiet before they carry on - a menu opening, a
 * scene change, the battle ending - call BtlSeqWaitDone, which pumps frames
 * until the script clears the flag and then runs five more so the last one is
 * actually on the screen.
 */
#include <types.h>

extern short g_btl_seq_active;
extern int   g_btl_seq_mode;

extern void BtlUpdateVoices(void);
extern void BtlDrawFrame(void);

void BtlSeqSetState(int active, int mode)
{
    g_btl_seq_active = active;
    g_btl_seq_mode = mode;
}

int BtlSeqIsActive(void)
{
    return g_btl_seq_active;
}

void BtlRunFrames(int frames)
{
    while (frames-- != 0) {
        BtlDrawFrame();
    }
}

void BtlSeqWaitDone(void)
{
    while (BtlSeqIsActive() != 0) {
        BtlUpdateVoices();
        BtlDrawFrame();
    }
    BtlRunFrames(5);
}
