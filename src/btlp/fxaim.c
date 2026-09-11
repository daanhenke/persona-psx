/* Persona 1 (JP) - the move whose effect stands on the fighter it is aimed at.
 * BTLP only.
 *   0x800B7514 BtlFxStartOnAim
 *
 * A start handler out of g_btl_spell_fx, and the plainest of them: the one
 * fighter the acting record is aimed at is put on the effect's blue and given
 * a single record standing on it.
 *
 * The aim is read off the acting fighter rather than out of g_btl_fx_target,
 * which is the same byte reached the long way round.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* The blue the fighter under the effect is walked to, and how fast. */
#define FX_AIM_R    0
#define FX_AIM_G    0x80
#define FX_AIM_B    0xFF
#define FX_AIM_FADE 1

BtlObj *BtlFxStartOnAim(void)
{
    int slot;

    slot = g_btl_actors[g_btl_actor_turn].order;
    g_btl_actors[slot].obj->rgb_to[0] = FX_AIM_R;
    g_btl_actors[slot].obj->rgb_to[1] = FX_AIM_G;
    g_btl_actors[slot].obj->rgb_to[2] = FX_AIM_B;
    g_btl_actors[slot].obj->fade      = FX_AIM_FADE;
    return BtlOpenFxObj(slot, 0);
}
