/* Persona 1 (JP) - the ward moves 0x8C..0x8F leave behind.  BTLP only.
 *   0x800C2D54 BtlFxFinish8C
 *
 * The effect is over and the ward has to be handed out. Every fighter the
 * acting record still reaches - and the target the chain was armed on, which
 * is put back into the mask first so it is reached like the rest - gets
 * whatever ward the move grants, and three rounds for it to run. The four
 * wards are mutually exclusive, so all four bits come off before the one this
 * move carries goes on, and a fighter that is out of the fight is stepped
 * over.
 *
 * Then the record waits out whatever timer the effect left on it and lets the
 * five effect sound slots go.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/spellfx.h>

/* The two phases the finish runs in: hand the ward out, then wait. The
   effect's own steps leave the record on the first of them. */
#define FX_8C_APPLY 0x80
#define FX_8C_WAIT  0x90

/* Rounds a ward stands for. */
#define FX_8C_TURNS 3

/* The effect sound slots, which the finish gives back one at a time. */
#define FX_8C_SLOT_FIRST 7
#define FX_8C_SLOT_LAST  12

void BtlFxFinish8C(BtlObj *o)
{
    int slot;

    g_btl_actors[g_btl_actor_turn].targets |= 1 << g_btl_fx_target;
    switch (o->phase) {
    case FX_8C_APPLY:
        g_btl_actors[g_btl_actor_turn].unkD0++;
        g_btl_hit_walk = 0;
        g_btl_hit_mask = 1;
        do {
            if ((g_btl_actors[g_btl_actor_turn].targets
                 & (u_short)g_btl_hit_mask) != 0
                && g_btl_actors[g_btl_hit_walk].c.key != 0
                && (signed char)g_btl_actors[g_btl_hit_walk].c.status
                       != BTL_STATUS_DOWN
                && (g_btl_actors[g_btl_hit_walk].flags & BTL_ACTOR_OUT) == 0) {
                g_btl_actors[g_btl_hit_walk].flags &= ~BTL_ACTOR_WARDS;
                /* 0x8D and 0x8E are the wrong way round on purpose. Only
                   these two leave a block of their own: the tests for 0x8C
                   and 0x8F have a free delay slot and gcc folds their single
                   instruction into it, so where those two are written leaves
                   no trace, while the middle pair comes out in source order
                   and the image has 0x8E's block first. */
                switch (o->kind) {
                case 0x8C:
                    g_btl_actors[g_btl_hit_walk].flags |= BTL_ACTOR_WARD_8C;
                    break;
                case 0x8E:
                    g_btl_actors[g_btl_hit_walk].flags |= BTL_ACTOR_WARD_8E;
                    break;
                case 0x8D:
                    g_btl_actors[g_btl_hit_walk].flags |= BTL_ACTOR_WARD_8D;
                    break;
                case 0x8F:
                    g_btl_actors[g_btl_hit_walk].flags |= BTL_ACTOR_WARD_8F;
                    break;
                }
                g_btl_actors[g_btl_hit_walk].ward_turns = FX_8C_TURNS;
            }
            g_btl_hit_walk++;
            g_btl_hit_mask <<= 1;
        } while (g_btl_hit_walk < BTL_ACTORS);
        o->phase = FX_8C_WAIT;
        return;
    case FX_8C_WAIT:
        if (o->timer != 0) {
            return;
        }
        slot = FX_8C_SLOT_FIRST;
        do {
            BtlSoundClose(slot);
            slot++;
        } while (slot < FX_8C_SLOT_LAST);
        o->motion = 0;
        o->phase = 0;
        return;
    default:
        return;
    }
}
