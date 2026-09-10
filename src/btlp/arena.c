/* Persona 1 (JP) - the box the fight stands in.  BTLP only.
 *   0x80082584 BtlDrawArenaBack    0x800828DC BtlDrawArenaRight
 *   0x80082B7C BtlDrawArenaLeft    0x80082E1C BtlDrawArenaBottom
 *   0x800830B4 BtlDrawArenaTop
 *
 * The fight stands in a shallow box: a face in the plane z = 0 and four edges
 * standing ten units in front of it. Each edge is a run of quads built one at
 * a time into the same four corners and put through the GTE, and a quad that
 * comes back facing away is dropped rather than linked.
 *
 * The two upright edges are ten quads of 0x28 with a twelve-wide texture cell
 * each; the two flat ones are six of the same 0x28 with a twenty-wide cell, so
 * the six of them span the whole 0xF0 across. The uprights take the texture
 * row at v 0xE0 and the flats the one at 0xD0.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <inline.h>
#include <libgpu.h>
#include <persona/btlp/battle.h>

/* How far apart the quads are, and how deep the edges stand. */
#define ARENA_STEP  0x28
#define ARENA_DEPTH 0xA

/* Where the four edges are, and how many quads each takes. */
#define ARENA_X       0x78
#define ARENA_Y       0xC8
#define ARENA_UPRIGHT 10
#define ARENA_FLAT    6

/* The two texture rows and the cell widths that go with them. */
#define ARENA_CELL_UP   0xC
#define ARENA_CELL_FLAT 0x14
#define ARENA_V_UP0     0xE0
#define ARENA_V_UP1     0xEA
#define ARENA_V_FLAT0   0xD0
#define ARENA_V_FLAT1   0xDA

/* Where in a frame's primitives the arena's ordering table sits. */
#define ARENA_OT    0xD6C0
#define ARENA_FRAME 0xE660

/* The texture page and palette the arena is drawn from. */
#define ARENA_SLOT 21

/* The back face is six quads across and ten down, in the same 0x28 steps, and
   takes a twenty-wide cell each way. */
#define ARENA_BACK_COLS 6
#define ARENA_BACK_ROWS 10

extern void BtlApproach(short *cur, const short *target, int step);

extern SVECTOR   g_btl_arena_face[];
extern SVECTOR   g_btl_arena_quad[];
extern short     g_btl_arena_fade;
extern short     g_btl_arena_rgb[];

/* The four edges reach the arena's colour by address, one component at a time.
   Through the array symbol gcc hoists the base into a saved register for the
   loop and then has to spill a live value to make room; the original
   re-materialises the address at each of the three uses. */
#define g_btl_arena_r (*(short *)0x800CCA12)
#define g_btl_arena_g (*(short *)0x800CCA14)
#define g_btl_arena_b (*(short *)0x800CCA16)

/* The face, in the plane z = 0. It is the only one of the five put through
   the GTE by hand rather than through RotAverageNclip4, and it is the one that
   walks the arena's colour toward the scene's. */
#ifdef NON_MATCHING
void BtlDrawArenaBack(void)
{
    u_long *ot;
    short  *rgb;
    int     row;
    int     col;
    int     x0;
    int     x1;
    int     y0;
    int     y1;
    int     ymid;
    int     u0;
    int     u1;
    int     v0;
    int     v1;

    row = 0;
    rgb = g_btl_arena_rgb;
    v1 = ARENA_CELL_FLAT;
    v0 = 0;
    y1 = -(ARENA_Y - ARENA_STEP);
    y0 = -ARENA_Y;
    do {
        col = 0;
        ymid = y0;
        u1 = ARENA_CELL_FLAT;
        u0 = 0;
        x1 = -(ARENA_X - ARENA_STEP);
        x0 = -ARENA_X;
        do {
            g_btl_arena_face[0].vx = x0;
            g_btl_arena_face[0].vy = y0;
            g_btl_arena_face[0].vz = 0;
            g_btl_arena_face[1].vx = x1;
            g_btl_arena_face[1].vy = ymid;
            g_btl_arena_face[1].vz = 0;
            g_btl_arena_face[2].vx = x0;
            g_btl_arena_face[2].vy = y1;
            g_btl_arena_face[2].vz = 0;
            g_btl_arena_face[3].vx = x1;
            g_btl_arena_face[3].vy = y1;
            g_btl_arena_face[3].vz = 0;
            gte_ldv3(&g_btl_arena_face[0], &g_btl_arena_face[1],
                     &g_btl_arena_face[2]);
            gte_rtpt();
            gte_stsxy3((long *)&g_btl_polyft4_next->x0,
                       (long *)&g_btl_polyft4_next->x1,
                       (long *)&g_btl_polyft4_next->x2);
            gte_ldv0(&g_btl_arena_face[3]);
            gte_rtps();
            gte_stsxy((long *)&g_btl_polyft4_next->x3);
            g_btl_polyft4_next->u0 = u0;
            g_btl_polyft4_next->v0 = v0;
            g_btl_polyft4_next->u1 = u1;
            g_btl_polyft4_next->v1 = v0;
            g_btl_polyft4_next->u2 = u0;
            g_btl_polyft4_next->v2 = v1;
            g_btl_polyft4_next->u3 = u1;
            x1 += ARENA_STEP;
            g_btl_polyft4_next->v3 = v1;
            x0 += ARENA_STEP;
            g_btl_polyft4_next->r0 = rgb[0];
            col++;
            g_btl_polyft4_next->g0 = rgb[1];
            g_btl_polyft4_next->b0 = rgb[2];
            u0 += ARENA_CELL_FLAT;
            g_btl_polyft4_next->clut = g_btl_clut[ARENA_SLOT];
            g_btl_polyft4_next->tpage = g_btl_tpage[ARENA_SLOT];
            ot = (u_long *)(g_btl_prim_pool + g_btl_frame * ARENA_FRAME
                            + ARENA_OT);
            addPrim(ot, g_btl_polyft4_next);
            g_btl_polyft4_next++;
            u1 += ARENA_CELL_FLAT;
        } while (col < ARENA_BACK_COLS);
        v1 += ARENA_CELL_FLAT;
        v0 += ARENA_CELL_FLAT;
        y1 += ARENA_STEP;
        row++;
        y0 += ARENA_STEP;
    } while (row < ARENA_BACK_ROWS);

    BtlApproach(&g_btl_arena_rgb[0], &g_btl_scene_rgb[0], g_btl_arena_fade);
    BtlApproach(&g_btl_arena_rgb[1], &g_btl_scene_rgb[1], g_btl_arena_fade);
    BtlApproach(&g_btl_arena_rgb[2], &g_btl_scene_rgb[2], g_btl_arena_fade);
}
#else
INCLUDE_ASM("btlp/nonmatchings/arena", BtlDrawArenaBack);
#endif

/* The right edge, standing in the plane x = 0x78 and running upwards. */
#ifdef NON_MATCHING
void BtlDrawArenaRight(void)
{
    u_long *ot;
    long    out[4];
    int     i;
    long    n;
    short   y0;
    short   y1;
    char    u0;
    char    u1;
    char    vtop;

    i = 0;
    vtop = ARENA_V_UP0;
    u1 = ARENA_CELL_UP;
    u0 = 0;
    y1 = (ARENA_Y - ARENA_STEP);
    y0 = ARENA_Y;
    do {
        g_btl_arena_quad[0].vx = ARENA_X;
        g_btl_arena_quad[0].vy = y0;
        g_btl_arena_quad[0].vz = 0;
        g_btl_arena_quad[1].vx = ARENA_X;
        g_btl_arena_quad[1].vy = y1;
        g_btl_arena_quad[1].vz = 0;
        g_btl_arena_quad[2].vx = ARENA_X;
        g_btl_arena_quad[2].vy = y0;
        g_btl_arena_quad[3].vx = ARENA_X;
        g_btl_arena_quad[3].vy = y1;
        /* The depth of the two far corners never changes, and writing it
           after the rest of the quad is what puts it where it goes. */
        g_btl_arena_quad[2].vz = ARENA_DEPTH;
        g_btl_arena_quad[3].vz = ARENA_DEPTH;
        n = RotAverageNclip4(&g_btl_arena_quad[0], &g_btl_arena_quad[1],
                             &g_btl_arena_quad[2], &g_btl_arena_quad[3],
                             (long *)&g_btl_polyft4_next->x0,
                             (long *)&g_btl_polyft4_next->x1,
                             (long *)&g_btl_polyft4_next->x2,
                             (long *)&g_btl_polyft4_next->x3,
                             &out[0], &out[1], &out[2]);
        if (n > 0) {
            g_btl_polyft4_next->u0 = u0;
            g_btl_polyft4_next->v0 = vtop;
            g_btl_polyft4_next->u1 = u1;
            g_btl_polyft4_next->v1 = vtop;
            g_btl_polyft4_next->u2 = u0;
            g_btl_polyft4_next->v2 = ARENA_V_UP1;
            g_btl_polyft4_next->u3 = u1;
            g_btl_polyft4_next->v3 = ARENA_V_UP1;
            g_btl_polyft4_next->r0 = g_btl_arena_r;
            g_btl_polyft4_next->g0 = g_btl_arena_g;
            g_btl_polyft4_next->b0 = g_btl_arena_b;
            g_btl_polyft4_next->clut = g_btl_clut[ARENA_SLOT];
            g_btl_polyft4_next->tpage = g_btl_tpage[ARENA_SLOT];
            ot = (u_long *)(g_btl_prim_pool + g_btl_frame * ARENA_FRAME
                            + ARENA_OT);
            addPrim(ot, g_btl_polyft4_next);
            g_btl_polyft4_next++;
        }
        u1 += ARENA_CELL_UP;
        u0 += ARENA_CELL_UP;
        y1 -= ARENA_STEP;
        i++;
        y0 -= ARENA_STEP;
    } while (i < ARENA_UPRIGHT);
}
#else
INCLUDE_ASM("btlp/nonmatchings/arena", BtlDrawArenaRight);
#endif

/* The left edge, in the plane x = -0x78, running downwards. */
#ifdef NON_MATCHING
void BtlDrawArenaLeft(void)
{
    u_long *ot;
    long    out[4];
    int     i;
    long    n;
    short   y0;
    short   y1;
    char    u0;
    char    u1;
    char    vtop;

    i = 0;
    vtop = ARENA_V_UP0;
    u1 = ARENA_CELL_UP;
    u0 = 0;
    y1 = (-ARENA_Y + ARENA_STEP);
    y0 = -ARENA_Y;
    do {
        g_btl_arena_quad[0].vx = -ARENA_X;
        g_btl_arena_quad[0].vy = y0;
        g_btl_arena_quad[0].vz = 0;
        g_btl_arena_quad[1].vx = -ARENA_X;
        g_btl_arena_quad[1].vy = y1;
        g_btl_arena_quad[1].vz = 0;
        g_btl_arena_quad[2].vx = -ARENA_X;
        g_btl_arena_quad[2].vy = y0;
        g_btl_arena_quad[3].vx = -ARENA_X;
        g_btl_arena_quad[3].vy = y1;
        /* The depth of the two far corners never changes, and writing it
           after the rest of the quad is what puts it where it goes. */
        g_btl_arena_quad[2].vz = ARENA_DEPTH;
        g_btl_arena_quad[3].vz = ARENA_DEPTH;
        n = RotAverageNclip4(&g_btl_arena_quad[0], &g_btl_arena_quad[1],
                             &g_btl_arena_quad[2], &g_btl_arena_quad[3],
                             (long *)&g_btl_polyft4_next->x0,
                             (long *)&g_btl_polyft4_next->x1,
                             (long *)&g_btl_polyft4_next->x2,
                             (long *)&g_btl_polyft4_next->x3,
                             &out[0], &out[1], &out[2]);
        if (n > 0) {
            g_btl_polyft4_next->u0 = u0;
            g_btl_polyft4_next->v0 = vtop;
            g_btl_polyft4_next->u1 = u1;
            g_btl_polyft4_next->v1 = vtop;
            g_btl_polyft4_next->u2 = u0;
            g_btl_polyft4_next->v2 = ARENA_V_UP1;
            g_btl_polyft4_next->u3 = u1;
            g_btl_polyft4_next->v3 = ARENA_V_UP1;
            g_btl_polyft4_next->r0 = g_btl_arena_r;
            g_btl_polyft4_next->g0 = g_btl_arena_g;
            g_btl_polyft4_next->b0 = g_btl_arena_b;
            g_btl_polyft4_next->clut = g_btl_clut[ARENA_SLOT];
            g_btl_polyft4_next->tpage = g_btl_tpage[ARENA_SLOT];
            ot = (u_long *)(g_btl_prim_pool + g_btl_frame * ARENA_FRAME
                            + ARENA_OT);
            addPrim(ot, g_btl_polyft4_next);
            g_btl_polyft4_next++;
        }
        u1 += ARENA_CELL_UP;
        u0 += ARENA_CELL_UP;
        y1 += ARENA_STEP;
        i++;
        y0 += ARENA_STEP;
    } while (i < ARENA_UPRIGHT);
}
#else
INCLUDE_ASM("btlp/nonmatchings/arena", BtlDrawArenaLeft);
#endif

/* The bottom edge, in the plane y = 0xC8, running to the right.
 *
 * The odd one of the four: eight bytes shorter than the top edge it mirrors,
 * because the original's allocator spent its ninth saved register on the
 * colour's address and built addPrim's 0x00FFFFFF mask inside the loop, where
 * the other three do the reverse. Both forms of the colour access, and a local
 * pointer for it, were tried here; each leaves gcc holding the mask. */
#ifdef NON_MATCHING
void BtlDrawArenaBottom(void)
{
    u_long *ot;
    long    out[4];
    int     i;
    long    n;
    short   x0;
    short   x1;
    char    u0;
    char    u1;
    char    vtop;

    i = 0;
    vtop = ARENA_V_FLAT0;
    u1 = ARENA_CELL_FLAT;
    u0 = 0;
    x1 = (-ARENA_X + ARENA_STEP);
    x0 = -ARENA_X;
    do {
        g_btl_arena_quad[0].vx = x0;
        g_btl_arena_quad[0].vy = ARENA_Y;
        g_btl_arena_quad[0].vz = 0;
        g_btl_arena_quad[1].vx = x1;
        g_btl_arena_quad[1].vy = ARENA_Y;
        g_btl_arena_quad[1].vz = 0;
        g_btl_arena_quad[2].vx = x0;
        g_btl_arena_quad[2].vy = ARENA_Y;
        g_btl_arena_quad[3].vx = x1;
        g_btl_arena_quad[3].vy = ARENA_Y;
        /* The depth of the two far corners never changes, and writing it
           after the rest of the quad is what puts it where it goes. */
        g_btl_arena_quad[2].vz = ARENA_DEPTH;
        g_btl_arena_quad[3].vz = ARENA_DEPTH;
        n = RotAverageNclip4(&g_btl_arena_quad[0], &g_btl_arena_quad[1],
                             &g_btl_arena_quad[2], &g_btl_arena_quad[3],
                             (long *)&g_btl_polyft4_next->x0,
                             (long *)&g_btl_polyft4_next->x1,
                             (long *)&g_btl_polyft4_next->x2,
                             (long *)&g_btl_polyft4_next->x3,
                             &out[0], &out[1], &out[2]);
        if (n > 0) {
            g_btl_polyft4_next->u0 = u0;
            g_btl_polyft4_next->v0 = vtop;
            g_btl_polyft4_next->u1 = u1;
            g_btl_polyft4_next->v1 = vtop;
            g_btl_polyft4_next->u2 = u0;
            g_btl_polyft4_next->v2 = ARENA_V_FLAT1;
            g_btl_polyft4_next->u3 = u1;
            g_btl_polyft4_next->v3 = ARENA_V_FLAT1;
            g_btl_polyft4_next->r0 = g_btl_arena_r;
            g_btl_polyft4_next->g0 = g_btl_arena_g;
            g_btl_polyft4_next->b0 = g_btl_arena_b;
            g_btl_polyft4_next->clut = g_btl_clut[ARENA_SLOT];
            g_btl_polyft4_next->tpage = g_btl_tpage[ARENA_SLOT];
            ot = (u_long *)(g_btl_prim_pool + g_btl_frame * ARENA_FRAME
                            + ARENA_OT);
            addPrim(ot, g_btl_polyft4_next);
            g_btl_polyft4_next++;
        }
        u1 += ARENA_CELL_FLAT;
        u0 += ARENA_CELL_FLAT;
        x1 += ARENA_STEP;
        i++;
        x0 += ARENA_STEP;
    } while (i < ARENA_FLAT);
}
#else
INCLUDE_ASM("btlp/nonmatchings/arena", BtlDrawArenaBottom);
#endif

/* The top edge, in the plane y = -0xC8, running to the left. */
#ifdef NON_MATCHING
void BtlDrawArenaTop(void)
{
    u_long *ot;
    long    out[4];
    int     i;
    long    n;
    short   x0;
    short   x1;
    char    u0;
    char    u1;
    char    vtop;

    i = 0;
    vtop = ARENA_V_FLAT0;
    u1 = ARENA_CELL_FLAT;
    u0 = 0;
    x1 = (ARENA_X - ARENA_STEP);
    x0 = ARENA_X;
    do {
        g_btl_arena_quad[0].vx = x0;
        g_btl_arena_quad[0].vy = -ARENA_Y;
        g_btl_arena_quad[0].vz = 0;
        g_btl_arena_quad[1].vx = x1;
        g_btl_arena_quad[1].vy = -ARENA_Y;
        g_btl_arena_quad[1].vz = 0;
        g_btl_arena_quad[2].vx = x0;
        g_btl_arena_quad[2].vy = -ARENA_Y;
        g_btl_arena_quad[3].vx = x1;
        g_btl_arena_quad[3].vy = -ARENA_Y;
        /* The depth of the two far corners never changes, and writing it
           after the rest of the quad is what puts it where it goes. */
        g_btl_arena_quad[2].vz = ARENA_DEPTH;
        g_btl_arena_quad[3].vz = ARENA_DEPTH;
        n = RotAverageNclip4(&g_btl_arena_quad[0], &g_btl_arena_quad[1],
                             &g_btl_arena_quad[2], &g_btl_arena_quad[3],
                             (long *)&g_btl_polyft4_next->x0,
                             (long *)&g_btl_polyft4_next->x1,
                             (long *)&g_btl_polyft4_next->x2,
                             (long *)&g_btl_polyft4_next->x3,
                             &out[0], &out[1], &out[2]);
        if (n > 0) {
            g_btl_polyft4_next->u0 = u0;
            g_btl_polyft4_next->v0 = vtop;
            g_btl_polyft4_next->u1 = u1;
            g_btl_polyft4_next->v1 = vtop;
            g_btl_polyft4_next->u2 = u0;
            g_btl_polyft4_next->v2 = ARENA_V_FLAT1;
            g_btl_polyft4_next->u3 = u1;
            g_btl_polyft4_next->v3 = ARENA_V_FLAT1;
            g_btl_polyft4_next->r0 = g_btl_arena_r;
            g_btl_polyft4_next->g0 = g_btl_arena_g;
            g_btl_polyft4_next->b0 = g_btl_arena_b;
            g_btl_polyft4_next->clut = g_btl_clut[ARENA_SLOT];
            g_btl_polyft4_next->tpage = g_btl_tpage[ARENA_SLOT];
            ot = (u_long *)(g_btl_prim_pool + g_btl_frame * ARENA_FRAME
                            + ARENA_OT);
            addPrim(ot, g_btl_polyft4_next);
            g_btl_polyft4_next++;
        }
        u1 += ARENA_CELL_FLAT;
        u0 += ARENA_CELL_FLAT;
        x1 -= ARENA_STEP;
        i++;
        x0 -= ARENA_STEP;
    } while (i < ARENA_FLAT);
}
#else
INCLUDE_ASM("btlp/nonmatchings/arena", BtlDrawArenaTop);
#endif

