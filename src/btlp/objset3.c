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
#include <decomp/types.h>
#include <persona/btlp/object.h>
/* Defined in the unit before this one; the prototype is what
   decides how the arguments are converted. */

void BtlObjSetFade(BtlObj *obj, u_char rate)
{
    obj->fade = rate;
    if (obj->attached != 0) {
        BtlObjSetFade(obj->attached, rate);
    }
}
