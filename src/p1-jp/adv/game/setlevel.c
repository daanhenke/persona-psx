/* Persona 1 (JP) - putting a character's experience where its level says.
 *   ADV @ 0x800B02E4
 *
 * Run after a level change - the entry code sets the hero up at level 5 this
 * way, and the growth routine calls it again every time somebody gains one.
 * The curve holds experience per level rather than running totals, so the
 * amount a level starts at is everything below it added up, and what the next
 * one costs is the curve's own entry. At level 99 there is no next one.
 *
 * Two curves are carried and an event flag picks between them; they hold the
 * same 98 words, so which one is summed cannot change the answer.
 */
#include <types.h>
#include <persona/common/char.h>

/* The event flag that picks the second curve. */
#define EXP_ALT_CURVE 0x21

/* The last level, where nothing is owed for the next one. */
#define LEVEL_MAX 99

extern const int g_exp_curve[];
extern const int g_exp_curve_alt[];

extern int EventFlagGet(int flag);

void CharSetLevelExp(u_char level, u_char slot)
{
    Char *c;
    int   i;
    int   total;

    c = &g_chars[slot];
    /* The flag lands in the accumulator, so the ordinary curve's loop starts
       from a total that is already zero. */
    total = EventFlagGet(EXP_ALT_CURVE);
    if (total != 0) {
        i = 0;
        total = 0;
        for (; i < level - 1; i++) {
            total += g_exp_curve_alt[i];
        }
        c->unk18 = g_exp_curve_alt[level - 1];
    } else {
        i = 0;
        for (; i < level - 1; i++) {
            total += g_exp_curve[i];
        }
        c->unk18 = g_exp_curve[level - 1];
    }
    if (level == LEVEL_MAX) {
        c->unk18 = 0;
    }
    c->unk10 = total;
}
