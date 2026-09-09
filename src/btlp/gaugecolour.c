/* Persona 1 (JP) - what colour an HP or SP bar is drawn in.
 *   BTLP @ 0x800AD924 BtlSetGaugeColour
 *
 * The same quarter-of-maximum threshold the field's status HUD uses for its
 * danger colour. Either sprite may be null: the caller draws only the gauges
 * it has.
 *
 * A unit of its own that does not come out of the C yet, so the overlay still
 * takes this stretch from asm and this file is not wired.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/common/char.h>

/* Tints, two apart, picked by how full the gauge is. */
#define GAUGE_FULL   0x24
#define GAUGE_NORMAL 0x20
#define GAUGE_LOW    0x22

/* Only the tint byte matters here; it sits at +9 of whatever draws the bar. */
typedef struct {
    /* 0x0 */ u_char pad00[9];
    /* 0x9 */ u_char tint;
} BtlGaugeSprite;

#ifdef NON_MATCHING
void BtlSetGaugeColour(const Char *c, BtlGaugeSprite *hp,
                       BtlGaugeSprite *sp)
{
    int max;
    u_char tint;
    u_char low;

    if (hp != 0) {
        /* The do/while(0) is load-bearing: it gives the block a boundary of
           its own, which is where the original keeps the tint. */
        do {
            max = c->hp_max;
            low = max / 4 < c->hp;
            tint = GAUGE_LOW;
            if (low) {
                tint = GAUGE_NORMAL;
                if (c->hp == max) {
                    tint = GAUGE_FULL;
                }
            }
        } while (0);
        hp->tint = tint;
    }
    if (sp != 0) {
        max = c->sp_max;
        low = max / 4 < c->sp;
        tint = GAUGE_LOW;
        if (low) {
            tint = GAUGE_NORMAL;
            if (c->sp == max) {
                tint = GAUGE_FULL;
            }
        }
        sp->tint = tint;
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/gaugecolour", BtlSetGaugeColour);
#endif
