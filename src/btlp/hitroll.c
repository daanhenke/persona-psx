/* Persona 1 (JP) - how many times a swing lands.  BTLP only.
 *   0x80094E60 BtlRollHits
 *
 * One row of g_btl_hit_rolls per hit rating, eight thresholds a row. A byte is
 * rolled - rand() % 255 - and the answer is the first place in the row whose
 * threshold is above the roll, so a row reads as how likely each count is. A
 * roll no threshold is above answers HIT_ROLL_NONE.
 */
#include <decomp/types.h>
#include <rand.h>

#define HIT_ROLL_STEPS 8
#define HIT_ROLL_RANGE 255
#define HIT_ROLL_NONE  9

extern const u_char g_btl_hit_rolls[][HIT_ROLL_STEPS];

int BtlRollHits(int hits)
{
    u_int roll;
    int   i;

    /* The row indexed in the loop, not taken into a local first: a row
       pointer is worked out ahead of the table's address. */
    roll = (u_char)(rand() % HIT_ROLL_RANGE);
    for (i = 0; i < HIT_ROLL_STEPS; i++) {
        if (roll < g_btl_hit_rolls[hits][i]) {
            return i;
        }
    }
    return HIT_ROLL_NONE;
}
