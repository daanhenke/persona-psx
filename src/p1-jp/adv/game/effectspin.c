/* Persona 1 (JP) - the two ways a cutscene effect leaves the screen.
 *
 *   ADV @ 0x80081D34, 0x80081E94
 *
 * The cutscene dispatcher at 0x80080CE0 plays an effect over an actor's head:
 * it spawns a sprite in slot 62 at the actor's screen position, holds it for a
 * while, and then calls one of these to take it away. Each puts its own sprite
 * up for 48 frames and then spins it out over 16 more before clearing the
 * slot, so the call blocks for the whole animation.
 *
 * The actor index arrives from the dispatcher already biased past the first
 * eight records, which is why nothing here adds to it.
 */
#include <types.h>
#include <persona/adv/actor.h>
#include <persona/common/slot.h>

/* Reached by hardcoded address rather than through the linker symbol. */
#define g_slots ((Slot *)0x800DC10C)

#define EFFECT_SLOT 62
#define HOLD_FRAMES 0x30
#define SPIN_FRAMES 0x10
#define SPIN_STEP   0x16000     /* rotation added per frame */
#define SHRINK_STEP 0xF0        /* scale lost per frame     */

extern Slot  *g_slot_cur;
extern short  g_cam_x;
extern short  g_cam_y;

/* Slot definitions for the two exit sprites. */
extern void g_adv_shrink_away_def;
extern void g_adv_squash_away_def;

extern void AdvRunFrame(void);

/* Shrinks the sprite to nothing while it turns clockwise. */
void AdvEffectShrinkAway(u_char actor)
{
    Slot *s;
    int i;

    g_slot_cur = &g_slots[EFFECT_SLOT];
    SlotInitTagged(&g_adv_shrink_away_def, EFFECT_SLOT,
                   g_adv_actors[actor].z - 1,
                   g_adv_actors[actor].world_x - g_cam_x,
                   g_adv_actors[actor].world_y - g_cam_y - 0x50);
    s = g_slot_cur;
    s->mx = 0x18;
    s->my = 4;
    i = 0;
    do {
        i++;
        AdvRunFrame();
    } while (i < HOLD_FRAMES);

    i = 0;
    do {
        g_slot_cur = &g_slots[EFFECT_SLOT];
        g_slots[EFFECT_SLOT].scale_x -= SHRINK_STEP;
        g_slots[EFFECT_SLOT].scale_y -= SHRINK_STEP;
        g_slots[EFFECT_SLOT].rotate += SPIN_STEP;
        i++;
        AdvRunFrame();
    } while (i < SPIN_FRAMES);
    SlotClear(EFFECT_SLOT);
}

/* Stretches the sprite wide as it flattens, turning the other way. */
void AdvEffectSquashAway(u_char actor)
{
    Slot *s;
    int i;

    g_slot_cur = &g_slots[EFFECT_SLOT];
    SlotInitTagged(&g_adv_squash_away_def, EFFECT_SLOT,
                   g_adv_actors[actor].z - 1,
                   g_adv_actors[actor].world_x - g_cam_x,
                   g_adv_actors[actor].world_y - g_cam_y - 0x50);
    s = g_slot_cur;
    s->mx = 0x15;
    s->my = 4;
    i = 0;
    do {
        i++;
        AdvRunFrame();
    } while (i < HOLD_FRAMES);

    i = 0;
    do {
        g_slot_cur = &g_slots[EFFECT_SLOT];
        g_slots[EFFECT_SLOT].scale_x += 0x200;
        g_slots[EFFECT_SLOT].scale_y -= SHRINK_STEP;
        g_slots[EFFECT_SLOT].rotate -= SPIN_STEP;
        i++;
        AdvRunFrame();
    } while (i < SPIN_FRAMES);
    SlotClear(EFFECT_SLOT);
}
