/* Persona 1 (JP) - the battle HUD's pulsing row bar.  BTLP only.
 *   0x80075D80 BtlHighlightInitPrims   0x80075E48 BtlHighlightBegin
 *   0x80075EB8 BtlHighlightDraw
 *
 * One slanted, semi-transparent bar is drawn over a row of the battle HUD and
 * pulses: BtlHighlightBegin picks a row and a colour, the level rises to full
 * over sixteen frames, holds, and falls again. The bar is five pixels tall
 * with thirteen between rows, and its four corners take four different x
 * values, which is what makes it a parallelogram rather than a box.
 *
 * There is one primitive per frame buffer. Both are set up here and then
 * filled in each frame, so nothing has to re-issue SetPolyG4 while the bar is
 * on screen.
 */
#include <types.h>
#include <libgpu.h>

/* tpage 0x20 is the page the HUD's own texture sits in. */
#define HIGHLIGHT_TPAGE 0x20

/* 0 off, 1 fading in, 2 held, 3 fading out. */
#define HIGHLIGHT_OFF     0
#define HIGHLIGHT_RISING  1
#define HIGHLIGHT_HELD    2
#define HIGHLIGHT_FALLING 3

/* The level is fixed point with twelve fractional bits. */
#define HIGHLIGHT_FULL   0x1000
#define HIGHLIGHT_STEP   0x100

/* Rows are thirteen pixels apart and the bar is five tall. */
#define HIGHLIGHT_PITCH  13
#define HIGHLIGHT_TOP    0x18
#define HIGHLIGHT_BOTTOM 0x1D
#define HIGHLIGHT_LEFT   8

extern POLY_G4 g_btl_highlight_poly[];
extern DR_MODE g_btl_highlight_mode[];
extern int     g_btl_highlight_state;
extern int     g_btl_highlight_level;
extern int     g_btl_highlight_row;
extern int     g_btl_highlight_colour;
extern u_char  g_btl_highlight_x[];
extern u_char  g_btl_highlight_rgb[];

void BtlHighlightInitPrims(void)
{
    POLY_G4 *poly;
    DR_MODE *mode;

    poly = &g_btl_highlight_poly[0];
    SetPolyG4(poly);
    SetSemiTrans(poly, 1);
    SetShadeTex(poly, 0);
    g_btl_highlight_poly[1] = *poly;

    mode = &g_btl_highlight_mode[0];
    SetDrawMode(mode, 0, 0, HIGHLIGHT_TPAGE, 0);
    g_btl_highlight_mode[1] = *mode;
}

/* `which` picks both the colour and the row, and the two do not run in step:
   the rows come out in the order 3, 1, 2, 0.

   The row is assigned before the colour even though the stores come out the
   other way round; writing them in the order they are stored does not. */
void BtlHighlightBegin(int which)
{
    u_char rows[4] = { 3, 1, 2, 0 };

    g_btl_highlight_state = HIGHLIGHT_RISING;
    g_btl_highlight_level = 2;
    g_btl_highlight_row = rows[which];
    g_btl_highlight_colour = which;
}

/* One frame. The level is fixed point with twelve fractional bits, so full
   brightness is 0x1000 and sixteen frames carry it either way. Corners 0 and
   3 are left grey and 1 and 2 take the row's colour, which is what makes the
   bar read as a diagonal sheen rather than a flat block. */
void BtlHighlightDraw(int buf, u_long *ot)
{
    POLY_G4 *p;
    u_char  *rgb;
    int     *state;
    int      top;
    int      bottom;

    state = &g_btl_highlight_state;
    if (*state == HIGHLIGHT_OFF) {
        return;
    }
    switch (*state) {
    case HIGHLIGHT_HELD:
        break;
    case HIGHLIGHT_RISING:
        g_btl_highlight_level += HIGHLIGHT_STEP;
        if (g_btl_highlight_level > HIGHLIGHT_FULL) {
            g_btl_highlight_level = HIGHLIGHT_FULL;
        }
        if (g_btl_highlight_level == HIGHLIGHT_FULL) {
            *state = HIGHLIGHT_HELD;
        }
        break;
    case HIGHLIGHT_FALLING:
        g_btl_highlight_level -= HIGHLIGHT_STEP;
        if (g_btl_highlight_level < 0) {
            g_btl_highlight_level = 0;
        }
        if (g_btl_highlight_level == 0) {
            *state = HIGHLIGHT_OFF;
        }
        break;
    }

    p = &g_btl_highlight_poly[buf];
    top = g_btl_highlight_row * HIGHLIGHT_PITCH + HIGHLIGHT_TOP;
    bottom = g_btl_highlight_row * HIGHLIGHT_PITCH + HIGHLIGHT_BOTTOM;
    p->x0 = g_btl_highlight_x[0] + HIGHLIGHT_LEFT;
    p->y0 = top;
    p->x1 = g_btl_highlight_x[1] + HIGHLIGHT_LEFT;
    p->y1 = top;
    p->x2 = g_btl_highlight_x[2] + HIGHLIGHT_LEFT;
    p->y2 = bottom;
    p->x3 = g_btl_highlight_x[3] + HIGHLIGHT_LEFT;
    p->y3 = bottom;

    rgb = &g_btl_highlight_rgb[g_btl_highlight_colour * 4];
    p->r0 = (u_int)g_btl_highlight_level >> 6;
    p->g0 = (u_int)g_btl_highlight_level >> 6;
    p->b0 = (u_int)g_btl_highlight_level >> 6;
    p->r1 = rgb[0] * g_btl_highlight_level >> 12;
    p->g1 = rgb[1] * g_btl_highlight_level >> 12;
    p->b1 = rgb[2] * g_btl_highlight_level >> 12;
    p->r2 = rgb[0] * g_btl_highlight_level >> 12;
    p->g2 = rgb[1] * g_btl_highlight_level >> 12;
    p->b2 = rgb[2] * g_btl_highlight_level >> 12;
    p->r3 = (u_int)g_btl_highlight_level >> 6;
    p->g3 = (u_int)g_btl_highlight_level >> 6;
    p->b3 = (u_int)g_btl_highlight_level >> 6;
    AddPrim(ot, p);
    AddPrim(ot, &g_btl_highlight_mode[buf]);
}
