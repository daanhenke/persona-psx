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
#include <persona/btlp/object.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <decomp/gte.h>
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

extern SVECTOR   g_btl_arena_face[];
extern SVECTOR   g_btl_arena_quad[];

/* Three of the edges reach the arena's colour by address, one component at a
   time. Through the array symbol gcc hoists the base into a saved register
   for the loop, and the original re-materialises the address at each of the
   three uses. reloc.btlp.txt keeps splat from naming those loads, since the
   image has no relocation there. The bottom edge does reach it through the
   symbol.

   Every edge is written as expressions of its counter alone. loop.c then
   steps each coordinate and texture offset itself, which is where the image
   gets its registers from. A run counted down is written `i * -STEP + base`:
   as `base - i * STEP`, gcc steps i * STEP and subtracts every time round. */
#define g_btl_arena_r (*(short *)0x800CCA12)
#define g_btl_arena_g (*(short *)0x800CCA14)
#define g_btl_arena_b (*(short *)0x800CCA16)

/* 95.14%. Written as the edges are, every coordinate worked out from the row
   and the column. The image steps a copy of the row's top (s6) for the
   second corner. Traced through loop.c 2026-09-26: that is two computations
   of row*40-200 that cse cannot prove equal, both lifted out of the column
   loop and merged by combine_givs in the row loop, the second left as a copy
   in the column loop's preheader. The second corner's value is set at the
   top of the column body so it lives long enough to be lifted (loop.c lifts
   an insn only when threshold * uses * lifetime reaches the loop's insn
   count, 134); the first is spelled (row - 5) * 40. That lifts both, gets
   every rgb read and the per-round 0xFF000000 right, but the two givs end in
   one register here (t8) with no copy, so the frame is still 0x30 against
   0x38 and the registers shift by one.
   The tail reads the fade off the rgb pointer (lh -2(s0)); here cse folds
   rgb[-1] into the symbol. A struct {fade; rgb[3]; to[3];} view keeps it
   register-relative but bases on the fade.
   The face, in the plane z = 0. It is the only one of the five put through
   the GTE by hand rather than through RotAverageNclip4, and it is the one that
   walks the arena's colour toward the scene's. */
#ifdef NON_MATCHING
void BtlDrawArenaBack(void)
{
    u_long *ot;
    short  *rgb;
    int     row;
    int     col;
    int     y;

    for (row = 0; row < ARENA_BACK_ROWS; row++) {
        for (col = 0; col < ARENA_BACK_COLS; col++) {
            y = row * ARENA_STEP - ARENA_Y;
            g_btl_arena_face[0].vx = col * ARENA_STEP - ARENA_X;
            g_btl_arena_face[0].vy = (row - ARENA_Y / ARENA_STEP) * ARENA_STEP;
            g_btl_arena_face[0].vz = 0;
            g_btl_arena_face[1].vx = col * ARENA_STEP - (ARENA_X - ARENA_STEP);
            g_btl_arena_face[1].vy = y;
            g_btl_arena_face[1].vz = 0;
            g_btl_arena_face[2].vx = col * ARENA_STEP - ARENA_X;
            g_btl_arena_face[2].vy = row * ARENA_STEP - (ARENA_Y - ARENA_STEP);
            g_btl_arena_face[2].vz = 0;
            g_btl_arena_face[3].vx = col * ARENA_STEP - (ARENA_X - ARENA_STEP);
            g_btl_arena_face[3].vy = row * ARENA_STEP - (ARENA_Y - ARENA_STEP);
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
            g_btl_polyft4_next->u0 = col * ARENA_CELL_FLAT;
            g_btl_polyft4_next->v0 = row * ARENA_CELL_FLAT;
            g_btl_polyft4_next->u1 = col * ARENA_CELL_FLAT + ARENA_CELL_FLAT;
            g_btl_polyft4_next->v1 = row * ARENA_CELL_FLAT;
            g_btl_polyft4_next->u2 = col * ARENA_CELL_FLAT;
            g_btl_polyft4_next->v2 = row * ARENA_CELL_FLAT + ARENA_CELL_FLAT;
            g_btl_polyft4_next->u3 = col * ARENA_CELL_FLAT + ARENA_CELL_FLAT;
            g_btl_polyft4_next->v3 = row * ARENA_CELL_FLAT + ARENA_CELL_FLAT;
            g_btl_polyft4_next->r0 = g_btl_arena_rgb[0];
            g_btl_polyft4_next->g0 = g_btl_arena_rgb[1];
            g_btl_polyft4_next->b0 = g_btl_arena_rgb[2];
            g_btl_polyft4_next->clut = g_btl_clut[ARENA_SLOT];
            g_btl_polyft4_next->tpage = g_btl_tpage[ARENA_SLOT];
            ot = (u_long *)(g_btl_prim_pool + g_btl_frame * ARENA_FRAME
                            + ARENA_OT);
            addPrim(ot, g_btl_polyft4_next);
            g_btl_polyft4_next++;
        }
    }

    rgb = g_btl_arena_rgb;
    BtlApproach(&rgb[0], &rgb[3], rgb[-1]);
    BtlApproach(&rgb[1], &rgb[4], rgb[-1]);
    BtlApproach(&rgb[2], &rgb[5], rgb[-1]);
}
#else
INCLUDE_ASM("btlp/nonmatchings/arena", BtlDrawArenaBack);
#endif

/* The right edge, standing in the plane x = 0x78 and running upwards. */
void BtlDrawArenaRight(void)
{
    u_long *ot;
    long    out[4];
    int     i;
    long    n;

    for (i = 0; i < ARENA_UPRIGHT; i++) {
        g_btl_arena_quad[0].vx = ARENA_X;
        g_btl_arena_quad[0].vy = i * -ARENA_STEP + ARENA_Y;
        g_btl_arena_quad[0].vz = 0;
        g_btl_arena_quad[1].vx = ARENA_X;
        g_btl_arena_quad[1].vy = i * -ARENA_STEP + (ARENA_Y - ARENA_STEP);
        g_btl_arena_quad[1].vz = 0;
        g_btl_arena_quad[2].vx = ARENA_X;
        g_btl_arena_quad[2].vy = i * -ARENA_STEP + ARENA_Y;
        g_btl_arena_quad[3].vx = ARENA_X;
        g_btl_arena_quad[3].vy = i * -ARENA_STEP + (ARENA_Y - ARENA_STEP);
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
            g_btl_polyft4_next->u0 = i * ARENA_CELL_UP;
            g_btl_polyft4_next->v0 = ARENA_V_UP0;
            g_btl_polyft4_next->u1 = i * ARENA_CELL_UP + ARENA_CELL_UP;
            g_btl_polyft4_next->v1 = ARENA_V_UP0;
            g_btl_polyft4_next->u2 = i * ARENA_CELL_UP;
            g_btl_polyft4_next->v2 = ARENA_V_UP1;
            g_btl_polyft4_next->u3 = i * ARENA_CELL_UP + ARENA_CELL_UP;
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
    }
}

/* The left edge, in the plane x = -0x78, running downwards. */
void BtlDrawArenaLeft(void)
{
    u_long *ot;
    long    out[4];
    int     i;
    long    n;

    for (i = 0; i < ARENA_UPRIGHT; i++) {
        g_btl_arena_quad[0].vx = -ARENA_X;
        g_btl_arena_quad[0].vy = i * ARENA_STEP + -ARENA_Y;
        g_btl_arena_quad[0].vz = 0;
        g_btl_arena_quad[1].vx = -ARENA_X;
        g_btl_arena_quad[1].vy = i * ARENA_STEP + (-ARENA_Y + ARENA_STEP);
        g_btl_arena_quad[1].vz = 0;
        g_btl_arena_quad[2].vx = -ARENA_X;
        g_btl_arena_quad[2].vy = i * ARENA_STEP + -ARENA_Y;
        g_btl_arena_quad[3].vx = -ARENA_X;
        g_btl_arena_quad[3].vy = i * ARENA_STEP + (-ARENA_Y + ARENA_STEP);
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
            g_btl_polyft4_next->u0 = i * ARENA_CELL_UP;
            g_btl_polyft4_next->v0 = ARENA_V_UP0;
            g_btl_polyft4_next->u1 = i * ARENA_CELL_UP + ARENA_CELL_UP;
            g_btl_polyft4_next->v1 = ARENA_V_UP0;
            g_btl_polyft4_next->u2 = i * ARENA_CELL_UP;
            g_btl_polyft4_next->v2 = ARENA_V_UP1;
            g_btl_polyft4_next->u3 = i * ARENA_CELL_UP + ARENA_CELL_UP;
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
    }
}

/* The bottom edge, in the plane y = 0xC8, running to the right.
 *
 * The odd one of the four: eight bytes shorter than the top edge it mirrors,
 * because the original's allocator spent its ninth saved register on the
 * colour's address and built addPrim's 0x00FFFFFF mask inside the loop, where
 * the other three do the reverse. Both forms of the colour access, and a local
 * pointer for it, were tried here; each leaves gcc holding the mask. */
void BtlDrawArenaBottom(void)
{
    u_long *ot;
    long    out[4];
    int     i;
    long    n;

    for (i = 0; i < ARENA_FLAT; i++) {
        g_btl_arena_quad[0].vx = i * ARENA_STEP + -ARENA_X;
        g_btl_arena_quad[0].vy = ARENA_Y;
        g_btl_arena_quad[0].vz = 0;
        g_btl_arena_quad[1].vx = i * ARENA_STEP + (-ARENA_X + ARENA_STEP);
        g_btl_arena_quad[1].vy = ARENA_Y;
        g_btl_arena_quad[1].vz = 0;
        g_btl_arena_quad[2].vx = i * ARENA_STEP + -ARENA_X;
        g_btl_arena_quad[2].vy = ARENA_Y;
        g_btl_arena_quad[3].vx = i * ARENA_STEP + (-ARENA_X + ARENA_STEP);
        g_btl_arena_quad[3].vy = ARENA_Y;
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
            g_btl_polyft4_next->u0 = i * ARENA_CELL_FLAT;
            g_btl_polyft4_next->v0 = ARENA_V_FLAT0;
            g_btl_polyft4_next->u1 = i * ARENA_CELL_FLAT + ARENA_CELL_FLAT;
            g_btl_polyft4_next->v1 = ARENA_V_FLAT0;
            g_btl_polyft4_next->u2 = i * ARENA_CELL_FLAT;
            g_btl_polyft4_next->v2 = ARENA_V_FLAT1;
            g_btl_polyft4_next->u3 = i * ARENA_CELL_FLAT + ARENA_CELL_FLAT;
            g_btl_polyft4_next->v3 = ARENA_V_FLAT1;
            g_btl_polyft4_next->r0 = g_btl_arena_rgb[0];
            g_btl_polyft4_next->g0 = g_btl_arena_rgb[1];
            g_btl_polyft4_next->b0 = g_btl_arena_rgb[2];
            g_btl_polyft4_next->clut = g_btl_clut[ARENA_SLOT];
            g_btl_polyft4_next->tpage = g_btl_tpage[ARENA_SLOT];
            ot = (u_long *)(g_btl_prim_pool + g_btl_frame * ARENA_FRAME
                            + ARENA_OT);
            addPrim(ot, g_btl_polyft4_next);
            g_btl_polyft4_next++;
        }
    }
}

/* The top edge, in the plane y = -0xC8, running to the left. */
void BtlDrawArenaTop(void)
{
    u_long *ot;
    long    out[4];
    int     i;
    long    n;

    for (i = 0; i < ARENA_FLAT; i++) {
        g_btl_arena_quad[0].vx = i * -ARENA_STEP + ARENA_X;
        g_btl_arena_quad[0].vy = -ARENA_Y;
        g_btl_arena_quad[0].vz = 0;
        g_btl_arena_quad[1].vx = i * -ARENA_STEP + (ARENA_X - ARENA_STEP);
        g_btl_arena_quad[1].vy = -ARENA_Y;
        g_btl_arena_quad[1].vz = 0;
        g_btl_arena_quad[2].vx = i * -ARENA_STEP + ARENA_X;
        g_btl_arena_quad[2].vy = -ARENA_Y;
        g_btl_arena_quad[3].vx = i * -ARENA_STEP + (ARENA_X - ARENA_STEP);
        g_btl_arena_quad[3].vy = -ARENA_Y;
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
            g_btl_polyft4_next->u0 = i * ARENA_CELL_FLAT;
            g_btl_polyft4_next->v0 = ARENA_V_FLAT0;
            g_btl_polyft4_next->u1 = i * ARENA_CELL_FLAT + ARENA_CELL_FLAT;
            g_btl_polyft4_next->v1 = ARENA_V_FLAT0;
            g_btl_polyft4_next->u2 = i * ARENA_CELL_FLAT;
            g_btl_polyft4_next->v2 = ARENA_V_FLAT1;
            g_btl_polyft4_next->u3 = i * ARENA_CELL_FLAT + ARENA_CELL_FLAT;
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
    }
}

