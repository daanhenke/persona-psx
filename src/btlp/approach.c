/* Persona 1 (JP) - stepping a value toward another.
 *   BTLP @ 0x800814A0
 *
 * Moves *cur one step toward *target without overshooting it, and holds the
 * result inside 0..0xFF. Both ends are guarded separately: going up it will not
 * pass the target and will not come out below zero, going down it will not pass
 * the target and will not come out above 0xFF. The two clamps are what say the
 * values are byte-sized even though they are kept as shorts.
 *
 * It reads like two macro expansions: no locals at all, *cur written once with
 * the step and once with the clamp, every value read back through the
 * pointer. That is load-bearing. With *cur in an int local the load outlives
 * the compares and takes the wrong register; with the sum in a local the
 * rising path loses its copy. The ternaries stored through the short pointer
 * are also what reserve the twenty-four bytes of frame nothing reads.
 */
#include <decomp/types.h>

#define APPROACH_MAX 0xFF

void BtlApproach(short *cur, const short *target, int step)
{
    if (*cur != *target) {
        if (*cur < *target) {
            *cur = step + *cur;
            *cur = *cur < 0 ? 0 : (*cur > *target ? *target : *cur);
        } else {
            *cur -= step;
            *cur = *cur < *target ? *target
                : (*cur > APPROACH_MAX ? APPROACH_MAX : *cur);
        }
    }
}
