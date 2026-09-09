/* Persona 1 (JP) - where the negotiation panel goes.  BTLP only.
 *   0x80075064 BtlPlacePanel
 *
 * BtlDrawPanel runs this once a frame while the panel is up. A rotation is
 * built from g_btl_panel_rot, pushed 100 units away and scaled by
 * g_btl_panel_scale: below unity the panel is a quad whose four corners are
 * projected, and at unity it becomes a plain sprite with no projection at all.
 *
 * Then one wedge per mood gauge. Two of a wedge's corners never move; the third
 * slides along whichever axis its row names, by the gauge's value scaled down
 * by twelve bits, and sits on the panel's midline in the other axis. While the
 * panel is showing its second image every wedge collapses onto that midline.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>

/* The panel is 0x38 tall about its own centre, so everything given in panel
   coordinates is lifted by half of that. */
#define PANEL_MID 0x1C

/* g_btl_panel_scale, below which the panel is still growing. */
#define PANEL_UNITY 0x1000

/* g_btl_panel_image: the one that shows no gauges. */
#define PANEL_FLAT 2

/* Where the panel is drawn and where its texture and palette live. */
#define PANEL_X    8
#define PANEL_Y    0x10
#define PANEL_U    0
#define PANEL_V    0xB0
#define PANEL_W    0x70
#define PANEL_H    0x38
#define PANEL_CLUTX 0
#define PANEL_CLUTY 0x1FC

/* The geometry offset the panel is projected under, and how far away. */
#define PANEL_OFX 8
#define PANEL_OFY 0x2C
#define PANEL_OFZ 100

#define BTL_MOODS 4

/* One wedge per gauge. */
typedef struct {
    /* 0x0 */ short x0;    /* the corner the sliding one starts from */
    /* 0x2 */ short y0;
    /* 0x4 */ short x1;    /* the corner that never moves            */
    /* 0x6 */ short y1;
    /* 0x8 */ short axis;  /* zero slides the x, anything else the y */
    /* 0xA */ short sign;
} BtlPanelWedge;           /* 12 bytes */

extern SVECTOR       g_btl_panel_rot;
extern VECTOR        g_btl_panel_scale;
extern u_char        g_btl_panel_image;
extern SVECTOR       g_btl_panel_face[];
extern long          g_btl_panel_face_xy[];
extern BtlPanelWedge g_btl_panel_wedges[];
extern long          g_btl_panel_wedge_xy[][3];
extern SPRT          g_btl_panel_sprite;
extern short         g_btl_mood_gauge[];

#ifdef NON_MATCHING
void BtlPlacePanel(void)
{
    /* How far a full gauge pushes its wedge, one per gauge. */
    short   push[4] = { -20, 20, -20, 20 };
    SVECTOR tip;
    SVECTOR base;
    SVECTOR foot;
    MATRIX  m;
    int     ofx;
    int     ofy;
    long    p;
    long    flag;
    short  *px;
    short  *py;
    int     i;
    short   v;
    short   y1;
    short  *py1;

    ReadGeomOffset(&ofx, &ofy);
    SetGeomOffset(PANEL_OFX, PANEL_OFY);
    RotMatrix(&g_btl_panel_rot, &m);
    m.t[0] = 0;
    m.t[1] = 0;
    m.t[2] = PANEL_OFZ;
    ScaleMatrix(&m, &g_btl_panel_scale);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (g_btl_panel_scale.vx < PANEL_UNITY) {
        RotTransPers4(&g_btl_panel_face[3], &g_btl_panel_face[2],
                      &g_btl_panel_face[0], &g_btl_panel_face[1],
                      &g_btl_panel_face_xy[0], &g_btl_panel_face_xy[1],
                      &g_btl_panel_face_xy[2], &g_btl_panel_face_xy[3],
                      &p, &flag);
    } else {
        SetSprt(&g_btl_panel_sprite);
        SetSemiTrans(&g_btl_panel_sprite, 0);
        SetShadeTex(&g_btl_panel_sprite, 1);
        g_btl_panel_sprite.x0 = PANEL_X;
        g_btl_panel_sprite.y0 = PANEL_Y;
        g_btl_panel_sprite.v0 = PANEL_V;
        g_btl_panel_sprite.w = PANEL_W;
        g_btl_panel_sprite.u0 = PANEL_U;
        g_btl_panel_sprite.h = PANEL_H;
        g_btl_panel_sprite.clut = GetClut(PANEL_CLUTX, PANEL_CLUTY);
    }

    py = &g_btl_panel_wedges[0].y0;
    px = &g_btl_panel_wedges[0].x0;
    i = 0;
    do {
        /* Through `v` rather than straight into the vector: the original
           stores each coordinate once, with both arms feeding the one store. */
        if (g_btl_panel_wedges[i].axis != 0) {
            tip.vx = PANEL_MID;
            if (g_btl_panel_image != PANEL_FLAT) {
                v = *py + (short)(push[i] * g_btl_mood_gauge[i] >> 12);
            } else {
                v = PANEL_MID;
            }
            tip.vy = v;
        } else {
            if (g_btl_panel_image != PANEL_FLAT) {
                v = *px + (short)(push[i] * g_btl_mood_gauge[i] >> 12);
            } else {
                v = PANEL_MID;
            }
            tip.vx = v;
            tip.vy = PANEL_MID;
        }
        base.vx = *px;
        px += 6;
        base.vy = *py;
        py += 6;
        foot.vx = g_btl_panel_wedges[i].x1;
        /* Read up here rather than at the point of use: the original
           holds it across the four zeroes below. */
        y1 = g_btl_panel_wedges[i].y1;
        base.vy -= PANEL_MID;
        tip.vz = 0;
        base.vz = 0;
        foot.vz = 0;
        tip.vy -= PANEL_MID;
        /* Read back through a pointer: that is what leaves it in the
           register the original uses across the four zeroes above. */
        py1 = &y1;
        foot.vy = *py1 - PANEL_MID;
        RotTransPers3(&tip, &base, &foot,
                      &g_btl_panel_wedge_xy[i][0],
                      &g_btl_panel_wedge_xy[i][1],
                      &g_btl_panel_wedge_xy[i][2], &p, &flag);
        i++;
    } while ((int)py < (int)&g_btl_panel_wedges[BTL_MOODS].y0);

    SetGeomOffset(ofx, ofy);
}
#else
INCLUDE_ASM("btlp/nonmatchings/panelplace", BtlPlacePanel);
#endif

