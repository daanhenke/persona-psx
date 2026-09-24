/* Persona 1 (JP) - one frame of the room.  ADV only.
 *   0x80086250 AdvRunFrame
 *
 * The room's per-frame work, skipped while a scene file is loading: the
 * effects and the view shake step, the first eight slots get their x scale,
 * the actors are sorted against the one the view follows and their sprites
 * placed - and then, loading or not, the frame is drawn. The two middle passes
 * are SlotsApplyXScale and ActorsSetDepth written out in place.
 */
#include <decomp/types.h>
#include <persona/adv/actor.h>
#include <persona/common/slot.h>

/* 0x44-byte records at 0x800DC10C, reached by hardcoded address. */
#define g_slots ((Slot *)0x800DC10C)
extern Slot *g_slot_cur;

/* Set while AdvLoadBst's file is on its way in. */
extern u_char g_adv_loading;

extern void AdvDrawEffect(void);
extern void ViewShakeStep(void);
extern void RenderFrame(void);

void AdvRunFrame(void)
{
    AdvActor *a;
    u_char    i;
    int       y;

    if (g_adv_loading == 0) {
        AdvDrawEffect();
        ViewShakeStep();

        for (i = 0; i < 8; i++) {
            g_slot_cur = &g_slots[i];
            if (g_slots[i].attr & SLOT_ATTR_XSCALE) {
                g_slots[i].scale_x = 0xFFF;
            } else {
                g_slots[i].scale_x = 0x1000;
            }
        }

        y = g_adv_actors[g_cam_actor].y;
        for (a = g_adv_actors; (long)a < (long)&g_adv_actors[ACTOR_COUNT]; a++) {
            if (a->id == ACTOR_NONE) {
                a->depth = 0;
            } else if (y >= a->y) {
                a->depth = DEPTH_BEHIND;
            } else {
                a->depth = 0;
            }
        }

        ActorsPlaceSprites();
    }
    RenderFrame();
}
