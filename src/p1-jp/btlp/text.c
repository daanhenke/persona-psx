/* Persona 1 (JP) - the battle's second message window.  BTLP only.
 *   0x8007CA3C BtlTextState   0x8007D3AC BtlTextWaitDone
 *
 * The overlay runs two message windows. Both are records of the same shape,
 * with the state at +2, and both are driven by the same per-frame routine; the
 * sequencer owns one of them and this is the other. Opening it clears the whole
 * record and sets the state to 1, and the state goes back to zero when the
 * script runs out.
 *
 * Waiting for it runs five more frames afterwards, exactly as BtlSeqWaitDone
 * does, so the last frame of the message is on the screen before whatever
 * follows starts drawing over it.
 */
#include <types.h>

#define BTL_TEXT_TAIL 5

extern short g_btl_text_state;

extern void BtlDrawFrame(void);
extern void BtlRunFrames(int frames);

int BtlTextState(void)
{
    return g_btl_text_state;
}

void BtlTextWaitDone(void)
{
    while (BtlTextState() != 0) {
        BtlDrawFrame();
    }
    BtlRunFrames(BTL_TEXT_TAIL);
}
