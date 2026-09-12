/* Persona 1 (JP) - the breathing on whatever is being aimed at.  BTLP only.
 *   0x8008B204 BtlPulsePicked
 *
 * Called once a frame on every record of the marker group. A record that is
 * held is left alone; anything else is walked between full brightness and a
 * sixteenth of it, back and forth, with BTL_OBJ_PICKED doing double duty as
 * which way it is going: it is set as the record reaches the top and cleared
 * as it reaches the bottom, and the walk is aimed at whichever end it is not
 * standing on.
 *
 * The rate doubles when the field is being drawn every other frame, so the
 * breathing takes the same time either way.
 */
#include <decomp/include_asm.h>
#include <decomp/types.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>

/* The two ends of the walk, as the pair of shorts the record keeps the first
   two channels in and on their own. */
#define PULSE_LIT      0xFF
#define PULSE_DIM      0x10
#define PULSE_LIT_PAIR 0x00FF00FF
#define PULSE_DIM_PAIR 0x00100010

/* How much of the gap is closed each frame, and what that becomes when only
   every other field is drawn. */
#define PULSE_RATE 0x10
#define PULSE_FAST 0x20

/* Forty-three of the forty-eight instructions. What is left is the rate: the
   image keeps it in a saved argument register alongside the walked-to colour
   and sets it in each arm, and every spelling tried here - the rate as a local
   set before the half-rate flag or after it, the flag read at the test
   instead, the fade written in each arm of the test - leaves the two in each
   other's registers. */
#ifdef NON_MATCHING
void BtlPulsePicked(BtlObj *o)
{
    int rate;
    int half;
    int to;

    if ((o->attr & BTL_OBJ_HELD) != 0) {
        return;
    }
    if ((o->attr & BTL_OBJ_PICKED) != 0) {
        if (*(long *)&o->rgb[0] == PULSE_DIM_PAIR && o->rgb[2] == PULSE_DIM) {
            o->attr &= ~BTL_OBJ_PICKED;
            return;
        }
        half = g_btl_half_rate;
        to = PULSE_DIM;
    } else {
        if (*(long *)&o->rgb[0] == PULSE_LIT_PAIR && o->rgb[2] == PULSE_LIT) {
            o->attr |= BTL_OBJ_PICKED;
            return;
        }
        half = g_btl_half_rate;
        to = PULSE_LIT;
    }
    o->rgb_to[0] = to;
    o->rgb_to[1] = to;
    o->rgb_to[2] = to;
    if (half != 0) {
        o->fade = PULSE_FAST;
    } else {
        o->fade = PULSE_RATE;
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/pulsepick", BtlPulsePicked);
#endif
