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
 * kept from the test at the top.
 */
#include <types.h>

#define APPROACH_MAX 0xFF

void BtlApproach(short *cur, const short *target, int step)
{
    int   now;
    int   next;
    short out;

    now = *cur;
    if (now == *target) {
        return;
    }
    if (now < *target) {
        *cur = now + step;
        next = (short)(now + step);
        out = next;
        if (next < 0) {
            out = 0;
        } else if (*target < next) {
            out = *target;
        }
        *cur = out;
    } else {
        *cur = now - step;
        next = (short)(now - step);
        out = *target;
        if (*target <= next) {
            out = next;
            if (next > APPROACH_MAX) {
                out = APPROACH_MAX;
            }
        }
        *cur = out;
    }
}
