/* Persona 1 (JP) - a mesh of textured quads into the ordering table.
 *   BTLP @ 0x8006643C BtlDrawMeshFT4
 *
 * A mesh is a count and then that many quads. Each quad carries its own
 * primitives - more than one, so the same geometry can be drawn in more than
 * one style - and the four corner vectors after them; `slot` picks which
 * primitive of the quad this pass writes into.
 *
 * RotAverage4 both projects the four corners into that primitive and answers
 * the average depth, which becomes the ordering-table row once it is shifted
 * down. A larger `shift` puts the mesh nearer the front.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>

/* Primitives a quad carries. */
#define MESH_PRIMS 2

/* What the average depth is shifted by before it indexes the table, less
   whatever the caller asks for. */
#define MESH_OTZ_SHIFT 0xE

typedef struct {
    /* 0x00 */ POLY_FT4 prim[MESH_PRIMS];
    /* 0x50 */ SVECTOR  v[4];
} BtlMeshQuad;                            /* 0x70 bytes */

typedef struct {
    /* 0x0 */ int         count;
    /* 0x4 */ BtlMeshQuad quad[1];
} BtlMesh;

void BtlDrawMeshFT4(BtlMesh *mesh, u_long *ot, int shift, int slot)
{
    BtlMeshQuad *q;
    POLY_FT4    *prim;
    /* RotAverage4 wants somewhere to leave the interpolation value and the
       flag; the two after them are never looked at, and the frame is the wrong
       size without them. */
    long         out[4];
    int          otz;
    int          i;
    int          sh;
    int          off;
    int          n;

    q = mesh->quad;
    /* The counter is zeroed before the test, not inside it: that is what lets
       the quad pointer be worked out up with the register saves instead of
       filling the test's delay slot. */
    i = 0;
    n = mesh->count;
    if (n > 0) {
        off = slot * sizeof(POLY_FT4);
        sh = MESH_OTZ_SHIFT - shift;
        do {
            prim = (POLY_FT4 *)((char *)q + off);
            otz = RotAverage4(&q->v[0], &q->v[1], &q->v[2], &q->v[3],
                              (long *)&prim->x0, (long *)&prim->x1,
                              (long *)&prim->x2, (long *)&prim->x3, &out[0], &out[1]);
            AddPrim(&ot[otz >> sh], prim);
            i++;
            q++;
        } while (i < n);
    }
}
