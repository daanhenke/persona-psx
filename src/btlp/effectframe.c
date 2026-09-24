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

/* The four corners copied as one block, the way the transform's vectors are
   filled. */
typedef struct {
    SVECTOR v[4];
} BtlFrameQuad;

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

/* 97.02%. The levers:
   - the corners set as vx, vy, vz in every box;
   - the four copied into the transform's vectors as one 32-byte block;
   - each piece's run read once into a short, with its corner bits tested as
     (r >> 14) & 1 and multiplied by the stretch plus one;
   - the stretch worked out as an int from the signed width, and narrowed to a
     short where the loop uses it.
   What is left:
   - the prologue saves ra, s2 and s0 before memset's arguments, where gcc
     puts them after;
   - the width and height swap registers with the copy the image makes of the
     height for the loop;
   - the run table's .rodata is D_8006489C in the image and anonymous here. */
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
        box[0].vx = g_btl_effect_ox;
        box[0].vy = g_btl_effect_oy - FRAME_RISE;
        box[0].vz = 0;
        vec[0] = *(SVECTOR *)&box[0];
        memset(&box[1], 0, sizeof(box[1]));
        box[1].vx = g_btl_effect_ox + e->dx * FRAME_CELL;
        box[1].vy = g_btl_effect_oy - FRAME_RISE;
        box[1].vz = 0;
        vec[1] = *(SVECTOR *)&box[1];
        memset(&box[2], 0, sizeof(box[2]));
        box[2].vx = g_btl_effect_ox;
        box[2].vy = g_btl_effect_oy + (u_short)e->dy * FRAME_CELL - FRAME_RISE;
        box[2].vz = 0;
        vec[2] = *(SVECTOR *)&box[2];
        memset(&box[3], 0, sizeof(box[3]));
        box[3].vx = g_btl_effect_ox + e->dx * FRAME_CELL;
        box[3].vy = g_btl_effect_oy + (u_short)e->dy * FRAME_CELL - FRAME_RISE;
        box[3].vz = 0;
        vec[3] = *(SVECTOR *)&box[3];
        *(BtlFrameQuad *)corner = *(BtlFrameQuad *)vec;
    }
    {
        /* Kept in .rodata and copied onto the stack, the way the piece drawer
           keeps its cells. */
        u_short run[FRAME_PIECES] = {
            0x0001, 0x4001, 0x2001, 0x6001, 0x1000, 0x3000, 0x8800, 0xC800,
        };
        int     x;
        int     y;
        short   r;

        across = (short)e->dx - 2;
        if (across < 0) {
            across = 0;
        }
        run[4] = across + run[4];
        run[5] = across + run[5];
        down = (short)e->dy - 2;
        if (down < 0) {
            down = 0;
        }
        i = 0;
        run[6] = down + run[6];
        run[7] = down + run[7];
        do {
            r = run[i];
            x = (((r >> 14) & 1) * ((short)across + 1) + ((r >> 12) & 1)) * FRAME_CELL;
            y = (((r >> 13) & 1) * ((short)down + 1) + ((r >> 11) & 1)) * FRAME_CELL;
            if (e->scale_y >= FRAME_FULL) {
                if (BtlDrawFramePiece(i, g_btl_effect_ox + x + e->curx,
                                      g_btl_effect_oy + y + e->cury, r, 1)
                    == 0) {
                    return 0;
                }
            } else {
                if (BtlDrawFramePiece(i, g_btl_effect_ox + x,
                                      g_btl_effect_oy + y, r, 0)
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

/* 99.35%. The quad is reached through e->prim[g_btl_effect_page] at every
   use, as the image re-reads the page each time. The corners are set as vx,
   vy, vz and copied as one block, as in the frame above. What is left is the
   prologue: the image saves ra straight after s0, and gcc here schedules the
   save after memset's first two arguments. */
#define CURSOR_QUAD (&e->prim[g_btl_effect_page].shaded)

#ifdef NON_MATCHING
int BtlEffectCursorBox(BtlEffect *e)
{
    SVECTOR  corner[4];
    long     i;

    {
        SVECTOR     vec[4];
        BtlFrameVec box[4];

        memset(&box[0], 0, sizeof(box[0]));
        box[0].vx = g_btl_effect_ox;
        box[0].vy = g_btl_effect_oy - FRAME_RISE;
        box[0].vz = 0;
        vec[0] = *(SVECTOR *)&box[0];
        memset(&box[1], 0, sizeof(box[1]));
        box[1].vx = g_btl_effect_ox + e->dx * FRAME_CELL;
        box[1].vy = g_btl_effect_oy - FRAME_RISE;
        box[1].vz = 0;
        vec[1] = *(SVECTOR *)&box[1];
        memset(&box[2], 0, sizeof(box[2]));
        box[2].vx = g_btl_effect_ox;
        box[2].vy = g_btl_effect_oy + (u_short)e->dy * FRAME_CELL - FRAME_RISE;
        box[2].vz = 0;
        vec[2] = *(SVECTOR *)&box[2];
        memset(&box[3], 0, sizeof(box[3]));
        box[3].vx = g_btl_effect_ox + e->dx * FRAME_CELL;
        box[3].vy = g_btl_effect_oy + (u_short)e->dy * FRAME_CELL - FRAME_RISE;
        box[3].vz = 0;
        vec[3] = *(SVECTOR *)&box[3];
        *(BtlFrameQuad *)corner = *(BtlFrameQuad *)vec;
    }

    setPolyG4(CURSOR_QUAD);
    setSemiTrans(CURSOR_QUAD, 1);
    setShadeTex(CURSOR_QUAD, 0);
    RotTransPers4(&corner[0], &corner[1], &corner[2], &corner[3],
                  (long *)&CURSOR_QUAD->x0, (long *)&CURSOR_QUAD->x1, (long *)&CURSOR_QUAD->x2,
                  (long *)&CURSOR_QUAD->x3, &i, &i);
    CURSOR_QUAD->r0 = 0;
    CURSOR_QUAD->g0 = 0;
    CURSOR_QUAD->b0 = CURSOR_DIM;
    CURSOR_QUAD->r1 = 0;
    CURSOR_QUAD->g1 = CURSOR_MID;
    CURSOR_QUAD->b1 = CURSOR_BRIGHT;
    CURSOR_QUAD->r2 = CURSOR_BRIGHT;
    CURSOR_QUAD->g2 = 0;
    CURSOR_QUAD->b2 = CURSOR_BRIGHT;
    CURSOR_QUAD->r3 = 0;
    CURSOR_QUAD->g3 = 0;
    CURSOR_QUAD->b3 = CURSOR_DIM;
    addPrim(g_btl_effect_ot, CURSOR_QUAD);
    return 1;
}
#else
INCLUDE_ASM("btlp/nonmatchings/effectframe", BtlEffectCursorBox);
#endif
