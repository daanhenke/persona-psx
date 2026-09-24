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
 * Each arm stores its own colour. gcc merges the three stores into one and
 * leaves each arm only the colour to set, which is how the low colour ends up
 * in the compare's delay slot in the image. With the colour held in a local
 * and stored once, it cannot get there.
 */
#include <decomp/types.h>
#include <persona/common/char.h>
#include <persona/btlp/board.h>

void BtlSetGaugeColour(const Char *c, u_char *hp, u_char *sp)
{
    if (hp != 0) {
        if (c->hp <= c->hp_max / GAUGE_LOW_AT) {
            hp[GAUGE_CLUT_AT] = GAUGE_LOW;
        } else if (c->hp == c->hp_max) {
            hp[GAUGE_CLUT_AT] = GAUGE_FULL;
        } else {
            hp[GAUGE_CLUT_AT] = GAUGE_OK;
        }
    }
    if (sp != 0) {
        if (c->sp <= c->sp_max / GAUGE_LOW_AT) {
            sp[GAUGE_CLUT_AT] = GAUGE_LOW;
        } else if (c->sp == c->sp_max) {
            sp[GAUGE_CLUT_AT] = GAUGE_FULL;
        } else {
            sp[GAUGE_CLUT_AT] = GAUGE_OK;
        }
    }
}
