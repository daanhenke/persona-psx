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
#include <types.h>
#include <libgte.h>

/* What BtlDrawPanel is looking at. */
#define PANEL_SHUT    0
#define PANEL_OPENING 1
#define PANEL_OPEN    2
#define PANEL_CLOSING 3

/* The scale is fixed point with twelve fractional bits, as the HUD's highlight
   bar is, and moves a sixteenth of full size a frame. */
#define PANEL_FULL 0x1000
#define PANEL_STEP 0x100

/* The colour is kept in shorts rather than bytes so the ramp can run past zero
   and be caught; BtlDrawPanel's prims take the low byte of each. */
#define PANEL_WHITE 0xFF
#define PANEL_FADE  8

extern u_char g_btl_fast_anim;
extern int    g_btl_panel_state;
extern VECTOR g_btl_panel_scale;
extern short  g_btl_panel_rgb[];

void BtlPanelOpen(void)
{
    g_btl_panel_state = PANEL_OPENING;
}

void BtlPanelClose(void)
{
    g_btl_panel_state = PANEL_CLOSING;
}

int BtlPanelStepOpen(void)
{
    short *rgb;
    int    more;

    if (g_btl_fast_anim != 0) {
        g_btl_panel_scale.vx = PANEL_FULL;
        g_btl_panel_rgb[0] = 0;
        g_btl_panel_rgb[1] = 0;
        g_btl_panel_rgb[2] = 0;
    }
    g_btl_panel_scale.vx += PANEL_STEP;
    if (g_btl_panel_scale.vx > PANEL_FULL) {
        g_btl_panel_scale.vx = PANEL_FULL;
    }
    if (g_btl_panel_scale.vx != PANEL_FULL) {
        return 1;
    }
    /* Only the red channel is watched for the end of the ramp, and it is the
       one reached through a pointer; the other two keep the array spelling,
       which is what puts their addresses back in $at. */
    rgb = g_btl_panel_rgb;
    *rgb -= PANEL_FADE;
    g_btl_panel_rgb[1] -= PANEL_FADE;
    g_btl_panel_rgb[2] -= PANEL_FADE;
    /* Both arms assigning `more` rather than returning outright is
       load-bearing; leave it. */
    if (*rgb < 0) {
        *rgb = 0;
        g_btl_panel_rgb[1] = 0;
        g_btl_panel_rgb[2] = 0;
        more = 0;
    } else {
        more = 1;
    }
    return more;
}

int BtlPanelStepClose(void)
{
    if (g_btl_fast_anim != 0) {
        g_btl_panel_scale.vx = 0;
        g_btl_panel_rgb[0] = PANEL_WHITE;
        g_btl_panel_rgb[1] = PANEL_WHITE;
        g_btl_panel_rgb[2] = PANEL_WHITE;
    }
    g_btl_panel_scale.vx -= PANEL_STEP;
    if (g_btl_panel_scale.vx < 0) {
        g_btl_panel_rgb[0] = PANEL_WHITE;
        g_btl_panel_rgb[1] = PANEL_WHITE;
        g_btl_panel_rgb[2] = PANEL_WHITE;
    }
    return g_btl_panel_scale.vx >= 0;
}
