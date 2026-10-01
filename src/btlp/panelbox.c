/* Persona 1 (JP) - building the primitives for one command panel.  BTLP only.
 *   0x80075474 BtlDrawPanelBox
 *
 * BtlDrawPanel runs three things in a row: BtlPlacePanel works out where the
 * panel goes, this turns that into primitives, and BtlHighlightDraw puts the
 * cursor on it.
 *
 * BtlPlacePanel leaves the panel's four projected corners in
 * g_btl_panel_face_xy and the four wedges' triangles in g_btl_panel_wedge_xy.
 * Here the corners go into the panel's textured quad and its gouraud quad, and
 * each wedge gets a triangle whose three vertices are shaded from the
 * highlight colour.
 *
 * A wedge's colour is a mix of two things: the highlight colour scaled by
 * `level`, and white scaled by `flash`, both 12-bit fractions clamped at full
 * brightness.  Which of the two the panel animates is what g_btl_panel_image
 * selects, and g_btl_panel_lit says which of the four corners take part.
 *
 *   image 1 - the lit corners pulse white; flash follows a sine, level stays
 *             at 1.0, so the panel breathes.
 *   image 2 - the lit corners alternate between the first two highlight
 *             colours, one swap every 0x1000 of phase, and level itself is the
 *             sine so the colour fades in and out with it.
 *
 * g_btl_panel_phase advances 0x80 a frame either way; rsin turns it into the
 * -0x1000..0x1000 the mixes want.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/panel.h>

/* One full brightness in the 12-bit fraction the mixes work in. */
#define BTL_MIX_ONE 0x1000
#define BTL_MIX_SHIFT 12

/* How far the sine advances each frame, and the quarter turn image 2 leads by. */
#define BTL_PHASE_STEP 0x80
#define BTL_PHASE_LEAD 0x400

/* Which of the two alternating colours: bit 12 of the phase. */
#define BTL_PHASE_SWAP 12

#define BTL_WHITE 0xFF

/* What g_btl_panel_image selects. */
#define BTL_PANEL_STILL 0
#define BTL_PANEL_PULSE 1
#define BTL_PANEL_CYCLE 2


extern int rsin(int a);

/* Each prim table's address is taken into a pointer of its own before the
   panel's entry is indexed off it: the image loads the address ahead of the
   panel * size product and adds the product to it. The wedge loop sets its
   table pointer afresh each round and builds each wedge's address as one
   offset, panel * 112 + i * 28, added to it. The colour stores reach the
   same offset through the table's name. Setting the pointer inside the loop
   keeps the loop long enough that loop.c leaves panel * 112 in it (lifting
   needs threshold * savings * lifetime, 52 * 2 * 2 = 208, to reach the
   loop's insn count). The parenthesised offset is what makes the wedge and
   the colours share it. */
void BtlDrawPanelBox(int panel)
{
    POLY_FT4 *face;
    POLY_FT4 *faces;
    POLY_G4  *glows;
    POLY_G4  *glow;
    POLY_G3  *w;
    POLY_G3 (*corners)[BTL_PANEL_CORNERS];
    CVECTOR  *src;
    u_char   *pick;
    u_char   *pick2;
    short     level[BTL_PANEL_CORNERS] = {
        BTL_MIX_ONE, BTL_MIX_ONE, BTL_MIX_ONE, BTL_MIX_ONE
    };
    short     flash[BTL_PANEL_CORNERS] = { 0, 0, 0, 0 };
    u_char    lit[8];
    CVECTOR   col[BTL_PANEL_CORNERS];
    int       i;
    int       n;

    faces = g_btl_panel_poly;
    face  = &faces[panel];
    face->x0 = g_btl_panel_face_xy[0].vx;
    face->y0 = g_btl_panel_face_xy[0].vy;
    face->x1 = g_btl_panel_face_xy[1].vx;
    face->y1 = g_btl_panel_face_xy[1].vy;
    face->x2 = g_btl_panel_face_xy[2].vx;
    face->y2 = g_btl_panel_face_xy[2].vy;
    face->x3 = g_btl_panel_face_xy[3].vx;
    face->y3 = g_btl_panel_face_xy[3].vy;

    glows = g_btl_panel_glow;
    glow  = &glows[panel];
    glow->x0 = g_btl_panel_face_xy[0].vx;
    glow->y0 = g_btl_panel_face_xy[0].vy;
    glow->x1 = g_btl_panel_face_xy[1].vx;
    glow->y1 = g_btl_panel_face_xy[1].vy;
    glow->x2 = g_btl_panel_face_xy[2].vx;
    glow->y2 = g_btl_panel_face_xy[2].vy;
    glow->x3 = g_btl_panel_face_xy[3].vx;
    glow->y3 = g_btl_panel_face_xy[3].vy;
    glow->r0 = g_btl_panel_rgb[0];
    glow->g0 = g_btl_panel_rgb[1];
    glow->b0 = g_btl_panel_rgb[2];

    i = 0;
    src = g_btl_highlight_rgb;
    for (; i < BTL_PANEL_CORNERS; i++) {
        col[i] = *src;
        src++;
    }

    switch (g_btl_panel_image) {
    case BTL_PANEL_STILL:
        break;

    case BTL_PANEL_PULSE:
        g_btl_panel_phase += BTL_PHASE_STEP;
        for (i = 0; i < BTL_PANEL_CORNERS; i++) {
            if ((g_btl_panel_lit >> i) & 1) {
                flash[i] = (rsin((short)g_btl_panel_phase) + BTL_MIX_ONE) / 4;
            }
        }
        break;

    case BTL_PANEL_CYCLE:
        i = 0;
        pick2 = lit + 1;
        pick = lit;
        for (; i < BTL_PANEL_CORNERS; i++) {
            if ((g_btl_panel_lit >> i) & 1) {
                *pick = i;
                *pick2 = i;
                pick2++;
                pick++;
            }
        }
        for (i = 0; i < BTL_PANEL_CORNERS; i++) {
            col[i] = g_btl_highlight_rgb[
                lit[(g_btl_panel_phase >> BTL_PHASE_SWAP) & 1]];
            level[i] = (rsin((short)g_btl_panel_phase - BTL_PHASE_LEAD)
                        + BTL_MIX_ONE) / 2;
        }
        g_btl_panel_phase += BTL_PHASE_STEP;
        break;
    }

    for (i = 0; i < BTL_PANEL_CORNERS; i++) {
        /* The table's address, which the image holds in a register of its
           own and adds each wedge's offset to. */
        corners = g_btl_panel_corner;
        /* The three vertices go through a pointer and the colours through
           the array: each clamp puts its store in a block of its own, where
           gcc 2.6 has no CSE to carry the pointer, so only the run of six
           coordinates shares a base. */
        w = (POLY_G3 *)((u_char *)corners
                        + (panel * sizeof(corners[0]) + i * sizeof(corners[0][0])));
        w->x0 = g_btl_panel_wedge_xy[i][0].vx;
        w->y0 = g_btl_panel_wedge_xy[i][0].vy;
        w->x1 = g_btl_panel_wedge_xy[i][1].vx;
        w->y1 = g_btl_panel_wedge_xy[i][1].vy;
        w->x2 = g_btl_panel_wedge_xy[i][2].vx;
        w->y2 = g_btl_panel_wedge_xy[i][2].vy;

        n = (col[i].r * level[i] >> BTL_MIX_SHIFT)
            + (flash[i] * BTL_WHITE >> BTL_MIX_SHIFT);
        if (n > BTL_WHITE) {
            n = BTL_WHITE;
        }
        g_btl_panel_corner[panel][i].r0 = n;
        n = (col[i].g * level[i] >> BTL_MIX_SHIFT)
            + (flash[i] * BTL_WHITE >> BTL_MIX_SHIFT);
        if (n > BTL_WHITE) {
            n = BTL_WHITE;
        }
        g_btl_panel_corner[panel][i].g0 = n;
        n = (col[i].b * level[i] >> BTL_MIX_SHIFT)
            + (flash[i] * BTL_WHITE >> BTL_MIX_SHIFT);
        if (n > BTL_WHITE) {
            n = BTL_WHITE;
        }
        g_btl_panel_corner[panel][i].b0 = n;

        /* The two outer vertices sit at a quarter of the corner's colour, so
           the wedge fades away from the panel. */
        n = ((col[i].r >> 2) * level[i] >> BTL_MIX_SHIFT)
            + (flash[i] * BTL_WHITE >> BTL_MIX_SHIFT);
        if (n > BTL_WHITE) {
            n = BTL_WHITE;
        }
        g_btl_panel_corner[panel][i].r1 = n;
        n = ((col[i].g >> 2) * level[i] >> BTL_MIX_SHIFT)
            + (flash[i] * BTL_WHITE >> BTL_MIX_SHIFT);
        if (n > BTL_WHITE) {
            n = BTL_WHITE;
        }
        g_btl_panel_corner[panel][i].g1 = n;
        n = ((col[i].b >> 2) * level[i] >> BTL_MIX_SHIFT)
            + (flash[i] * BTL_WHITE >> BTL_MIX_SHIFT);
        if (n > BTL_WHITE) {
            n = BTL_WHITE;
        }
        g_btl_panel_corner[panel][i].b1 = n;
        n = ((col[i].r >> 2) * level[i] >> BTL_MIX_SHIFT)
            + (flash[i] * BTL_WHITE >> BTL_MIX_SHIFT);
        if (n > BTL_WHITE) {
            n = BTL_WHITE;
        }
        g_btl_panel_corner[panel][i].r2 = n;
        n = ((col[i].g >> 2) * level[i] >> BTL_MIX_SHIFT)
            + (flash[i] * BTL_WHITE >> BTL_MIX_SHIFT);
        if (n > BTL_WHITE) {
            n = BTL_WHITE;
        }
        g_btl_panel_corner[panel][i].g2 = n;
        n = ((col[i].b >> 2) * level[i] >> BTL_MIX_SHIFT)
            + (flash[i] * BTL_WHITE >> BTL_MIX_SHIFT);
        if (n > BTL_WHITE) {
            n = BTL_WHITE;
        }
        g_btl_panel_corner[panel][i].b2 = n;
    }
}

