/* Persona 1 (JP) - what is still owed to reach a level.
 *
 *   ADV @ 0x8007D90C   DNG @ 0x8008BEA8   S2D @ 0x8007C30C
 *
 * The curve holds experience per level rather than running totals, so getting
 * to a level means adding up everything below it. Its successive differences
 * are the perfect squares from 3 upwards, which is what gives the totals their
 * shape.
 *
 * Two curves are carried and an event flag picks between them - and they hold
 * the same 98 words, so which one is summed cannot change the answer.
 */
#include <types.h>

/* The event flag that picks the second curve. */
#define EXP_ALT_CURVE 0x21

extern const int g_exp_curve[];
extern const int g_exp_curve_alt[];

extern u_char EventFlagGet2(int flag);

int ExpToLevel(u_char level, int unused, int have)
{
    int i;
    int total;

    /* The flag lands in the accumulator, so the ordinary curve's loop starts
       from a total that is already zero. */
    total = EventFlagGet2(EXP_ALT_CURVE);
    if (total != 0) {
        i = 0;
        total = 0;
        for (; i < level; i++) {
            total += g_exp_curve_alt[i];
        }
        return total - have;
    }
    i = 0;
    for (; i < level; i++) {
        total += g_exp_curve[i];
    }
    return total - have;
}
