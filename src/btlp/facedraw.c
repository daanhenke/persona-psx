/* Persona 1 (JP) - the portrait quad, once a frame.  BTLP only.
 *   0x80076844 BtlFaceDraw
 *
 * Moves the face window's animation on a step and then draws it: a flat quad
 * 88 by 96 about the origin, put through a matrix built from the scale the
 * animation is moving, and handed to the ordering table as one textured quad.
 *
 * The step is read through a mask because the high bit of g_btl_face_step is
 * used for something else. Growing and shrinking each report whether they are
 * still going: growing settles into BTL_FACE_OPEN, shrinking - and anything
 * the step should never hold - back to BTL_FACE_GONE. Nothing is drawn while
 * the step is zero, which is what takes the portrait away.
 *
 * The projection offset is the battle's own for the length of the quad and
 * then put back, so whoever called this gets the offset it had. Both of
 * RotTransPers4's out-parameters are handed the same word: the quad is flat,
 * so its depth and its flag are read by nobody.
 *
 * One primitive per side, and the caller hands over which side it is - the
 * two frames in flight each own their own copy.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/battle.h>

/* g_btl_face_step, which is read through a mask. */
#define BTL_FACE_GONE      0
#define BTL_FACE_GROWING   1
#define BTL_FACE_OPEN      2
#define BTL_FACE_SHRINKING 3
#define BTL_FACE_STEP_MASK 0x7FFF

/* How far in front of the camera the quad stands, and how big it is. */
#define FACE_Z 100
#define FACE_W 44
#define FACE_H 48

extern int   g_btl_face_step;
extern VECTOR g_btl_face_scale;
extern short g_btl_face_x;
extern short g_btl_face_y;

extern POLY_FT4 g_btl_face_prim[];

extern int BtlFaceGrow(void);
extern int BtlFaceShrink(void);

void BtlFaceDraw(int side, u_long *ot)
{
    MATRIX  m;
    SVECTOR rot   = { 0, 0, 0, 0 };
    VECTOR  trans = { 0, 0, FACE_Z, 0 };
    SVECTOR quad[4] = {
        { -FACE_W, -FACE_H, 0, 0 },
        {  FACE_W, -FACE_H, 0, 0 },
        { -FACE_W,  FACE_H, 0, 0 },
        {  FACE_W,  FACE_H, 0, 0 }
    };
    long ofx;
    long ofy;
    long otz;

    switch (g_btl_face_step & BTL_FACE_STEP_MASK) {
    case BTL_FACE_GROWING:
        if (BtlFaceGrow() == 0) {
            g_btl_face_step = BTL_FACE_OPEN;
        }
        break;
    case BTL_FACE_OPEN:
        break;
    case BTL_FACE_SHRINKING:
        if (BtlFaceShrink() != 0) {
            break;
        }
        /* Falls through: a shrink that has finished is gone. */
    default:
        g_btl_face_step = BTL_FACE_GONE;
        break;
    }

    if (g_btl_face_step != BTL_FACE_GONE) {
        ReadGeomOffset(&ofx, &ofy);
        SetGeomOffset(g_btl_face_x, g_btl_face_y);
        RotMatrix(&rot, &m);
        TransMatrix(&m, &trans);
        ScaleMatrix(&m, &g_btl_face_scale);
        SetRotMatrix(&m);
        SetTransMatrix(&m);
        RotTransPers4(&quad[0], &quad[1], &quad[2], &quad[3],
                      (long *)&g_btl_face_prim[side].x0,
                      (long *)&g_btl_face_prim[side].x1,
                      (long *)&g_btl_face_prim[side].x2,
                      (long *)&g_btl_face_prim[side].x3, &otz, &otz);
        AddPrim(ot, &g_btl_face_prim[side]);
        SetGeomOffset(ofx, ofy);
    }
}
