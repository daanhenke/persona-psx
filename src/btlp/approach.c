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
#include <decomp/types.h>
#include <decomp/include_asm.h>

#define APPROACH_MAX 0xFF

#ifdef NON_MATCHING
void BtlApproach(short *cur, const short *target, int step)
{
    int unused[2];
    int now;
    int up;
    int sum;

    now = *cur;
    up = now;
    if (now != *target) {
        if (now < *target) {
            sum = step + up;
            *cur = sum;
            up = sum;
            if (*cur >= 0) {
                if (*target < *cur) {
                    up = *target;
                }
            } else {
                up = 0;
            }
            *cur = up;
        } else {
            *cur = up - step;
            *cur = *cur < *target ? *target
                : (*cur > APPROACH_MAX ? APPROACH_MAX : *cur);
        }
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/approach", BtlApproach);
#endif
