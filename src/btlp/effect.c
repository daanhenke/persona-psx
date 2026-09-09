/* Persona 1 (JP) - the battle's effect slots.  BTLP only.
 *   0x800776F8 BtlEffectRelease  0x8007772C BtlEffectSelect
 *   0x8007778C BtlEffectRestore  0x800777E4 BtlEffectSetKind
 *   0x800774E0 BtlEffectDrop     0x800774B0 BtlCopyMood
 *
 * Four slots, each either a pointer to an effect record or -1 for free. The
 * draw pass walks all four: it dispatches on the low nibble of the record's
 * kind byte, and a handler that reports itself finished has its slot cleared
 * and its running bit taken away. BtlEffectRelease is the same ending done by
 * hand, for an effect that is being cut short rather than running out.
 *
 * Selecting a slot marks whatever was current before and remembers it, so a
 * caller can borrow the current slot and hand it back.
 *
 * The kind is what the record animates, and it is filtered on the way in: with
 * battle animations turned off the longer kinds are replaced by shorter ones,
 * which is the same setting that snaps the panel and the camera to their end
 * states.
 */
#include <decomp/types.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/effect.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/input.h>

extern u_long g_btl_mood_shown[];

void BtlCopyMood(const u_short *mood)
{
    u_long  *p;
    u_short  v;
    int      i;

    i = 0;
    p = g_btl_mood_shown;
    do {
        v = *mood;
        mood++;
        i++;
        *p = v;
        p++;
    } while (i < BTL_MOODS);
}

void BtlEffectDrop(void)
{
    int *held;

    held = &g_btl_effect_held;
    if (*held != BTL_EFFECT_FREE) {
        BtlEffectRelease(*held);
        BtlEffectRestore();
        *held = BTL_EFFECT_FREE;
    }
}

/* Kinds 1 to 3 all become 3, and 5 and 6 become 4; kind 0 and kind 4 are left
   alone, and so is anything past 6. */
