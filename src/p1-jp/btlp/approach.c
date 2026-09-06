/* Persona 1 (JP) - stepping a value toward another.
 *   BTLP @ 0x800814A0
 *
 * Moves *cur one step toward *target without overshooting it, and holds the
 * result inside 0..0xFF. Both ends are guarded separately: going up it will not
 * pass the target and will not come out below zero, going down it will not pass
 * the target and will not come out above 0xFF. The two clamps are what say the
 * values are byte-sized even though they are kept as shorts.
 *
 * *cur is written twice on either path - once with the raw step and again with
 * the clamped result - and *target is read again inside the branch rather than
 * kept from the test at the top. The rising path keeps the untruncated sum as
 * its answer while the falling one keeps the truncated short; each path has its
 * own result variable, and they are not one variable spelt twice.
 */
#include <types.h>

#define APPROACH_MAX 0xFF

void BtlApproach(short *cur, const short *target, int step)
{
    int   now;
    int   sum;
    int   next;
    int   up;
    int   down;

    now = *cur;
    if (now == *target) {
        return;
    }
    if (now < *target) {
        sum = step + now;
        *cur = sum;
        up = sum;
        next = (short)sum;
        if (next < 0) {
            up = 0;
        } else if (*target < next) {
            up = *target;
        }
        *cur = up;
    } else {
        sum = now - step;
        *cur = sum;
        next = (short)sum;
        if (next < *target) {
            down = *target;
        } else {
            if (next > APPROACH_MAX) {
                next = APPROACH_MAX;
            }
            down = next;
        }
        *cur = down;
    }
}
