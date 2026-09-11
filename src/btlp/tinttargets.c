/* Persona 1 (JP) - putting every fighter a move reaches on one colour.
 * BTLP only.
 *   0x800C005C BtlTintTargets
 *
 * Walks both sides against the acting fighter's target mask and starts every
 * live fighter in it walking toward the colour it is handed. Only the colour
 * is set - nothing is drawn - so an effect handler calls this on its way in to
 * tint what it is about to hit and again on its way out to put it back.
 *
 * The slot it is handed is not read: the routine takes the mask off
 * g_btl_actor_turn itself, and half its callers pass nothing at all.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>

/* How far a fighter walks toward the colour in one frame. */
#define TINT_FADE 2

void BtlTintTargets(int slot, short r, short g, short b)
{
    int     i;
    int     mask;

    i    = 0;
    mask = 1;
    do
    {
        if (g_btl_actors[g_btl_actor_turn].targets & mask)
        {
            if (g_btl_actors[i].c.key != 0
                && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
                && !(g_btl_actors[i].flags & BTL_ACTOR_OUT))
            {
                g_btl_actors[i].obj->rgb_to[0] = r;
                g_btl_actors[i].obj->rgb_to[1] = g;
                g_btl_actors[i].obj->rgb_to[2] = b;
                g_btl_actors[i].obj->fade      = TINT_FADE;
            }
        }
        i++;
        mask <<= 1;
    } while (i < BTL_ACTORS);
}
