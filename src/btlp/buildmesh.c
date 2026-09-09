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
extern u_short       g_btl_tpage[];
extern u_char        g_btl_arena_show;
extern u_char        g_btl_mesh_show;

#ifdef NON_MATCHING
void BtlBuildMesh(void)
{
    BtlMeshVertex *v;
    BtlMeshVertex *rest;
    POLY_FT4      *p;
    const char    *u;
    const char    *w;
    int            buf;
    int            off;
    int            cell;
    int            col;
    int            row;
    int            i;
    short          x1;
    short          y0;
    short          y1;

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
            x1 = MESH_CELL;
            w = &g_btl_mesh_cells[cell * 2 + 1];
            u = w - 1;
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
                p->y2 = y1;
                p->x3 = x1;
                p->y3 = y1;
                p->u0 = *u * MESH_CELL;
                g_btl_polyft4_next->v0 = *w * MESH_CELL;
                g_btl_polyft4_next->u1 = *u * MESH_CELL + MESH_CELL;
                g_btl_polyft4_next->v1 = *w * MESH_CELL;
                g_btl_polyft4_next->u2 = *u * MESH_CELL;
                g_btl_polyft4_next->v2 = *w * MESH_CELL + MESH_CELL;
                g_btl_polyft4_next->u3 = *u * MESH_CELL + MESH_CELL;
                g_btl_polyft4_next->v3 = *w * MESH_CELL + MESH_CELL;
                p = g_btl_polyft4_next;
                /* Both the palette and the page go through the same variable
                   the strip loop counts with, and the palette is read out
                   first; taking either straight from its table where it is
                   used costs the match. */
                i = g_btl_clut[MESH_SLOT];
                p->clut = i;
                col++;
                i = g_btl_tpage[MESH_SLOT];
                g_btl_polyft4_next->tpage = i;
                p->r0 = MESH_GREY;
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

        i = 0;
        g_btl_polyft4_next = (POLY_FT4 *)(g_btl_prim_pool + off + MESH_STRIPS);
        do {
            i++;
            SetPolyFT4(g_btl_polyft4_next);
            SetShadeTex(g_btl_polyft4_next, 0);
            SetSemiTrans(g_btl_polyft4_next, 0);
            g_btl_polyft4_next++;
        } while (i < MESH_STRIP_PRIMS);

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
#else
INCLUDE_ASM("btlp/nonmatchings/buildmesh", BtlBuildMesh);
#endif

