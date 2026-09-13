/* Persona 1 (JP) - what runs down as a round ends.  BTLP only.
 *   0x80096510 BtlCountDownRound
 *
 * Every fighter on both sides has its turn-long flags dropped and each of its
 * timers counted down by one round, and whatever a timer was holding up is
 * let go of when it runs out:
 *
 * - the two conditions at BTL_ACTOR_TIMED_A and _B, each with a counter of its
 *   own, and the ward at BTL_ACTOR_WARDS with `ward_turns`;
 * - `unkDE`, which only lasts the round and takes attribute bit 0x40 with it;
 * - a lasting ailment, which is not lifted outright when `ail_turns` runs out
 *   but stepped down a level and given two more turns - it is only once the
 *   level is already at nought that the ailment goes;
 * - the cloak at BTL_STATUS_LIFTED, which is let go of the moment its turns
 *   are up, level or not.
 *
 * Poison, sickness, the puppet string, the counter and the wolf do not wear
 * off by themselves. Last of all the two rounds-long bans on fleeing are
 * counted down as well.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/round.h>

/* The flags that only ever last the turn they were set in. */
#define ROUND_FLAGS (0x20000000 | BTL_ACTOR_5E | BTL_ACTOR_5D)

/* What an ailment that has stepped down a level is given to run. */
#define ROUND_AIL_TURNS 2

extern BtlActor g_btl_enemies[];

/* 96.32%: the image keeps the table's address in a register of its own and
   builds the second and third counters' stores from it and the slot's
   offset, where gcc here reaches all three through the walked pointer. Every
   field through the global table, every field through the pointer, and a
   local copy of the table's address all land further away. */
#ifdef NON_MATCHING
void BtlCountDownRound(void)
{
    BtlActor *a;
    int       status;
    int       i;

    a = g_btl_actors;
    i = 0;
    do {
        a->flags &= ~ROUND_FLAGS;
        if (a->flags & BTL_ACTOR_TIMED_A) {
            if ((signed char)--g_btl_actors[i].timed_a <= 0) {
                a->flags &= ~BTL_ACTOR_TIMED_A;
            }
        }
        if (a->flags & BTL_ACTOR_TIMED_B) {
            if ((signed char)--g_btl_actors[i].timed_b <= 0) {
                g_btl_actors[i].flags &= ~BTL_ACTOR_TIMED_B;
            }
        }
        if (a->flags & BTL_ACTOR_WARDS) {
            if ((signed char)--g_btl_actors[i].ward_turns <= 0) {
                g_btl_actors[i].flags &= ~BTL_ACTOR_WARDS;
            }
        }
        if (a->unkDE != 0) {
            a->unkDE = 0;
            a->flags &= ~BTL_ACTOR_5C;
        }

        status = a->c.status;
        if ((status >= 1 && status <= 12) || status == 0x15 || status == 0x16
            || status == 0xE || status == 0xF) {
            if ((signed char)--a->ail_turns <= 0) {
                if ((signed char)--a->c.ail_level < 0) {
                    a->c.status = 0;
                    a->c.ail_level = 0;
                } else {
                    a->ail_turns = ROUND_AIL_TURNS;
                }
            }
        }
        if ((signed char)a->c.status == BTL_STATUS_LIFTED) {
            if ((signed char)--a->ail_turns <= 0) {
                a->c.status = 0;
                a->c.ail_level = 0;
            }
        }
        a->unkD3 = 0;
        i++;
        a++;
    } while ((int)&a->flags < (int)&g_btl_enemies[BTL_ENEMIES].flags);

    if (g_btl_party_no_flee != 0) {
        g_btl_party_no_flee--;
    }
    if (g_btl_no_flee != 0) {
        g_btl_no_flee--;
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/roundtick", BtlCountDownRound);
#endif
