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
#define BTL_HUD_HOLD 0x20

/* How long the panel sits at full size before the phase settles. */
#define BTL_HUD_HOLD_FRAMES 15

/* Full size, in the twelve-fraction-bit fixed point the rest of the overlay
   scales with. */
#define BTL_HUD_FULL 0x1000

extern short  g_btl_hud_flags;
extern u_char g_btl_hud_state;
extern int    g_btl_hud_scale;
extern int    g_btl_hud_scale_y;
extern int    g_btl_hud_scale_z;
extern u_char g_btl_hud_hold;
extern u_char g_btl_fast_anim;

void BtlHudShow(void)
{
    g_btl_hud_flags = BTL_HUD_DRAWN;
    g_btl_hud_state = BTL_HUD_ZOOM_IN;
    if (g_btl_fast_anim != 0) {
        g_btl_hud_scale = BTL_HUD_FULL;
        g_btl_hud_scale_y = BTL_HUD_FULL;
    }
}

void BtlHudHide(void)
{
    g_btl_hud_state = BTL_HUD_ZOOM_OUT;
    if (g_btl_fast_anim != 0) {
        g_btl_hud_scale = 0;
        g_btl_hud_scale_y = 0;
    }
}

int BtlHudState(void)
{
    return g_btl_hud_state;
}

/* One frame of the zoom. The two scales ramp at different rates in each
   direction, so the panel stretches as it grows rather than scaling evenly:
   on the way in the first runs at twice the second's rate, on the way out the
   second runs at twice the first's. Reaching full size parks the phase on a
   fifteen-frame hold before it settles; shrinking past nothing clears the
   drawn bit, which is what actually takes the panel off screen. */
void BtlHudTick(void)
{
    /* The phase byte is reached through a pointer: the ramp writes it back
       from three of the arms, and one materialised address is what the
       original keeps. */
    u_char *state = &g_btl_hud_state;

    switch (*state) {
    case BTL_HUD_SETTLED:  /* nothing to ramp */
        break;
    case BTL_HUD_HOLD:
        g_btl_hud_hold--;
        if (g_btl_hud_hold == 0) {
            *state = BTL_HUD_SETTLED;
        }
        break;
    case BTL_HUD_ZOOM_IN:
        g_btl_hud_scale += 0x100;
        if (g_btl_hud_scale > BTL_HUD_FULL) {
            g_btl_hud_scale = BTL_HUD_FULL;
        }
        g_btl_hud_scale_y += 0x80;
        if (g_btl_hud_scale_y > BTL_HUD_FULL) {
            g_btl_hud_scale = BTL_HUD_FULL;
            g_btl_hud_scale_y = BTL_HUD_FULL;
            g_btl_hud_scale_z = BTL_HUD_FULL;
            *state = BTL_HUD_HOLD;
            g_btl_hud_hold = BTL_HUD_HOLD_FRAMES;
        }
        break;
    case BTL_HUD_ZOOM_OUT:
        g_btl_hud_scale -= 0x80;
        g_btl_hud_scale_y -= 0x100;
        if (g_btl_hud_scale_y < 0) {
            g_btl_hud_scale_y = 0;
        }
        if (g_btl_hud_scale <= 0) {
            g_btl_hud_scale = 0;
            g_btl_hud_scale_y = 0;
            g_btl_hud_scale_z = 0;
            g_btl_hud_flags = 0;
            *state = BTL_HUD_SETTLED;
        }
        break;
    }
}
