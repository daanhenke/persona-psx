/* Persona 1 (JP) - the move whose effect stands on every fighter opposite.
 * BTLP only.
 *   0x800B75C4 BtlFxStartOnSide
 *
 * A start handler out of g_btl_spell_fx, and the only one that decides what it
 * covers for itself: it takes the side the acting fighter is not on and opens
 * one record over every fighter of it that is still in the fight. The target
 * mask is not read at all.
 *
 * Each record is given a later timer than the one before it, counted in
 * records rather than slots - so an empty or beaten slot does not leave a gap
 * in the timing - and they are chained together, head first.
 *
 * The code that says a fighter is out doubles as the mark every record but the
 * head carries: both are 0x11, and the image keeps one register for the two.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* The blue every fighter under the effect is walked to, and how fast. */
#define FX_SIDE_R    0
#define FX_SIDE_G    0x80
#define FX_SIDE_B    0xFF
#define FX_SIDE_FADE 1

/* Which of a set each record stands for, and how much later each one
   starts. */
#define FX_MARK_HEAD 0x10
#define FX_MARK_REST 0x11
#define FX_SIDE_STEP 2

BtlObj *BtlFxStartOnSide(void)
{
    BtlObj *o;
    BtlObj *head;
    BtlObj *last;
    int     first;
    int     end;
    int     slot;
    int     made;

    if (g_btl_actor_turn < BTL_PARTY)
    {
        first = BTL_PARTY;
        end   = BTL_ACTORS;
    }
    else
    {
        first = 0;
        end   = BTL_PARTY;
    }

    slot = first;
    made = 0;
    last = 0;
    while (slot < end)
    {
        if (g_btl_actors[slot].c.key != 0
            && (signed char)g_btl_actors[slot].c.status != BTL_STATUS_DOWN
            && !(g_btl_actors[slot].flags & BTL_ACTOR_OUT))
        {
            g_btl_actors[slot].obj->rgb_to[0] = FX_SIDE_R;
            g_btl_actors[slot].obj->rgb_to[1] = FX_SIDE_G;
            g_btl_actors[slot].obj->rgb_to[2] = FX_SIDE_B;
            g_btl_actors[slot].obj->fade      = FX_SIDE_FADE;

            o = BtlOpenFxObj(slot, made * FX_SIDE_STEP);
            if (last != 0)
            {
                last->attached = o;
                o->mark_num    = FX_MARK_REST;
            }
            else
            {
                o->mark_num = FX_MARK_HEAD;
                head        = o;
            }
            made++;
            last = BtlObjLast(o);
        }
        slot++;
    }
    return head;
}
