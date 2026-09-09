/* Persona 1 (JP) - the battle screen's pop-up panel.
 *
 * The panel is a textured quad put through the GTE rather than a flat sprite,
 * so it can be opened by scaling. Only the x scale moves: y and z are pinned
 * at unity when the panel is built, and the opening ramps x from nothing to
 * full over sixteen frames, which reads on screen as the panel widening out
 * of the middle. Once it is full width the colour fades from white to black
 * over the next thirty-two, and the panel is only counted as open when both
 * have finished. Closing is the scale alone, run back down.
 *
 * Each step returns whether it still has work to do; BtlDrawPanel calls the
 * one the state names and drops the state to shut or open when it says it is
 * done.
 *
 * Drawing follows the same split: while the panel is still scaling it goes
 * out as the transformed quad, and once it is at full size the flat sprite is
 * used instead - one template copied into the buffer's own prim.
 */
#ifndef PERSONA_BTLP_PANEL_H
#define PERSONA_BTLP_PANEL_H

#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>

/* What BtlDrawPanel is looking at. */
#define PANEL_SHUT    0
#define PANEL_OPENING 1
#define PANEL_OPEN    2
#define PANEL_CLOSING 3

/* The scale is fixed point with twelve fractional bits, as the HUD's
   highlight bar is, and moves a sixteenth of full size a frame. */
#define PANEL_FULL 0x1000
#define PANEL_STEP 0x100

/* The colour is kept in shorts rather than bytes so the ramp can run past
   zero and be caught; the prims take the low byte of each. */
#define PANEL_WHITE 0xFF
#define PANEL_FADE  8

/* Frame buffers, and the corner pieces each one carries. */
#define BTL_PANEL_BUFFERS 2
#define BTL_PANEL_CORNERS 4

extern int    g_btl_panel_state;
extern u_char g_btl_panel_image;
extern u_char g_btl_panel_lit;
extern VECTOR g_btl_panel_scale;
extern short  g_btl_panel_rgb[];

/* One set of prims per frame buffer, and the one template the flat sprite is
   copied from. */
extern POLY_FT4 g_btl_panel_poly[];
extern POLY_G3  g_btl_panel_corner[][BTL_PANEL_CORNERS];
extern SPRT     g_btl_panel_flat[];
extern SPRT     g_btl_panel_sprite;
extern DR_MODE  g_btl_panel_mode[];

extern int  BtlPanelStepOpen(void);
extern int  BtlPanelStepClose(void);
extern void BtlPanelOpen(void);
extern void BtlPanelClose(void);
extern void BtlPlacePanel(void);
extern void BtlDrawPanelBox(int panel);
extern void BtlDrawPanel(int buf, u_long *ot);

#endif
