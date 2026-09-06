/* Persona 1 (JP) - choosing the full-screen effect a scene runs under.
 *   ADV @ 0x8007F1DC
 *
 * The scene pack's header carries two effect codes and an event flag to pick
 * between them, so a place can look one way before something happens in the
 * story and another way after. A scene that never changes leaves the flag at
 * 0xFFFF and always gets the first code.
 *
 * Every caller follows this with AdvEffectSetupSlots, which is what arms the
 * slots the chosen effect draws through.
 */
#include <types.h>

/* No scene flag: the first code is the only one. */
#define SCENE_EFFECT_ALWAYS 0xFFFF

/* Fields of the pack header, which sits below the scene record itself. */
extern short  g_adv_scene_effect_flag;
extern u_char g_adv_scene_effect_clear;
extern u_char g_adv_scene_effect_set;

extern u_char g_adv_effect;

extern u_char EventFlagGet(short id);

void AdvPickEffect(void)
{
    if (EventFlagGet(g_adv_scene_effect_flag) != 0 &&
        (u_short)g_adv_scene_effect_flag != SCENE_EFFECT_ALWAYS) {
        g_adv_effect = g_adv_scene_effect_set;
    } else {
        g_adv_effect = g_adv_scene_effect_clear;
    }
}
