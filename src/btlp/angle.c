/* Persona 1 (JP) - the angle from one point to another.  BTLP only.
 *   0x8008D680 BtlAngleTo
 *
 * What a record's `angle` is set from when it is turned toward something: the
 * two offsets are folded into one of four quadrants by which is the larger,
 * the smaller over the larger is taken in 64ths, and that ratio is looked up
 * in g_btl_atan to give an angle in the wave tables' 512ths. No offset at all
 * answers 0.
 */
#include <decomp/types.h>

/* The ratio's scale, and where each quadrant starts. */
#define ANGLE_SHIFT 6
#define ANGLE_RIGHT 0x800
#define ANGLE_LEFT  0x180
#define ANGLE_DOWN  0x100
#define ANGLE_MASK  0x1FF

extern u_short g_btl_atan[];

int BtlAngleTo(int dx, int dy)
{
    int ax;
    int ay;
    int a;

    if ((dx | dy) == 0) {
        return 0;
    }
    ax = dx < 0 ? -dx : dx;
    ay = dy < 0 ? -dy : dy;
    if (ax >= ay) {
        if (dx >= 0) {
            a = (dy << ANGLE_SHIFT) / ax + ANGLE_RIGHT;
        } else {
            a = ANGLE_LEFT - (dy << ANGLE_SHIFT) / ax;
        }
    } else {
        if (dy >= 0) {
            a = ANGLE_DOWN - (dx << ANGLE_SHIFT) / ay;
        } else {
            a = (dx << ANGLE_SHIFT) / ay;
        }
    }
    a &= ANGLE_MASK;
    return g_btl_atan[a];
}
