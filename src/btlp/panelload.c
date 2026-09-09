/* Persona 1 (JP) - reading the pop-up panel in.  BTLP only.
 *   0x80074C5C BtlPanelLoad
 *
 * The panel's graphics arrive packed: unpacking them puts the palette at
 * 0x8014B700 and the tiles right after it, and both halves are queued for
 * VRAM in that order.
 *
 * Then one set of primitives per frame buffer - the textured quad the picture
 * sits on, a shaded quad over it, and four shaded triangles for the corners -
 * so nothing has to be issued again while the panel is on screen. Only the
 * quad's texture corners are fixed here; where it lands on screen is worked
 * out per frame from the panel's own matrix.
 *
 * The panel is left shut and white: the x scale is zero, y and z are at unity,
 * and there is no rotation.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/battle.h>

/* One set of primitives per frame buffer, and four corner triangles each. */
#define PANEL_BUFFERS 2
#define PANEL_CORNERS 4

/* Where the unpacked graphics are staged: the palette first, the tiles after
   it, exactly as the message box stages its own. */
#define PANEL_CLUT  ((u_char *)0x8014B700)
#define PANEL_TILES ((u_long *)0x8014B900)

/* Where they go in VRAM. */
#define PANEL_TILES_X 0x300
#define PANEL_TILES_Y 0x1B0
#define PANEL_TILES_W 0x38
#define PANEL_TILES_H 0x38
#define PANEL_CLUT_Y  0x1FC
#define PANEL_CLUT_W  0x100

/* The picture's corner in the page, and how big it is. */
#define PANEL_U0 0
#define PANEL_V0 0xB0
#define PANEL_U1 0x70
#define PANEL_V1 0xE8

/* Grey leaves the texture untinted; the panel itself starts white and shut. */
#define PANEL_GREY  0x80
#define PANEL_WHITE 0xFF
#define PANEL_FULL  0x1000

extern u_char  *g_btl_panel_pack;
extern POLY_FT4 g_btl_panel_poly[];
extern POLY_G4  g_btl_panel_glow[];
extern DR_MODE  g_btl_panel_mode[];
extern POLY_G3  g_btl_panel_corner[];
extern SVECTOR  g_btl_panel_rot;
extern VECTOR   g_btl_panel_scale;
extern short    g_btl_panel_rgb[];
extern int      g_btl_panel_state;

extern void BtlUnpack(u_char *dst, const u_char *src);
extern void BtlHighlightInitPrims(void);

#ifdef NON_MATCHING
void BtlPanelLoad(void)
{
    POLY_FT4 *p;
    POLY_G4  *g;
    POLY_G3  *t;
    int       i;
    int       j;
    int       k;
    int       off;
    int       glow;
    int       corner;

    i = 0;
    corner = 0;
    glow = 0;
    off = 0;
    BtlUnpack(PANEL_CLUT, g_btl_panel_pack);
    BtlQueueVramLoad(PANEL_TILES, PANEL_TILES_X, PANEL_TILES_Y,
                     PANEL_TILES_W, PANEL_TILES_H);
    BtlQueueVramLoad(PANEL_CLUT, 0, PANEL_CLUT_Y, PANEL_CLUT_W, 1);
    do {
        p = (POLY_FT4 *)((char *)g_btl_panel_poly + off);
        SetPolyFT4(p);
        SetSemiTrans(p, 0);
        SetShadeTex(p, 1);
        p->u0 = PANEL_U0;
        p->v0 = PANEL_V0;
        p->u1 = PANEL_U1;
        p->v1 = PANEL_V0;
        p->u2 = PANEL_U0;
        p->v2 = PANEL_V1;
        p->u3 = PANEL_U1;
        p->v3 = PANEL_V1;
        p->r0 = PANEL_GREY;
        p->g0 = PANEL_GREY;
        p->b0 = PANEL_GREY;
        /* Indexed rather than reached through `p`: the original rebuilds the
           address for these two, having let go of the record's. */
        g_btl_panel_poly[i].tpage = GetTPage(1, 2, PANEL_TILES_X, PANEL_TILES_Y);
        g_btl_panel_poly[i].clut = GetClut(0, PANEL_CLUT_Y);

        g = (POLY_G4 *)((char *)g_btl_panel_glow + glow);
        SetPolyG4(g);
        SetSemiTrans(g, 0);
        SetShadeTex(g, 1);

        j = 0;
        k = 0;
        do {
            t = (POLY_G3 *)((char *)g_btl_panel_corner + corner + k);
            SetPolyG3(t);
            SetSemiTrans(t, 0);
            SetShadeTex(t, 0);
            j++;
            k += sizeof(POLY_G3);
        } while (j < PANEL_CORNERS);

        corner += PANEL_CORNERS * sizeof(POLY_G3);
        glow += sizeof(POLY_G4);
        off += sizeof(POLY_FT4);
        i++;
    } while (i < PANEL_BUFFERS);

    SetDrawMode(&g_btl_panel_mode[0], 0, 0,
                GetTPage(1, 0, PANEL_TILES_X, PANEL_TILES_Y), 0);
    SetDrawMode(&g_btl_panel_mode[1], 0, 0,
                GetTPage(1, 0, PANEL_TILES_X, PANEL_TILES_Y), 0);
    g_btl_panel_scale.vy = PANEL_FULL;
    g_btl_panel_scale.vz = PANEL_FULL;
    g_btl_panel_rot.vx = 0;
    g_btl_panel_rot.vy = 0;
    g_btl_panel_rot.vz = 0;
    g_btl_panel_scale.vx = 0;
    g_btl_panel_rgb[0] = PANEL_WHITE;
    g_btl_panel_rgb[1] = PANEL_WHITE;
    g_btl_panel_rgb[2] = PANEL_WHITE;
    g_btl_panel_state = 0;
    BtlHighlightInitPrims();
}
#else
INCLUDE_ASM("btlp/nonmatchings/panelload", BtlPanelLoad);
#endif

