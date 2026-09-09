/* Persona 1 (JP) - one piece of the box an effect draws itself in.  BTLP only.
 *   0x8007822C BtlDrawFramePiece
 *
 * The frame is built out of eight 8x8 cells - four corners and four edges -
 * and a piece is a run of one of them. The run's length is the low byte of the
 * descriptor and its sign bit says which way it goes: clear runs across, set
 * runs down. BtlDrawEffectFrame walks the eight descriptors and hands each one
 * here with the corner it hangs off already added in.
 *
 * A flat piece goes out as an SPRT_8. Otherwise the four corners are laid out
 * in the scratchpad and put through RotTransPers4, so the frame can sit in
 * perspective with the effect it belongs to, and the piece goes out as a
 * POLY_FT4. Either way the primitive is written at g_btl_prim_next and linked
 * into the ordering table BtlDrawEffects left in g_btl_effect_ot.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <decomp/libc.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/battle.h>

/* The primitives are staged in the scratchpad and copied into the buffer, so
   only one of each is ever needed: the four corners first, then the quad whose
   screen coordinates RotTransPers4 writes into it, then the sprite. */
/* The two coordinates and the transform out-parameters share one scratch
   block on the stack. */
typedef struct {
    u_short vx;
    u_short vy;
    u_short vz;
    u_short pad;
} BtlFrameVec;

typedef struct {
    /* 0x00 */ SVECTOR  corner[4];
    /* 0x20 */ POLY_FT4 quad;
    /* 0x48 */ SPRT_8   sprite;
} BtlFramePad;

#define FRAME_PAD ((BtlFramePad *)0x1F800000)

/* Raw-texture forms of POLY_FT4 and SPRT_8: the frame is drawn at its own
   colours, not tinted by the primitive's. */
#define FRAME_QUAD_LEN    9
#define FRAME_QUAD_CODE   0x2D
#define FRAME_SPRITE_LEN  3
#define FRAME_SPRITE_CODE 0x75

/* Where the frame's cells live in VRAM, and the palette slot the loader put
   their CLUT in. */
#define FRAME_TP   0
#define FRAME_ABR  0
#define FRAME_VX   0x3C0
#define FRAME_VY   0x100
#define FRAME_CLUT 37

/* A cell is eight pixels square, and the whole frame is drawn two scanlines
   above where it is asked for. */
#define FRAME_CELL 8
#define FRAME_RISE 2

/* How many cells a piece is, and which way it runs. */
#define FRAME_RUN_LEN 0xFF

extern char    *g_btl_prim_next;
extern u_long  *g_btl_effect_ot;

#ifdef NON_MATCHING
int BtlDrawFramePiece(int piece, short x, short y, short run, short flat)
{
    /* Kept in .rodata and copied onto the stack: u at +0 of each entry and v
       at +2, in the order BtlDrawEffectFrame walks them. */
    u_char cell[8][4] = {
        { 0x00, 0x00, 0x60, 0x00 },
        { 0x70, 0x00, 0x60, 0x00 },
        { 0x00, 0x00, 0x68, 0x00 },
        { 0x70, 0x00, 0x68, 0x00 },
        { 0x08, 0x00, 0x60, 0x00 },
        { 0x08, 0x00, 0x68, 0x00 },
        { 0x90, 0x00, 0x60, 0x00 },
        { 0x98, 0x00, 0x60, 0x00 },
    };
    BtlFramePad *pad;
    BtlFrameVec  scratch[5];
    int          across;
    int          down;
    int          i;
    u_int        dir;
    int          px;
    int          py;

    /* The descriptor's sign bit picks the direction, so one of the two step
       counts is one and the other zero. */
    dir = run;
    across = (dir >> 15 ^ 1) & 1;
    down = 1 - across;
    pad = FRAME_PAD;
    scratch[1].vx = x;
    scratch[2].vx = y;

    if (flat != 0) {
        int col;
        int row;

        setlen(&pad->sprite, FRAME_SPRITE_LEN);
        setcode(&pad->sprite, FRAME_SPRITE_CODE);
        pad->sprite.clut = g_btl_clut[FRAME_CLUT];
        row = 0;
        col = 0;
        for (i = 0; i < (run & FRAME_RUN_LEN); i++) {
            pad->sprite.x0 = scratch[1].vx + col * FRAME_CELL;
            pad->sprite.y0 = scratch[2].vx + row * FRAME_CELL - FRAME_RISE;
            pad->sprite.u0 = cell[piece][0];
            pad->sprite.v0 = cell[piece][2];
            memcpy(g_btl_prim_next, &pad->sprite, sizeof(SPRT_8));
            addPrim(g_btl_effect_ot, g_btl_prim_next);
            g_btl_prim_next += sizeof(SPRT_8);
            col += across;
            row += down;
        }
    } else {
        int col;
        int row;

        setlen(&pad->quad, FRAME_QUAD_LEN);
        setcode(&pad->quad, FRAME_QUAD_CODE);
        pad->quad.tpage = getTPage(FRAME_TP, FRAME_ABR, FRAME_VX, FRAME_VY);
        pad->quad.clut = g_btl_clut[FRAME_CLUT];
        row = 0;
        col = 0;
        for (i = 0; i < (run & FRAME_RUN_LEN); i++) {
            px = scratch[1].vx + col * FRAME_CELL;
            py = scratch[2].vx + row * FRAME_CELL;
            /* The transform's two out-parameters are never read, so the
               counter shares their storage. */
            for ((*(long *)&scratch[0]) = 0; (*(long *)&scratch[0]) < 4; (*(long *)&scratch[0])++) {
                pad->corner[(*(long *)&scratch[0])].vx =
                    px + (*(long *)&scratch[0]) % 2 * FRAME_CELL;
                pad->corner[(*(long *)&scratch[0])].vy =
                    py + (*(long *)&scratch[0]) / 2 * FRAME_CELL - FRAME_RISE;
                pad->corner[(*(long *)&scratch[0])].vz = 0;
            }
            RotTransPers4(&pad->corner[0], &pad->corner[1],
                          &pad->corner[2], &pad->corner[3],
                          (long *)&pad->quad.x0, (long *)&pad->quad.x1,
                          (long *)&pad->quad.x2, (long *)&pad->quad.x3,
                          (long *)scratch, (long *)scratch);
            pad->quad.u0 = cell[piece][0];
            pad->quad.v0 = cell[piece][2];
            pad->quad.u1 = cell[piece][0] + FRAME_CELL;
            pad->quad.v1 = cell[piece][2];
            pad->quad.u2 = cell[piece][0];
            pad->quad.v2 = cell[piece][2] + FRAME_CELL;
            pad->quad.u3 = cell[piece][0] + FRAME_CELL;
            pad->quad.v3 = cell[piece][2] + FRAME_CELL;
            memcpy(g_btl_prim_next, &pad->quad, sizeof(POLY_FT4));
            addPrim(g_btl_effect_ot, g_btl_prim_next);
            g_btl_prim_next += sizeof(POLY_FT4);
            col += across;
            row += down;
        }
    }
    return 1;
}
#else
INCLUDE_ASM("btlp/nonmatchings/framepiece", BtlDrawFramePiece);
#endif

