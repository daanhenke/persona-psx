/* Persona 1 (JP) - the battle's face window.  BTLP only.
 *   0x80076768 BtlFaceGrow   0x80076ABC BtlFaceOpen
 *   0x800767DC BtlFaceShrink 0x80076AE8 BtlFaceClose
 *
 * The portrait of whoever is taking their turn. BtlFaceLoad reads it off the
 * disc and builds a quad for it; this is the animation that brings that quad
 * on and takes it away, and like the message box it is entirely a matter of
 * moving the scale the quad's matrix is built from.
 *
 * Callers open it at (0x3C, 0x70) as a character's turn comes round and close
 * it with the command panel. Both steps report whether they are still going,
 * which is how BtlFaceDraw knows when to move the animation on.
 *
 * The third component of the scale is only written when the animation reaches
 * its end. Nothing reads it in between - the quad is flat, so its depth scale
 * only has to be right once the picture is standing still.
 */
#include <types.h>
#include <psyq/libgte.h>

/* How much of the way the portrait travels in one frame. */
#define BTL_FACE_STEP 0x200

/* g_btl_face_step */
#define BTL_FACE_GONE      0
#define BTL_FACE_GROWING   1
#define BTL_FACE_OPEN      2
#define BTL_FACE_SHRINKING 3

extern u_char g_btl_fast_anim;

extern int    g_btl_face_step;
extern long   g_btl_face_scale[];
extern short  g_btl_face_x;
extern short  g_btl_face_y;
extern short  g_btl_face_scale_to;

/* Both steps reach the scale through indices held in variables
   rather than writing s[0] and s[1], and both wrap the body in a test of one.
   That looks redundant and is not: with constant indices gcc folds the whole
   address into each store and never uses the base it just built. Leave it. */
int BtlFaceGrow(void)
{
    long   *s;
    u_char  x;
    int     y;
    int     to;
    int     n;

    if (g_btl_fast_anim != 0) {
        g_btl_face_scale[0] = g_btl_face_scale_to;
        g_btl_face_scale[1] = g_btl_face_scale_to;
    }
    s = g_btl_face_scale;
    n = s[0] + BTL_FACE_STEP;
    x = 0;
    to = g_btl_face_scale_to;
    y = 1;
    if (y) {
        s[x] = n;
        s[y] = n;
        if (n < to) {
            return y;
        }
        s[x] = to;
    }
    s[y] = to;
    g_btl_face_scale[2] = to;
    return x;
}

int BtlFaceShrink(void)
{
    long *s;
    int   y;
    int   step;
    int   n;

    if (g_btl_fast_anim != 0) {
        g_btl_face_scale[0] = 0;
        g_btl_face_scale[1] = 0;
    }
    s = g_btl_face_scale;
    step = BTL_FACE_STEP;
    n = s[0] - step;
    y = 1;
    if (y) {
        s[0] = n;
        s[y] = n;
        if (n >= 0) {
            return y;
        }
        n = 0;
        s[n] = n;
    }
    s[y] = n;
    g_btl_face_scale[2] = n;
    return n;
}

void BtlFaceOpen(short x, short y, short scale)
{
    g_btl_face_step = BTL_FACE_GROWING;
    g_btl_face_x = x;
    g_btl_face_y = y;
    g_btl_face_scale_to = scale;
}

void BtlFaceClose(void)
{
    g_btl_face_step = BTL_FACE_SHRINKING;
}
