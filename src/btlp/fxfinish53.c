/* Persona 1 (JP) - the finish the stage moves share.  BTLP only.
 *   0x800C10E0 BtlFxFinish53
 *
 * The finish column's entry for 0x53..0x5E, 0x74 and 0xEF. It walks the acting
 * fighter's targets one at a time the way BtlFxFinish01 does, but instead of
 * resolving a blow it applies the move to the fighter it has reached: the
 * raising moves put one of the fighter's seven stage counters up by one - to
 * at most four for the first three and seven for the other four - and 0x56 and
 * 0x5B clear each group; 0x5C sets BTL_ACTOR_5C, 0x74 lifts BTL_ACTOR_TIMED_B
 * and 0xEF sets unkD3. A demon is only given 0x53..0x55 where
 * g_btl_demon_stage_moves has the move's bit set; otherwise it is stepped
 * over. Every application counts one on the acting fighter's unkD0.
 *
 * 0x5D and 0x5E are not walked one fighter at a time: every fighter the
 * acting record reaches, the one the chain was armed on included, is marked at
 * once, and the record goes straight to waiting out its timer and letting the
 * effect sound slots go. The walk that runs out of targets does neither and
 * simply lets the record go.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/spellfx.h>
#include <persona/btlp/status.h>

/* Which of the raising moves each demon can be given, one bit per move from
   FX_53_BIT_MOVE. It is indexed by the key itself, as g_btl_demon_statuses
   is, so only the entries from BTL_KEY_DEMON on are ever read. */
extern const u_char g_btl_demon_stage_moves[];

/* The finish's phases after FX_STEP_DONE: apply the move to the fighter the
   walk has reached, step the walk, find the next fighter, let the record go;
   and the pair 0x5D and 0x5E take instead of the walk. */
#define FX_53_APPLY 0x81
#define FX_53_NEXT  0x82
#define FX_53_FIND  0x83
#define FX_53_DONE  0x84
#define FX_53_ALL   0x8F
#define FX_53_WAIT  0x90

#define FX_53_WAIT_FRAMES 30

/* The effect sound slots, which the finish gives back one at a time. */
#define FX_53_SLOT_FIRST 7
#define FX_53_SLOT_LAST  12

/* The three raising moves a demon may be spared, and the move whose bit is
   the low one in g_btl_demon_stage_moves. */
#define FX_53_DEMON_FIRST 0x53
#define FX_53_DEMON_MOVES 3
#define FX_53_BIT_MOVE    0x54

/* How far the two groups of stage counters go. */
#define FX_53_LOW_MAX  4
#define FX_53_HIGH_MAX 7

/* Puts a stage counter up by one and holds it between nought and `max`. */
#define FX_53_RAISE(stage, max)                                            \
    if ((signed char)++(stage) >= 0) {                                     \
        n = (signed char)(stage);                                          \
        if (n > (max)) {                                                   \
            n = (max);                                                     \
        }                                                                  \
        m = n;                                                             \
    } else {                                                               \
        m = 0;                                                             \
    }                                                                      \
    (stage) = m

void BtlFxFinish53(BtlObj *o)
{
    BtlActor *self;
    BtlActor *a;
    int       n;
    int       m;
    int       slot;

    self = o->actor;
    a = &g_btl_actors[g_btl_hit_slot];
    switch (o->phase) {
    case FX_STEP_DONE:
        o->phase++;
        /* fall through */
    case FX_53_APPLY:
        if ((u_short)(o->kind - FX_53_DEMON_FIRST) < FX_53_DEMON_MOVES
            && a->c.key >= BTL_KEY_DEMON
            && ((g_btl_demon_stage_moves[a->c.key]
                 >> (o->kind - FX_53_BIT_MOVE)) & 1) == 0) {
            o->phase++;
            return;
        }
        switch (o->kind) {
        case 0xEF:
            a->unkD3 = 1;
            break;
        case 0x53:
            FX_53_RAISE(a->stage[0], FX_53_LOW_MAX);
            break;
        case 0x54:
            FX_53_RAISE(a->stage[1], FX_53_LOW_MAX);
            break;
        case 0x55:
            FX_53_RAISE(a->stage[2], FX_53_LOW_MAX);
            break;
        case 0x56:
            a->stage[0] = 0;
            a->stage[1] = 0;
            a->stage[2] = 0;
            break;
        case 0x57:
            FX_53_RAISE(a->stage[3], FX_53_HIGH_MAX);
            break;
        case 0x58:
            FX_53_RAISE(a->stage[4], FX_53_HIGH_MAX);
            break;
        case 0x59:
            FX_53_RAISE(a->stage[5], FX_53_HIGH_MAX);
            break;
        case 0x5A:
            FX_53_RAISE(a->stage[6], FX_53_HIGH_MAX);
            break;
        case 0x5B:
            a->stage[3] = 0;
            a->stage[4] = 0;
            a->stage[5] = 0;
            a->stage[6] = 0;
            break;
        case 0x5C:
            a->flags |= BTL_ACTOR_5C;
            a->unkDE = 0;
            break;
        case 0x74:
            a->flags &= ~BTL_ACTOR_TIMED_B;
            break;
        case 0x5D:
        case 0x5E:
            g_btl_actors[g_btl_actor_turn].targets |= 1 << g_btl_fx_target;
            g_btl_hit_walk = 0;
            do {
                if ((g_btl_actors[g_btl_actor_turn].targets
                     & g_btl_hit_mask) != 0
                    && g_btl_actors[g_btl_hit_walk].c.key != 0
                    && (signed char)g_btl_actors[g_btl_hit_walk].c.status
                           != BTL_STATUS_DOWN
                    && (g_btl_actors[g_btl_hit_walk].flags & BTL_ACTOR_OUT)
                           == 0) {
                    g_btl_actors[g_btl_hit_walk].flags |=
                        o->kind == 0x5E ? BTL_ACTOR_5E : BTL_ACTOR_5D;
                }
                g_btl_hit_walk++;
                g_btl_hit_mask <<= 1;
            } while (g_btl_hit_walk < BTL_ACTORS);
            o->phase = FX_53_ALL;
            break;
        }
        self->unkD0++;
        o->phase++;
        return;
    case FX_53_NEXT:
        if (o->timer != 0) {
            return;
        }
        g_btl_hits_left--;
        if (g_btl_hit_walk < 0) {
            g_btl_hit_mask = 1;
        } else {
            g_btl_hit_mask <<= 1;
        }
        g_btl_hit_walk++;
        o->phase++;
        return;
    case FX_53_FIND:
        for (; g_btl_hit_walk < BTL_ACTORS;
             g_btl_hit_walk++, g_btl_hit_mask <<= 1) {
            if ((g_btl_actors[g_btl_actor_turn].targets
                 & g_btl_hit_mask) != 0
                && g_btl_actors[g_btl_hit_walk].c.key != 0
                && (signed char)g_btl_actors[g_btl_hit_walk].c.status
                       != BTL_STATUS_DOWN
                && (g_btl_actors[g_btl_hit_walk].flags & BTL_ACTOR_OUT) == 0) {
                g_btl_hit_slot = g_btl_hit_walk;
                break;
            }
        }
        if (g_btl_hit_walk >= BTL_ACTORS) {
            o->timer = FX_53_WAIT_FRAMES;
            o->phase++;
        } else {
            o->phase = FX_53_APPLY;
        }
        return;
    case FX_53_WAIT:
        if (o->timer != 0) {
            return;
        }
        slot = FX_53_SLOT_FIRST;
        do {
            BtlSoundClose(slot);
            slot++;
        } while (slot < FX_53_SLOT_LAST);
        /* fall through */
    case FX_53_DONE:
        o->motion = 0;
        o->phase = 0;
        return;
    }
}
