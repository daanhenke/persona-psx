/* Persona 1 (JP) - changing a display object.  BTLP only.
 *   0x800C4488 BtlObjSetMotion  0x800C44CC BtlObjSetAttr
 *   0x800C4500 BtlObjClearAttr  0x800C4538 BtlObjSetTimer
 *   0x800C45A0 BtlObjSetPhase   0x800C4608 BtlObjSetRgb
 *   0x800C469C BtlObjSetFade
 *
 * One shape, seven times over: write the field, then hand the same value to
 * whatever is attached. An enemy carries its shadow that way, so tinting the
 * enemy tints the shadow with it and nothing has to know the assembly's parts.
 * The recursion is only as deep as the chain is long, which in practice is one.
 *
 * Colour is not written straight to the object. Each record keeps the colour it
 * is drawn in and the colour it is heading for, and the per-frame walk moves the
 * first toward the second by `fade` a frame - so BtlObjSetRgb asks for a colour
 * and BtlObjSetFade says how quickly to get there. A fade of 0xFF arrives in one
 * frame, which is how a caller that wants no transition spells it.
 *
 * Motion doubles as a busy flag: the code that plays one sets it and then pumps
 * frames until the object puts it back to zero, and the command menu ignores the
 * pad while it is set. Clearing it resets the phase, because a motion that is no
 * longer running has no step to be on.
 */
#include <types.h>
#include <persona/btlp/object.h>

void BtlObjSetMotion(BtlObj *obj, u_char motion)
{
    obj->motion = motion;
    if (motion == 0) {
        obj->phase = 0;
    }
    if (obj->attached != 0) {
        BtlObjSetMotion(obj->attached, motion);
    }
}

void BtlObjSetAttr(BtlObj *obj, u_long bits)
{
    obj->attr |= bits;
    if (obj->attached != 0) {
        BtlObjSetAttr(obj->attached, bits);
    }
}

void BtlObjClearAttr(BtlObj *obj, u_long bits)
{
    obj->attr &= ~bits;
    if (obj->attached != 0) {
        BtlObjClearAttr(obj->attached, bits);
    }
}

void BtlObjSetTimer(BtlObj *obj, short frames)
{
    obj->timer = frames;
    if (obj->attached != 0) {
        BtlObjSetTimer(obj->attached, frames);
    }
}

void BtlObjSetPhase(BtlObj *obj, u_char phase)
{
    obj->phase = phase;
    if (obj->attached != 0) {
        BtlObjSetPhase(obj->attached, phase);
    }
}

/* The green parameter is wider than the two either side of it because it is
   only ever stored, never handed on - the recursion passes blue in its place.
   That is a bug, and it does not show: every caller asks for a grey, so green
   and blue arrive equal anyway. Left as the original has it. */
void BtlObjSetRgb(BtlObj *obj, short r, int g, short b)
{
    obj->rgb_to[0] = r;
    obj->rgb_to[1] = g;
    obj->rgb_to[2] = b;
    if (obj->attached != 0) {
        BtlObjSetRgb(obj->attached, r, b, b);
    }
}

void BtlObjSetFade(BtlObj *obj, u_char rate)
{
    obj->fade = rate;
    if (obj->attached != 0) {
        BtlObjSetFade(obj->attached, rate);
    }
}
