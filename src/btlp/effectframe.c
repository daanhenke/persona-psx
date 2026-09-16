/* Persona 1 (JP) - the box an effect is drawn in.  BTLP only.
 *   0x8007872C BtlDrawEffectFrame   0x80078C50 BtlEffectCursorBox
 *
 * The frame is eight pieces - four corners and four edges - and the run each
 * one is drawn as is kept in a table of eight descriptors: the low byte is how
 * many cells it spans and four bits above it say which corner it hangs off.
 * The two edge runs are stretched to the box's own width and height before the
 * walk starts, and each piece goes to BtlDrawFramePiece with the corner added
 * in. While the box is still growing the pieces are drawn where they stand;
 * once it is out to full size they take the cursor's own offset with them.
 *
 * Behind the pieces goes the box itself, one quad per ordering table, put
 * through the same transform so it sits with them: white while the box is
 * opening and blue once it is dark enough to read against. The cursor handler
 * lays a second quad over the whole box, shaded corner by corner.
 *
 * Both build the box's four corners the same way: each is cleared, filled in
 * and copied out as it is made, and the four are handed over together.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <decomp/libc.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/effect.h>

/* The corners are staged in halfword fields, the way the piece drawer stages
   its own. */
typedef struct {
    u_short vx;
    short   vy;
    short   vz;
    short   pad;
} BtlFrameVec;

/* A cell is eight pixels square, and the frame is drawn two scanlines above
   where it is asked for. */
#define FRAME_CELL 8
#define FRAME_RISE 2

/* Pieces to a frame. */
#define FRAME_PIECES 8

/* Which corner a piece hangs off, in the bits above its run: the box's width
   and height, and one cell of each. */
#define FRAME_AT_RIGHT  0x4000
#define FRAME_AT_BOTTOM 0x2000
#define FRAME_STEP_X    0x1000
#define FRAME_STEP_Y    0x0800

/* Unity for the box's scale, and the point on the way up where it is dark
   enough to read against. */
#define FRAME_FULL 0x1000
#define FRAME_DARK 0x41

/* What the box is drawn in while it opens, and once it is open. */
#define FRAME_OPENING 0x80
#define FRAME_BLUE    0x80

/* The cursor's own quad, shaded from its top corner down. */
#define CURSOR_DIM    0x10
#define CURSOR_MID    0x20
#define CURSOR_BRIGHT 0x40

/* 72.82%. The eight pieces, the two runs stretched to the box, the cursor's
   offset once it is open and the box's own quad are the image's; what is left
   is where each corner's stores sit among the loads that feed them, and how
   the copies into the transform's vectors are paired. Copying the four as one
   array rather than one at a time, and counting the stretch in shorts, both
   come out further away. */
#ifdef NON_MATCHING
int BtlDrawEffectFrame(BtlEffect *e)
{
    u_char   rgb[3];
    SVECTOR  corner[4];
    POLY_F4 *prim;
    int      across;
    int      down;
    long     i;

    {
        SVECTOR     vec[4];
        BtlFrameVec box[4];

        memset(&box[0], 0, sizeof(box[0]));
        box[0].vz = 0;
        box[0].vx = g_btl_effect_ox;
        box[0].vy = g_btl_effect_oy - FRAME_RISE;
        vec[0] = *(SVECTOR *)&box[0];
        memset(&box[1], 0, sizeof(box[1]));
        box[1].vz = 0;
        box[1].vx = g_btl_effect_ox + e->dx * FRAME_CELL;
        box[1].vy = g_btl_effect_oy - FRAME_RISE;
        vec[1] = *(SVECTOR *)&box[1];
        memset(&box[2], 0, sizeof(box[2]));
        box[2].vx = g_btl_effect_ox;
        box[2].vz = 0;
        box[2].vy = g_btl_effect_oy + (u_short)e->dy * FRAME_CELL - FRAME_RISE;
        vec[2] = *(SVECTOR *)&box[2];
        memset(&box[3], 0, sizeof(box[3]));
        box[3].vx = g_btl_effect_ox + e->dx * FRAME_CELL;
        box[3].vz = 0;
        box[3].vy = g_btl_effect_oy + (u_short)e->dy * FRAME_CELL - FRAME_RISE;
        vec[3] = *(SVECTOR *)&box[3];
        corner[0] = vec[0];
        corner[1] = vec[1];
        corner[2] = vec[2];
        corner[3] = vec[3];
    }
    {
        /* Kept in .rodata and copied onto the stack, the way the piece drawer
           keeps its cells. */
        u_short run[FRAME_PIECES] = {
            0x0001, 0x4001, 0x2001, 0x6001, 0x1000, 0x3000, 0x8800, 0xC800,
        };
        int     x;
        int     y;

        across = (short)e->dx - 2;
        if (across < 0) {
            across = 0;
        }
        run[4] += across;
        run[5] += across;
        down = e->dy - 2;
        if (down < 0) {
            down = 0;
        }
        i = 0;
        run[6] += down;
        run[7] += down;
        do {
            x = (((run[i] & FRAME_AT_RIGHT) != 0) * (across + 1)
                 + ((run[i] & FRAME_STEP_X) != 0))
                * FRAME_CELL;
            y = (((run[i] & FRAME_AT_BOTTOM) != 0) * (down + 1)
                 + ((run[i] & FRAME_STEP_Y) != 0))
                * FRAME_CELL;
            if (e->scale_y >= FRAME_FULL) {
                if (BtlDrawFramePiece(i, g_btl_effect_ox + x + e->curx,
                                      g_btl_effect_oy + y + e->cury, run[i], 1)
                    == 0) {
                    return 0;
                }
            } else {
                if (BtlDrawFramePiece(i, g_btl_effect_ox + x,
                                      g_btl_effect_oy + y, run[i], 0)
                    == 0) {
                    return 0;
                }
            }
            i++;
        } while (i < FRAME_PIECES);
    }

    prim = &e->prim[g_btl_effect_page].flat;
    setPolyF4(prim);
    if (e->scale_y < FRAME_DARK) {
        rgb[0] = FRAME_OPENING;
        rgb[1] = FRAME_OPENING;
        rgb[2] = FRAME_OPENING;
        setSemiTrans(prim, 0);
    } else {
        rgb[0] = 0;
        rgb[1] = 0;
        rgb[2] = FRAME_BLUE;
        setSemiTrans(prim, 1);
    }
    setShadeTex(prim, 0);
    RotTransPers4(&corner[0], &corner[1], &corner[2], &corner[3],
                  (long *)&prim->x0, (long *)&prim->x1, (long *)&prim->x2,
                  (long *)&prim->x3, &i, &i);
    prim->r0 = rgb[0];
    prim->g0 = rgb[1];
    prim->b0 = rgb[2];
    addPrim(g_btl_effect_ot, prim);
    return 1;
}
#else
INCLUDE_ASM("btlp/nonmatchings/effectframe", BtlDrawEffectFrame);
#endif

/* 46.47%. The corners, the shaded quad and its twelve colour bytes are the
   image's; the same staging question as above is most of what is left, and
   the image reads the ordering table's page before the corners are copied
   rather than after. Reading it into a local of its own, and copying the four
   corners as one array, both come out further away. */
#ifdef NON_MATCHING
int BtlEffectCursorBox(BtlEffect *e)
{
    SVECTOR  corner[4];
    POLY_G4 *prim;
    long     i;

    {
        SVECTOR     vec[4];
        BtlFrameVec box[4];

        memset(&box[0], 0, sizeof(box[0]));
        box[0].vz = 0;
        box[0].vx = g_btl_effect_ox;
        box[0].vy = g_btl_effect_oy - FRAME_RISE;
        vec[0] = *(SVECTOR *)&box[0];
        memset(&box[1], 0, sizeof(box[1]));
        box[1].vz = 0;
        box[1].vx = g_btl_effect_ox + e->dx * FRAME_CELL;
        box[1].vy = g_btl_effect_oy - FRAME_RISE;
        vec[1] = *(SVECTOR *)&box[1];
        memset(&box[2], 0, sizeof(box[2]));
        box[2].vx = g_btl_effect_ox;
        box[2].vz = 0;
        box[2].vy = g_btl_effect_oy + (u_short)e->dy * FRAME_CELL - FRAME_RISE;
        vec[2] = *(SVECTOR *)&box[2];
        memset(&box[3], 0, sizeof(box[3]));
        box[3].vx = g_btl_effect_ox + e->dx * FRAME_CELL;
        box[3].vz = 0;
        box[3].vy = g_btl_effect_oy + (u_short)e->dy * FRAME_CELL - FRAME_RISE;
        vec[3] = *(SVECTOR *)&box[3];
        corner[0] = vec[0];
        corner[1] = vec[1];
        corner[2] = vec[2];
        corner[3] = vec[3];
    }

    prim = &e->prim[g_btl_effect_page].shaded;
    setPolyG4(prim);
    setSemiTrans(prim, 1);
    setShadeTex(prim, 0);
    RotTransPers4(&corner[0], &corner[1], &corner[2], &corner[3],
                  (long *)&prim->x0, (long *)&prim->x1, (long *)&prim->x2,
                  (long *)&prim->x3, &i, &i);
    prim->r0 = 0;
    prim->g0 = 0;
    prim->b0 = CURSOR_DIM;
    prim->r1 = 0;
    prim->g1 = CURSOR_MID;
    prim->b1 = CURSOR_BRIGHT;
    prim->r2 = CURSOR_BRIGHT;
    prim->g2 = 0;
    prim->b2 = CURSOR_BRIGHT;
    prim->r3 = 0;
    prim->g3 = 0;
    prim->b3 = CURSOR_DIM;
    addPrim(g_btl_effect_ot, prim);
    return 1;
}
#else
INCLUDE_ASM("btlp/nonmatchings/effectframe", BtlEffectCursorBox);
#endif
