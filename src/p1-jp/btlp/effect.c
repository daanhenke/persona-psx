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
#include <types.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/effect.h>

extern int        g_btl_effect_cur;
extern int        g_btl_effect_prev;
extern int        g_btl_effect_held;
extern u_char     g_btl_fast_anim;

extern u_long g_btl_mood_shown[];

extern void BtlCursorInitPrims(void);

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
void BtlEffectSetKind(int slot, u_char kind)
{
    int k;

    /* The tests go through a plain int - the byte is masked once for them and
       stored unmasked - which is what makes the comparisons signed. */
    k = kind;
    if (g_btl_fast_anim != 0 && k != 0) {
        if (k < 4) {
            kind = 3;
        } else if (k < 7) {
            /* Nested rather than joined with the test above it: written as one
               condition gcc folds the pair into a single range check. */
            if (k > 4) {
                kind = 4;
            }
        }
    }
    g_btl_effect[slot]->kind = kind;
}

/* Not an effect, but it shares this translation unit: the offer's four gauges
   widened into words of their own for whatever draws them. */
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
