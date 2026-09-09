/* Persona 1 (JP) - one RMD model, transformed and sorted.  BTLP only.
 *   0x80066524 BtlDrawRmdFT4
 *
 * The whole of a model's setup in one call: the rotation, translation and
 * scale are built into a matrix of its own, handed to the GTE, and the model's
 * textured quads are rotated and sorted into the ordering table. The matrix
 * stack is pushed and popped around it, so whatever the caller had set up
 * comes back afterwards.
 *
 * All four clipping arguments are zero, which is the whole model drawn with no
 * near or far rejection.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>

/* No screen, near or far clipping, and the ordinary normal-clip rule. */
#define RMD_NO_CLIP 0

void BtlDrawRmdFT4(VECTOR *trans, SVECTOR *rot, VECTOR *scale, long *pa,
                   u_long *ot, int otlen, int id)
{
    MATRIX m;

    PushMatrix();
    RotMatrix(rot, &m);
    TransMatrix(&m, trans);
    ScaleMatrix(&m, scale);
    SetRotMatrix(&m);
    SetTransMatrix(&m);
    RotRMD_FT4(pa, ot, otlen, id, RMD_NO_CLIP, RMD_NO_CLIP, RMD_NO_CLIP,
               RMD_NO_CLIP);
    PopMatrix();
}
