/* Persona 1 (JP) - raising and lowering the status panel.  BTLP only.
 *   0x8007E19C BtlHudShow  0x8007E1DC BtlHudHide  0x8007E210 BtlHudState
 *
 * The panel is drawn through the GTE with the indicator on it, and the whole
 * draw is gated on one bit, so hiding it is not a move off screen. Two scales
 * carry the zoom: the per-frame driver ramps them to full size in the phase
 * BtlHudShow starts and back to nothing in the phase BtlHudHide starts, and
 * drops the phase to zero when the ramp finishes. Callers poll BtlHudState for
 * that zero.
 *
 * With battle animations turned off both write the scales the ramp would have
 * ended on, which is the same shortcut the pop-up panel and the camera take.
 */
#include <types.h>

#define BTL_HUD_DRAWN 0x8000

#define BTL_HUD_SETTLED 0
#define BTL_HUD_ZOOM_IN 1
#define BTL_HUD_ZOOM_OUT 6

/* Full size, in the twelve-fraction-bit fixed point the rest of the overlay
   scales with. */
#define BTL_HUD_FULL 0x1000

extern short  g_btl_hud_flags;
extern u_char g_btl_hud_state;
extern int    g_btl_hud_scale[];
extern u_char g_btl_fast_anim;

void BtlHudShow(void)
{
    g_btl_hud_flags = BTL_HUD_DRAWN;
    g_btl_hud_state = BTL_HUD_ZOOM_IN;
    if (g_btl_fast_anim != 0) {
        g_btl_hud_scale[0] = BTL_HUD_FULL;
        g_btl_hud_scale[1] = BTL_HUD_FULL;
    }
}

void BtlHudHide(void)
{
    g_btl_hud_state = BTL_HUD_ZOOM_OUT;
    if (g_btl_fast_anim != 0) {
        g_btl_hud_scale[0] = 0;
        g_btl_hud_scale[1] = 0;
    }
}

int BtlHudState(void)
{
    return g_btl_hud_state;
}
