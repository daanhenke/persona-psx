/* Persona 1 (JP) - waiting for the battle's second message window.  BTLP only.
 *   0x8007D3AC BtlTextWaitDone
 *
 * The state goes back to zero when the script runs out; five more frames after
 * that put the last frame of the message on the screen before whatever follows
 * starts drawing over it, exactly as BtlSeqWaitDone does.
 */
#include <decomp/types.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/text.h>

void BtlTextWaitDone(void)
{
    while (BtlTextState() != 0) {
        BtlDrawFrame();
    }
    BtlRunFrames(BTL_TEXT_TAIL);
}
