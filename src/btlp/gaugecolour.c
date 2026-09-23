/* Persona 1 (JP) - what colour a fighter's two bars are drawn in.  BTLP only.
 *   0x800AD924 BtlSetGaugeColour
 *
 * Three colours by how much is left, worked out the same way for HP and for SP:
 * full when the value is at the maximum, low when it is down to a quarter of it
 * or less, and the ordinary one in between. The quarter is a signed divide, so
 * the shift comes with the bias that makes it round toward zero.
 *
 * Either bar may be left out by handing in nothing for it. Every caller but the
 * Persona board asks for both; that one has only an SP gauge to colour.
 *
 * The colour lands on the cell's clut byte, which is the same field the text
 * rows carry their own colour in.
 *
 * 91.62% and behind INCLUDE_ASM. Nothing structural is left: the image keeps the
 * colour in the register the comparison was worked out in, and every shape tried
 * puts it in a spare argument register instead, which costs the delay slot the
 * low colour would otherwise fill. A colour of its own for each bar is the
 * closest; sharing one is worse, block-scoping them changes nothing, and writing
 * the test into the colour itself - the reuse direction of one-variable-or-two -
 * is worse again. It is a register question, so the permuter is the next move.
 *
 * Every if/else shape with the low colour in an arm of its own (low first, low
 * in the else, the ternary, the nested else-if) gets the registers exactly
 * right - the colour in the compare's register - but leaves the compare's
 * branch slot empty, where the image has stolen the low colour out of the
 * else arm into it and dropped the jump around. So the source is one of those;
 * what is not found is why reorg will steal from the arm there and not here.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/common/char.h>
#include <persona/btlp/board.h>

#ifdef NON_MATCHING
void BtlSetGaugeColour(const Char *c, u_char *hp, u_char *sp)
{
    int hp_colour;
    int sp_colour;

    if (hp != 0) {
        hp_colour = GAUGE_LOW;
        if (c->hp_max / GAUGE_LOW_AT < c->hp) {
            hp_colour = GAUGE_OK;
            if (c->hp == c->hp_max) {
                hp_colour = GAUGE_FULL;
            }
        }
        hp[GAUGE_CLUT_AT] = hp_colour;
    }
    if (sp != 0) {
        sp_colour = GAUGE_LOW;
        if (c->sp_max / GAUGE_LOW_AT < c->sp) {
            sp_colour = GAUGE_OK;
            if (c->sp == c->sp_max) {
                sp_colour = GAUGE_FULL;
            }
        }
        sp[GAUGE_CLUT_AT] = sp_colour;
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/gaugecolour", BtlSetGaugeColour);
#endif
