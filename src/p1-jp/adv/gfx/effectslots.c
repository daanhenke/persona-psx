/* Persona 1 (JP) - setting the slots up for a full-screen effect.
 *   ADV @ 0x800866D8
 *
 * The eight slots an effect owns are cleared first, and then one of them is
 * armed for whichever effect is running. Only three of the modes arm anything:
 * one has a script of its own and three more share another, and every other
 * mode leaves the slots empty - which is how the colour fades in screenfade.c
 * get a clear screen to draw over.
 *
 * SlotInit takes the script by pointer and keeps it in the slot, so what the
 * two addresses hold is a script and not the picture itself.
 */
#include <types.h>

/* The run of slots an effect owns, and the one it draws through. */
#define EFFECT_SLOT_FIRST 0x34
#define EFFECT_SLOT_LAST  0x3B
#define EFFECT_SLOT       0x38

/* The modes that arm something. */
#define EFFECT_LONE   4
#define EFFECT_SHARED 5
#define EFFECT_ALT_A  10
#define EFFECT_ALT_B  14

extern u_char g_adv_effect;
extern u_char g_effect4_script[];
extern u_char g_effect5_script[];

extern void SlotClear(u_char slot);
extern void SlotInit(void *def, u_char slot, int attr, short x, short y);
extern void SlotInitTagged(void *def, u_char slot, int attr, short x,
                           short y);

void AdvEffectSetupSlots(void)
{
    SlotClear(EFFECT_SLOT_FIRST);
    SlotClear(EFFECT_SLOT_FIRST + 1);
    SlotClear(EFFECT_SLOT_FIRST + 2);
    SlotClear(EFFECT_SLOT_FIRST + 3);
    SlotClear(EFFECT_SLOT_FIRST + 4);
    SlotClear(EFFECT_SLOT_FIRST + 5);
    SlotClear(EFFECT_SLOT_FIRST + 6);
    SlotClear(EFFECT_SLOT_LAST);

    switch (g_adv_effect) {
    case EFFECT_LONE:
        SlotInit(g_effect4_script, EFFECT_SLOT, 0x38, 0, -4);
        break;
    case EFFECT_SHARED:
    case EFFECT_ALT_A:
    case EFFECT_ALT_B:
        SlotInitTagged(g_effect5_script, EFFECT_SLOT, 0x3FF, -0x20, 2);
        break;
    }
}
