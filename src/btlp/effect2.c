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

/* Defined in the unit before this one; the prototype is what
   decides how the arguments are converted. */
extern void BtlCopyMood(const u_short *mood);
extern void BtlEffectDrop(void);

void BtlEffectRelease(int slot)
{
    BtlEffect *e;

    e = g_btl_effect[slot];
    e->flags &= ~BTL_EFFECT_RUNNING;
    g_btl_effect[slot] = (BtlEffect *)BTL_EFFECT_FREE;
}

void BtlEffectSelect(int slot)
{
    if (g_btl_effect_cur != BTL_EFFECT_FREE) {
        g_btl_effect[g_btl_effect_cur]->mark = BTL_EFFECT_MARK;
    }
    g_btl_effect_prev = g_btl_effect_cur;
    g_btl_effect_cur = slot;
    BtlCursorInitPrims();
}

void BtlEffectRestore(void)
{
    g_btl_effect_cur = g_btl_effect_prev;
}

/* Lets go of the slot this caller was holding and puts back whichever was
   current before it. Nothing else reads the held slot.

   Reached through a pointer so its address is worked out once and kept across
   the two calls, rather than rebuilt for the read and again for the write. */

/* Round to the next slot, keeping the one being left as the slot to restore.
   Nothing calls it; BtlEffectSelect is how a slot is actually chosen. */
void BtlEffectNext(void)
{
    int cur;

    cur = g_btl_effect_cur;
    g_btl_effect_prev = cur;
    g_btl_effect_cur = (cur + 1) % BTL_EFFECT_SLOTS;
}
