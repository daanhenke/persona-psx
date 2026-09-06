/* Persona 1 (JP) - the battle's second message window.  BTLP only.
 *   0x8007CA3C BtlTextState     0x8007D3AC BtlTextWaitDone
 *   0x8007CA4C BtlTextSetState  0x8007CC30 BtlTextAdvance
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

extern short  g_btl_text_state;
extern u_char g_btl_text[];
extern int    g_btl_text_timer;
extern int    g_btl_text_pause;

extern void BtlDrawFrame(void);
extern void BtlRunFrames(int frames);
extern void BtlWindowStep(u_char *window, int pause);

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

/* Puts the window into a state with a timer of its own, and makes it wait at
   the end of a field rather than finish there. */
void BtlTextSetState(short state, int timer)
{
    g_btl_text_state = state;
    g_btl_text_timer = timer;
    g_btl_text_pause = 1;
}

void BtlTextAdvance(void)
{
    BtlWindowStep(g_btl_text, g_btl_text_pause);
}
