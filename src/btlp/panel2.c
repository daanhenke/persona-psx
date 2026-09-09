/* Persona 1 (JP) - the battle screen's pop-up panel.
 *   BTLP @ 0x80074F0C BtlPanelStepOpen, 0x80074FE4 BtlPanelStepClose,
 *         0x80075D40 BtlPanelOpen,     0x80075D54 BtlPanelClose
 *
 * The panel is a textured quad put through the GTE rather than a flat sprite,
 * so it can be opened by scaling. Only the x scale moves: y and z are pinned at
 * unity when the panel is built, and the opening ramps x from nothing to full
 * over sixteen frames, which reads on screen as the panel widening out of the
 * middle. Once it is full width the colour fades from white to black over the
 * next thirty-two, and the panel is only counted as open when both have
 * finished. Closing is the scale alone, run back down.
 *
 * Each step returns whether it still has work to do; BtlDrawPanel calls the one
 * the state names and drops the state to shut or open when it says it is done.
 *
 * The player can turn battle animations off in the config menu, and that byte
 * is copied out of the save block into g_btl_fast_anim when the battle starts.
 * Both steps honour it the same way: they write the state the animation would
 * have ended in and let the single frame that follows finish the job.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/panel.h>

void BtlPanelOpen(void)
{
    g_btl_panel_state = PANEL_OPENING;
}

void BtlPanelClose(void)
{
    g_btl_panel_state = PANEL_CLOSING;
}

/* What the panel shows: which animation the corners run, and which of the four
   corners run it. Both are read by the prim builders and by nothing else, so
   this is the whole of choosing its picture. */
void BtlPanelSetImage(u_char image, u_char lit)
{
    g_btl_panel_image = image;
    g_btl_panel_lit = lit;
}
