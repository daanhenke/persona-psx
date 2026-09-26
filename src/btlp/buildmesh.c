/* Persona 1 (JP) - laying the battle floor out.  BTLP only.
 *   0x800874F8 BtlBuildMesh
 *
 * Two things get set up here, once per frame buffer. The floor is 20 x 9
 * sixteen-pixel quads, each taking its texture cell from g_btl_mesh_cells -
 * one u and one v per quad, counted in cells rather than pixels. Three hundred
 * more quads follow further into the buffer; they are only prepared here and
 * filled in by the runs of quads that stand around the fight.
 *
 * The 21 x 10 grid of vertices behind the floor is then put flat: every vertex
 * is placed on its own grid position and hung there, which is the rest
 * position BtlWaveMesh displaces the drawn one from.
 *
 * Both the floor and the standing quads are left switched off.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/battle.h>

/* Quads across and down the floor, and the vertices behind them. */
#define MESH_COLS 20
#define MESH_ROWS 9
#define MESH_VX   21
#define MESH_VY   10

/* A quad is sixteen pixels on a side, and a vertex position is that in the
   twenty-bit fixed point the wave works in. */
#define MESH_CELL  0x10
#define MESH_SHIFT 20

/* Where in a frame's primitives the floor sits and where the standing quads
   do, and how far apart the two frames are. */
#define MESH_AT     0xA588
#define MESH_STRIPS 0x76A8
#define MESH_FRAME  0xE660
#define MESH_BUFS   2

/* Standing quads prepared for the runs around the fight. */
#define MESH_STRIP_PRIMS 300

/* The texture page and palette the floor is drawn from live in slot 22 of
   the overlay's two tables. */
#define MESH_SLOT 22

/* Grey leaves the texture untinted. */
#define MESH_GREY 0x80

typedef struct {
    /* 0x00 */ int x;        /* where the vertex is drawn */
    /* 0x04 */ int y;
    /* 0x08 */ int rest_x;   /* what it is displaced from */
    /* 0x0C */ int rest_y;
    /* 0x10 */ int pad10[2];
} BtlMeshVertex;             /* 0x18 bytes */

extern BtlMeshVertex g_btl_mesh[];
extern const char    g_btl_mesh_cells[];
extern u_char        g_btl_arena_show;
extern u_char        g_btl_mesh_show;

/* Each row's two texture pointers are set in blocks of their own. cse1 stops
   at the end of a loop, so it does not see that one table address is the
   other plus one; loop.c then leaves both in the row loop, where cse2 derives
   the first from the second as the image does. Written as plain statements,
   cse1 relates them and loop.c lifts the pair out of the row loop instead.
   The quad's first and second halves are reached through pointers of their
   own, so each stays local to its block. */
void BtlBuildMesh(void)
{
    int            buf;
    BtlMeshVertex *v;
    BtlMeshVertex *rest;
    POLY_FT4      *p;
    POLY_FT4      *q;
    const char    *u;
    const char    *w;
    int            off;
    int            cell;
    int            idx;
    int            col;
    int            row;
    short          x1;
    short          y0;
    short          y1;
    short          yb;

    v = g_btl_mesh;
    buf = 0;
    off = 0;
    do {
        row = 0;
        cell = 0;
        y1 = MESH_CELL;
        g_btl_polyft4_next = (POLY_FT4 *)(g_btl_prim_pool + off + MESH_AT);
        do {
            col = 0;
            y0 = row * MESH_CELL;
            yb = y1;
            x1 = MESH_CELL;
            idx = cell * 2;
            do {
                w = &g_btl_mesh_cells[idx + 1];
            } while (0);
            do {
                u = &g_btl_mesh_cells[idx];
            } while (0);
            do {
                cell++;
                SetPolyFT4(g_btl_polyft4_next);
                SetShadeTex(g_btl_polyft4_next, 0);
                SetSemiTrans(g_btl_polyft4_next, 0);
                p = g_btl_polyft4_next;
                p->x0 = col * MESH_CELL;
                p->y0 = y0;
                p->x1 = x1;
                p->y1 = y0;
                p->x2 = col * MESH_CELL;
                p->y2 = yb;
                p->x3 = x1;
                p->y3 = yb;
                p->u0 = *u * MESH_CELL;
                g_btl_polyft4_next->v0 = *w * MESH_CELL;
                g_btl_polyft4_next->u1 = *u * MESH_CELL + MESH_CELL;
                g_btl_polyft4_next->v1 = *w * MESH_CELL;
                g_btl_polyft4_next->u2 = *u * MESH_CELL;
                g_btl_polyft4_next->v2 = *w * MESH_CELL + MESH_CELL;
                g_btl_polyft4_next->u3 = *u * MESH_CELL + MESH_CELL;
                g_btl_polyft4_next->v3 = *w * MESH_CELL + MESH_CELL;
                q = g_btl_polyft4_next;
                q->tpage = g_btl_tpage[MESH_SLOT];
                col++;
                g_btl_polyft4_next->clut = g_btl_clut[MESH_SLOT];
                q->r0 = MESH_GREY;
                g_btl_polyft4_next->g0 = MESH_GREY;
                x1 += MESH_CELL;
                u += 2;
                g_btl_polyft4_next->b0 = MESH_GREY;
                g_btl_polyft4_next++;
                w += 2;
            } while (col < MESH_COLS);
            row++;
            y1 += MESH_CELL;
        } while (row < MESH_ROWS);

        row = 0;
        g_btl_polyft4_next = (POLY_FT4 *)(g_btl_prim_pool + off + MESH_STRIPS);
        do {
            row++;
            SetPolyFT4(g_btl_polyft4_next);
            SetShadeTex(g_btl_polyft4_next, 0);
            SetSemiTrans(g_btl_polyft4_next, 0);
            g_btl_polyft4_next++;
        } while (row < MESH_STRIP_PRIMS);

        off += MESH_FRAME;
        buf++;
    } while (buf < MESH_BUFS);

    row = 0;
    do {
        col = 0;
        rest = v + 0;
        do {
            v->x = col << MESH_SHIFT;
            col++;
            rest->rest_x = v->x;
            rest->y = row << MESH_SHIFT;
            rest->rest_y = row << MESH_SHIFT;
            rest++;
            v++;
        } while (col < MESH_VX);
        row++;
    } while (row < MESH_VY);

    g_btl_arena_show = 0;
    g_btl_mesh_show = 0;
}

