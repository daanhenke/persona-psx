/* Persona 1 (JP) - handing over from the command to the action.  BTLP only.
 *   0x8006F098 BtlBeginAction
 *
 * Both paths into an action run this the moment a command has been chosen. It
 * closes the panel and then waits, drawing frames, for the sequencer to settle
 * - which is what lets the panel's own close animation play out rather than
 * being cut off - before resetting it.
 *
 * The indicator comes down while the acting character's graphics are read off
 * the disc and goes back up as the bar, so the gap where the disc is busy is
 * covered rather than showing a stale row.
 */
#include <types.h>

/* The state the sequencer settles on, and the one the action runs in. */
#define BTL_SEQ_IDLE   8
#define BTL_SEQ_ACTION 8
#define BTL_ACTION_FRAMES 4

extern short g_btl_actor_slot;

extern void BtlPanelClose(void);
extern int  BtlSeqState(void);
extern void BtlSeqSetState(int state, int frames);
extern void BtlDrawFrame(void);
extern void BtlIndicatorClear(void);
extern void BtlIndicatorBar(void);
extern void BtlLoadActorGfx(int slot);

void BtlBeginAction(void)
{
    BtlPanelClose();
    while (BtlSeqState() != BTL_SEQ_IDLE) {
        BtlDrawFrame();
    }
    BtlSeqSetState(0, 0);
    BtlIndicatorClear();
    BtlLoadActorGfx(g_btl_actor_slot);
    BtlSeqSetState(BTL_SEQ_ACTION, BTL_ACTION_FRAMES);
    BtlIndicatorBar();
}
