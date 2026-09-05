/* Persona 1 (JP) - slot brightness and per-slot fades.  ADV only.
 *
 * These four are compiled into the ADV overlay alone; DNG and S2D have the
 * rest of the slot family but not these, which fits ADV being the only mode
 * that fades individual actors rather than the whole screen.
 *   0x80066154  SlotSetBrightness
 *   0x800660B4  SlotFadeIn
 *   0x80066104  SlotFadeOut
 *   0x800661D0  SlotSetSemiTrans
 *
 * The renderer (0x80065730) drives all of it: each frame, a slot with
 * SLOT_ATTR_FADE_IN adds fade_step to its brightness until it reaches the
 * global fade level and clears its own bit; FADE_OUT subtracts until zero and
 * likewise clears itself. So a fade is started here and finishes on its own.
 */
#include <types.h>
#include <persona/common/slot.h>

/* 0x44-byte records at 0x800DC10C, indexed by a u8 slot. Reached by hardcoded
   address rather than through a linker symbol. */
#define g_slots ((Slot *)0x800DC10C)

/* 0 is black, 0x80 is full - the same scale as the global fade level, which
   the renderer clamps this against. SlotInit leaves it at 0x80. */
void SlotSetBrightness(u_char slot, u_char level)
{
    g_slots[slot].brightness = level;
}

/* Starts a fade up to the global level, `step` per frame. Clearing FADE_OUT
   and FLICKER as well is what makes the three mutually exclusive - whichever
   was set last wins, and the renderer only ever tests one of them. */
void SlotFadeIn(u_char slot, u_char step)
{
    g_slots[slot].attr =
        (g_slots[slot].attr & ~(SLOT_ATTR_FADE_OUT | SLOT_ATTR_FLICKER)) |
        SLOT_ATTR_FADE_IN;
    g_slots[slot].fade_step = step;
}

/* Starts a fade down to black, `step` per frame. */
void SlotFadeOut(u_char slot, u_char step)
{
    g_slots[slot].attr =
        (g_slots[slot].attr & ~(SLOT_ATTR_FADE_IN | SLOT_ATTR_FLICKER)) |
        SLOT_ATTR_FADE_OUT;
    g_slots[slot].fade_step = step;
}

/* Turns 50% semi-transparency on or off for the slot. The renderer forwards
   the bit straight into GsSPRITE.attribute, where bit 30 has the same
   meaning. */
void SlotSetSemiTrans(u_char slot, u_char on)
{
    Slot *s;

    s = g_slots + slot;
    if (on) {
        s->attr |= SLOT_ATTR_SEMITRANS;
    } else {
        s->attr &= ~SLOT_ATTR_SEMITRANS;
    }
}
