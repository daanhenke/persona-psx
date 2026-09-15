/* Persona 1 (JP) - the motion an enemy's attack is played out in.  BTLP only.
 *   0x800B262C BtlEnemyMotion02    0x800B38EC BtlEnemyStrike
 *
 * Entry 2 of g_btl_enemy_motion, the enemy twin of BtlMemberMotion02, and the
 * blow it lands through.
 *
 * The motion runs in five groups of phases, laid out in the image in the order
 * they are written here. Nought settles the approach - a plain walk in, a
 * lunge (0x30 on), a flight (5 on) - and reads the move's artwork; one to four
 * carry the body in and open the voice bank. 0x13 is the walk that picks the
 * next target out of the fighter's mask, 0x14 sounds the blow, 0x15 hands
 * over to BtlEnemyStrike and 0x16 closes that hit and goes back to 0x13.
 * 0x17 and 0x18 carry the body home, and 0x20 to 0x22 are the three ways a
 * turn ends early - a move that gave up, the fighter throwing itself away, and
 * the wait behind that.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <rand.h>
#include <libcd.h>
#include <libsnd.h>
#include <persona/main/cd.h>
#include <persona/common/persona.h>
#include <persona/common/spell.h>
#include <persona/common/status.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/clut.h>
#include <persona/btlp/damage.h>
#include <persona/btlp/fieldmarks.h>
#include <persona/btlp/load.h>
#include <persona/btlp/model.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pack.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sides.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/spellfx.h>
#include <persona/btlp/status.h>
#include <persona/btlp/strike.h>
#include <persona/btlp/text.h>

/* The enemy moves with a rule of their own. */
#define ENEMY_MOVE_BUILD       0xDC   /* worth an eighth more each time    */
#define ENEMY_MOVE_ANY_ELEMENT 0xDE   /* strikes with an element at random */
#define ENEMY_MOVE_SELF        0xDF   /* spends the fighter's own hp       */
#define ENEMY_MOVE_BOTH_DOWN   0xE1   /* takes the fighter and its target  */
#define ENEMY_MOVE_WOUND       0xE3   /* leaves BTL_ACTOR_WOUND behind     */

/* Set in a species' attribute word on an enemy that flies in rather than
   walking. */
#define ENEMY_ATTR_FLIES 0x8000

/* How a move's species attack byte says it is played out. */
#define ENEMY_LUNGE_LAST 2
#define ENEMY_FLIGHT     5
#define ENEMY_GROUNDED   4

/* The encounter that has its enemies strike from where they stand. */
#define ENEMY_ENCOUNTER_STILL 8

/* The approaches: frames each takes, and how far short of the target a walk
   and a lunge stop. */
#define ENEMY_WALK_FRAMES  20
#define ENEMY_WALK_SHORT   0x3C0000
#define ENEMY_LUNGE_FRAMES 16
#define ENEMY_LUNGE_SHORT  0x1E0000
#define ENEMY_HOME_FRAMES  40

/* A flight rises to here, and keeps its height above the floor at this. */
#define ENEMY_FLY_TOP   (-0x8C0000)
#define ENEMY_FLY_BACK  (-0x400000)
#define ENEMY_FLY_FLOOR (-0x180000)
#define ENEMY_FLY_SINK  0x20000

/* Where the grid puts a square, in whole pixels before the 16.16 shift. */
#define ENEMY_GRID_X    15
#define ENEMY_GRID_LEFT (-60)
#define ENEMY_GRID_Y    20
#define ENEMY_GRID_TOP  (-140)

void BtlEnemyMotion02(BtlObj *o)
{
    CdlLOC            loc;
    BtlSoundBank      bank;
    BtlActor         *a;
    const BtlSpecies *sp;
    SpellData        *spell;
    int               kind;
    int               done;
    int               i;
    int               speed;
    u_short           walk;
    u_short           kept;

    a = o->actor;
    sp = &g_btl_species[o->kind];
    spell = &g_spell_data[a->turn_move];
    switch (o->phase) {
    case 0:
        g_btl_hit_slot = a->order;
        g_btl_hits_left = BtlRollHits(spell->cost);
        if (g_btl_effect_obj == 0 && a->move != 0 && g_btl_msg_speed != 2) {
            BtlOpenMessage(5, 2, g_btl_move_lines[a->move], 0x68, 0xC);
            if (g_btl_msg_speed == 0) {
                speed = 0x5A;
            } else {
                speed = 0x1E;
            }
            g_btl_msg_timer = speed;
        }
        if (a->move == 0 && (o->attr & ENEMY_ATTR_FLIES) != 0
            && a->order < BTL_PARTY) {
            o->phase = 5;
            break;
        }
        if (a->move != 0) {
            kind = sp->attack[o->spell_slot];
            if (kind != 0) {
                if (kind <= ENEMY_LUNGE_LAST) {
                    o->phase = 0x30;
                } else if (kind == ENEMY_FLIGHT) {
                    o->phase = 5;
                }
            }
        }
        if (o->phase != 0) {
            break;
        }
        if (g_btl_encounter != ENEMY_ENCOUNTER_STILL) {
            o->step_x = (g_btl_actors[g_btl_hit_slot].obj->x - o->x)
                        / ENEMY_WALK_FRAMES;
            o->step_y = (g_btl_actors[g_btl_hit_slot].obj->y - o->y
                         - ENEMY_WALK_SHORT)
                        / ENEMY_WALK_FRAMES;
        } else {
            o->step_x = 0;
            o->step_y = 0;
        }
        o->steps = ENEMY_WALK_FRAMES;
        if (a->move != 0) {
            if ((spell->kind & 1) != 0) {
                CdIntToPos(g_btl_persona_sectors[BTL_MOVE_FILE_FIRST + a->move]
                               + g_btl_move_gfx_base, &loc);
                CdReadFileToAddrAsync(
                    (CdlFILE *)&loc,
                    g_btl_persona_sectors[BTL_MOVE_FILE_FIRST + a->move + 1]
                        - g_btl_persona_sectors[BTL_MOVE_FILE_FIRST + a->move],
                    BTL_LOAD_STAGE);
            } else {
                o->phase += 2;
                break;
            }
        } else {
            BtlReadVoiceBank(g_persona_data[a->c.key].attack);
        }
        o->phase++;
        break;
    case 1:
        o->x += o->step_x;
        o->y += o->step_y;
        if (--o->steps == 0) {
            o->steps = ENEMY_WALK_FRAMES;
            o->x2 = o->x;
            o->y2 = o->y;
            o->phase++;
        }
        break;
    case 2:
        if (g_cd_busy != -1) {
            break;
        }
        /* The call written out in each arm: with the script picked into a
           local first, the index lives across the join and loses v0. */
        if (a->move == 0) {
            BtlObjSetScript(o, (BtlSeqStep *)o->scripts[sp->talk]);
        } else {
            BtlObjSetScript(
                o, (BtlSeqStep *)o->scripts[sp->strike[o->spell_slot]]);
        }
        if (a->move != 0 && (spell->kind & 1) == 0) {
            CdIntToPos(g_btl_persona_sectors[BTL_MOVE_FILE_FIRST + a->move]
                           + g_btl_move_gfx_base, &loc);
            CdReadFileToAddrAsync(
                (CdlFILE *)&loc,
                g_btl_persona_sectors[BTL_MOVE_FILE_FIRST + a->move + 1]
                    - g_btl_persona_sectors[BTL_MOVE_FILE_FIRST + a->move],
                BTL_LOAD_STAGE);
        }
        o->phase++;
        break;
    case 3:
        if (g_cd_busy != -1) {
            break;
        }
        if (a->move != 0) {
            CdIntToPos(g_btl_pack_sectors[a->move] + g_btl_voice_base, &loc);
            CdReadFileToAddrAsync((CdlFILE *)&loc,
                                  g_btl_pack_sectors[a->move + 1]
                                      - g_btl_pack_sectors[a->move],
                                  (u_long *)BTL_VOICE_BUFFER);
        }
        o->phase++;
        break;
    case 4:
        if ((o->attr & BTL_OBJ_BUSY_MASK) == BTL_OBJ_BUSY) {
            break;
        }
        if (g_cd_busy != -1) {
            break;
        }
        if (a->move == 0) {
            BtlOpenVoiceBank();
        } else {
            bank.nsep = BTL_VOICE_SEPS;
            bank.vb = g_btl_voice_vb;
            bank.vh = g_btl_voice_vh;
            bank.seq = g_btl_voice_seq;
            BtlSoundOpen(&bank, BTL_BGM_SLOT, 0);
        }
        g_btl_hit_walk = -1;
        g_btl_hit_mask = 1;
        a->targets &= ~(1 << g_btl_hit_slot);
        o->phase = 0x14;
        break;
    case 0x30:
        CdIntToPos(g_btl_persona_sectors[BTL_MOVE_FILE_FIRST + a->move]
                       + g_btl_move_gfx_base, &loc);
        CdReadFileToAddrAsync(
            (CdlFILE *)&loc,
            g_btl_persona_sectors[BTL_MOVE_FILE_FIRST + a->move + 1]
                - g_btl_persona_sectors[BTL_MOVE_FILE_FIRST + a->move],
            BTL_LOAD_STAGE);
        o->phase++;
        break;
    case 0x31:
        if (g_cd_busy != -1) {
            break;
        }
        CdIntToPos(g_btl_pack_sectors[a->move] + g_btl_voice_base, &loc);
        CdReadFileToAddrAsync((CdlFILE *)&loc,
                              g_btl_pack_sectors[a->move + 1]
                                  - g_btl_pack_sectors[a->move],
                              (u_long *)BTL_VOICE_BUFFER);
        BtlObjSetScript(o, (BtlSeqStep *)o->scripts[sp->strike[o->spell_slot]]);
        o->step_x = (g_btl_actors[g_btl_hit_slot].obj->x - o->x)
                    / ENEMY_LUNGE_FRAMES;
        o->step_y = (g_btl_actors[g_btl_hit_slot].obj->y - o->y
                     - ENEMY_LUNGE_SHORT)
                    / ENEMY_LUNGE_FRAMES;
        o->steps = ENEMY_LUNGE_FRAMES;
        o->timer = 0x3C;
        o->phase++;
        break;
    case 0x32:
        if (o->timer != 0) {
            break;
        }
        if (--o->steps == 0) {
            o->steps = ENEMY_LUNGE_FRAMES;
            o->phase++;
        }
        o->x += o->step_x;
        o->y += o->step_y;
        o->z = -g_btl_wave_sin[(ENEMY_LUNGE_FRAMES - o->steps) * 16] * 0x50;
        break;
    case 0x33:
        if (g_cd_busy == -1) {
            o->phase = 4;
        }
        break;
    case 0x34:
        if (o->timer != 0) {
            break;
        }
        if (--o->steps == 0) {
            o->motion = 0;
            o->phase = 0;
            o->attr &= ~BTL_OBJ_CARRIED;
        }
        o->x -= o->step_x;
        o->y -= o->step_y;
        if (sp->attack[o->spell_slot] != ENEMY_GROUNDED) {
            o->z = -g_btl_wave_sin[(ENEMY_LUNGE_FRAMES - o->steps) * 16] * 0x40;
        }
        break;
    case 5:
        BtlReadVoiceBank(g_persona_data[a->c.key].attack);
        o->angle = BtlAngleTo(ENEMY_FLY_TOP - o->y, ENEMY_FLY_BACK - o->z);
        o->phase++;
        break;
    case 6:
        o->y += g_btl_wave_sin[o->angle] * 2;
        o->z += g_btl_wave_cos[o->angle] * 2;
        if (o->y > ENEMY_FLY_TOP && o->z > ENEMY_FLY_BACK) {
            break;
        }
        o->angle = BtlAngleTo(g_btl_actors[g_btl_hit_slot].obj->x - o->x,
                              g_btl_actors[g_btl_hit_slot].obj->y - o->y);
        o->steps = 1;
        BtlObjSetScript(o, (BtlSeqStep *)o->scripts[sp->talk]);
        o->phase++;
        break;
    case 7:
        o->x += o->steps * g_btl_wave_sin[o->angle];
        o->y += o->steps * g_btl_wave_cos[o->angle];
        o->z += ENEMY_FLY_SINK;
        if (o->z >= ENEMY_FLY_FLOOR) {
            o->z = ENEMY_FLY_FLOOR;
        }
        if (g_btl_tick & 1) {
            o->steps++;
        }
        if (o->y >= g_btl_actors[g_btl_hit_slot].obj->y - ENEMY_LUNGE_SHORT) {
            o->step_x = (g_btl_actors[g_btl_hit_slot].obj->x - o->x2)
                        / ENEMY_HOME_FRAMES;
            o->step_y = (g_btl_actors[g_btl_hit_slot].obj->y - o->y2)
                        / ENEMY_HOME_FRAMES;
            /* Ahead of the two stores below, where the image loads it. */
            o->step_z = g_btl_models[o->kind].depth << 16;
            o->steps = ENEMY_HOME_FRAMES;
            o->phase = 4;
        }
        break;
    case 0x13:
        do {
            done = 0;
            if ((spell->aim & 1) != 0) {
                if (g_btl_actors[g_btl_hit_slot].c.key == 0
                    || (signed char)g_btl_actors[g_btl_hit_slot].c.status
                           == BTL_STATUS_DOWN
                    || (done = 1,
                        (g_btl_actors[g_btl_hit_slot].flags & BTL_ACTOR_OUT)
                            != 0)) {
                    g_btl_hits_left = 0;
                    done = 1;
                }
            } else {
                walk = g_btl_hit_walk;
                /* A `while` inside the test, not a `do/while`. gcc rolls a
                   loop's first exit to its end: here that is only the test,
                   and loop.c then moves the found arm out ahead of the walk,
                   where the image has it. Written as a `do/while` the break
                   is the first exit and the whole search is rolled instead. */
                if ((short)g_btl_hit_walk < BTL_ACTORS) {
                    while ((short)g_btl_hit_walk < BTL_ACTORS) {
                        if ((a->targets & g_btl_hit_mask) != 0
                            && g_btl_actors[(short)walk].c.key != 0
                            && (signed char)g_btl_actors[(short)walk].c.status
                                   != BTL_STATUS_DOWN
                            && (g_btl_actors[(short)walk].flags & BTL_ACTOR_OUT)
                                   == 0) {
                            g_btl_hit_slot = walk;
                            done = 1;
                            break;
                        }
                        walk = g_btl_hit_walk + 1;
                        g_btl_hit_walk = walk;
                        g_btl_hit_mask <<= 1;
                    }
                    if ((short)g_btl_hit_walk < BTL_ACTORS) {
                        continue;
                    }
                }
                if ((spell->aim & 2) != 0) {
                    /* Set at the head of the arm, and then a block boundary:
                       cse must not know `done` is 1 at the shift below, or
                       it hands the shift done's register for its 1 where
                       the image keeps the constant the loop hoists. */
                    done = 1;
                    do {
                    } while (0);
                    i = 0;
                    a->targets |= 1 << a->order;
                    kept = g_btl_hits_left;
                    g_btl_hits_left = 0;
                    do {
                        if (((a->targets >> i) & 1) != 0
                            && g_btl_actors[i].c.key != 0
                            && (signed char)g_btl_actors[i].c.status
                                   != BTL_STATUS_DOWN
                            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
                            done = 0;
                            g_btl_hits_left = kept;
                            g_btl_hit_walk = 0;
                            g_btl_hit_mask = 1;
                        }
                        i++;
                    } while (i < BTL_ACTORS);
                } else {
                    g_btl_hits_left = 0;
                    done = 1;
                }
            }
        } while (done == 0);
        o->phase++;
        /* fallthrough */
    case 0x14:
        if (g_btl_hits_left <= 0) {
            if (a->turn_move == ENEMY_MOVE_SELF) {
                a->c.key = 0;
            }
            BtlReadyNextTurn();
            BtlSoundClose(STRIKE_VOICE);
            if ((o->attr & BTL_OBJ_CARRIED) != 0) {
                BtlObjSetScript(o, (BtlSeqStep *)o->scripts[sp->spawn]);
            } else if (a->move == 0) {
                BtlObjSetScript(o, (BtlSeqStep *)o->scripts[sp->stand]);
            } else {
                BtlObjSetScript(
                    o, (BtlSeqStep *)o->scripts[sp->after[o->actor->turn_slot]]);
            }
            o->phase = 0x17;
            break;
        }
        if (g_btl_hit_slot >= BTL_PARTY) {
            BtlSoundOpen(g_btl_slot_banks, STRIKE_VOICE,
                         (g_btl_actors[g_btl_hit_slot].obj->tpage >> 1) - 5);
        } else {
            BtlSoundOpen(g_btl_banks, STRIKE_VOICE,
                         g_btl_actors[g_btl_hit_slot].c.key);
        }
        o->phase++;
        break;
    case 0x15:
        BtlEnemyStrike(o);
        break;
    case 0x16:
        if (g_btl_actors[g_btl_hit_slot].obj->motion != 0) {
            break;
        }
        if (o->timer != 0) {
            break;
        }
        BtlSoundClose(STRIKE_VOICE);
        if ((spell->aim & 4) == 0) {
            g_btl_hits_left--;
        }
        if ((short)g_btl_hit_walk < 0) {
            g_btl_hit_mask = 1;
        } else {
            g_btl_hit_mask <<= 1;
        }
        g_btl_hit_walk++;
        o->phase = 0x13;
        break;
    case 0x17:
        if (a->turn_move != 0) {
            if (a->turn_move == ENEMY_MOVE_SELF) {
                o->phase = 0x21;
                break;
            }
            switch (sp->attack[o->actor->turn_slot]) {
            case 1:
                o->timer = 0x3C;
                o->phase = 0x34;
                break;
            case 2:
                o->steps = ENEMY_WALK_FRAMES;
                o->phase++;
                o->timer = 0x1E;
                o->step_x = (o->x - o->x2) / ENEMY_WALK_FRAMES;
                o->step_y = (o->y - o->y2) / ENEMY_WALK_FRAMES;
                break;
            }
        }
        if (o->phase != 0x17) {
            break;
        }
        if ((o->attr & BTL_OBJ_BUSY_MASK) == BTL_OBJ_BUSY) {
            break;
        }
        /* The wait is one block reached two ways, laid out ahead of the
           arm that sends the body straight home. */
        if (a->turn_move == 0) {
            if ((o->attr & ENEMY_ATTR_FLIES) == 0) {
                goto wait;
            }
            o->phase++;
        } else if ((spell->kind & 1) != 0) {
        wait:
            o->timer = 0x1E;
            o->phase++;
        } else {
            o->motion = 0;
            o->phase = 0;
            o->attr &= ~BTL_OBJ_CARRIED;
        }
        break;
    case 0x18:
        if (o->timer != 0) {
            break;
        }
        o->x -= o->step_x;
        o->y -= o->step_y;
        if (--o->steps == 0) {
            o->x = (o->col2 * ENEMY_GRID_X + ENEMY_GRID_LEFT) << 16;
            o->y = (o->row * ENEMY_GRID_Y + ENEMY_GRID_TOP) << 16;
            o->motion = 0;
            o->phase = 0;
            o->x2 = o->x;
            o->attr &= ~BTL_OBJ_CARRIED;
            o->y2 = o->y;
        }
        break;
    case 0x20:
        if (g_cd_busy != -1) {
            break;
        }
        BtlReadyNextTurn();
        BtlSoundClose(STRIKE_VOICE);
        o->motion = 0;
        o->phase = 0;
        break;
    case 0x21:
        a->resume_motion = o->motion;
        a->resume_phase = o->phase + 1;
        a->c.hp = 0;
        o->motion = STRIKE_MOTION_DOWN;
        o->timer = 0x1E;
        o->phase = 0;
        o->attr |= BTL_OBJ_CARRIED;
        break;
    case 0x22:
        if (o->timer != 0) {
            break;
        }
        o->motion = 0;
        o->phase = 0;
        break;
    }
}

/* One hit of the swing: run in phase 0x15 until the voice bank is in, then
 * once. The fighter's attack and accuracy and the target's defence are each
 * bent by the stage bytes, the blow is rolled against the target's evasion,
 * and what lands is the plain damage or the move's, less for a charmed
 * fighter, triple on a critical, capped at 9999 and passed through the
 * target's affinity. The affinity's answer decides the rest: absorbed, it
 * heals; nulled, the target flashes white and the fighter reels from its own
 * blow; otherwise the target takes it, may counter, and may take the move's
 * ailment or, on a critical that took a quarter of its hp, terror.
 */
void BtlEnemyStrike(BtlObj *o)
{
    long      pos[3];
    int       damage;
    BtlActor *a;
    BtlActor *t;
    BtlObj   *p;
    BtlObj   *miss;
    int       model;
    /* One variable for each stat's base and then the affinity's answer. Its
       last use being the answer is what keeps it, not the stat it is copied
       into, the register cse substitutes through each multiply - the image's
       a0 - and that in turn is what deals out the saved registers in the
       image's order. */
    int       n;
    int       atk;
    int       hit;
    int       def;
    int       element;
    int       crit;
    int       i;
    u_char    build;

    crit = 0;
    t = &g_btl_actors[g_btl_hit_slot];
    a = o->actor;
    model = g_persona_data[o->kind].attack;
    if (SsVabTransCompleted(SS_IMMEDIATE) == 0) {
        return;
    }
    if (a->move == ENEMY_MOVE_BOTH_DOWN) {
        pos[0] = (o->col2 * ENEMY_GRID_X + ENEMY_GRID_LEFT) << 16;
        pos[1] = (o->row * ENEMY_GRID_Y + ENEMY_GRID_TOP) << 16;
        pos[2] = 0;
        o->child = BtlSpawnMoveStrike(a->move, 1, pos);
        o->attr |= BTL_OBJ_CARRIED;
        a->resume_motion = o->motion;
        a->resume_phase = 0x20;
        a->hit_amount = 0;
        a->c.hp = 0;
        o->motion = STRIKE_MOTION_DOWN;
        o->phase = 0;
        BtlSePlay(STRIKE_VOICE, 1);
        g_btl_actors[g_btl_hit_slot].c.hp = 0;
        g_btl_actors[g_btl_hit_slot].hit_amount = 0;
        g_btl_actors[g_btl_hit_slot].obj->motion = STRIKE_MOTION_DOWN;
        g_btl_actors[g_btl_hit_slot].obj->phase = 0;
        return;
    }

    n = a->melee_atk;
    atk = n;
    if ((signed char)a->stage[3] != 0) {
        atk = n + n * ((signed char)a->stage[3] + 1) / 8;
    }
    if ((signed char)a->stage[0] != 0) {
        atk -= n * (signed char)a->stage[0] / 8;
    }
    n = a->melee_hit;
    hit = n;
    if ((signed char)a->stage[5] != 0) {
        hit = n + n * ((signed char)a->stage[5] * 2 + 2) / 8;
    }
    if ((signed char)a->stage[2] != 0) {
        hit -= n * (signed char)a->stage[2] / 8;
    }
    n = g_btl_actors[g_btl_hit_slot].defence;
    def = n;
    if ((signed char)t->stage[4] != 0) {
        def = n + n * ((signed char)t->stage[4] * 2 + 2) / 8;
    }
    if ((signed char)t->stage[1] != 0) {
        def -= n * (signed char)t->stage[1] / 8;
    }

    if (a->move != 0) {
        if (a->move == ENEMY_MOVE_ANY_ELEMENT) {
            element = rand() % 24 + 2;
        } else {
            element = g_spell_data[a->move].element;
        }
    } else {
        element = g_persona_data[a->c.key].attack + 2;
    }

    if ((g_btl_effect_obj == 0 || g_btl_effect_actor != t)
        && (a->flags & BTL_ACTOR_SCRIPT_READY) == 0
        && (g_btl_debug_max_damage != 0
            || BtlRollHit(g_btl_enemy_counted, hit, (signed char)a->c.status,
                          g_btl_party_counted,
                          g_btl_actors[g_btl_hit_slot].evade,
                          (signed char)g_btl_actors[g_btl_hit_slot].c.status)
                   != 0)) {
        if (a->move == 0) {
            damage = BtlDamageBase(atk, def);
        } else if (a->move == ENEMY_MOVE_SELF) {
            damage = a->c.hp * 2;
        } else {
            damage = BtlDamageBoosted(atk, g_spell_data[a->move].power,
                                      g_btl_actors[g_btl_hit_slot].defence);
        }
        if ((signed char)a->c.status == STATUS_CHARM) {
            damage /= STATUS_CHARM - (signed char)a->c.ail_level;
        }
        if (BtlRollCritical(a, &g_btl_actors[g_btl_hit_slot]) != 0) {
            crit = 1;
            damage *= 3;
        }
        if (a->move == ENEMY_MOVE_BUILD) {
            /* The count through a byte of its own, the clamps included -
               the same shape the round's end counts a wound with. */
            build = a->build + 1;
            a->build = build;
            if (a->build != 0) {
                if (build > 8) {
                    build = 8;
                    a->build = build;
                }
            } else {
                build = 1;
                a->build = build;
            }
            damage += damage * a->build / 8;
        }
        damage += rand() & 3;
        if (g_btl_debug_max_damage != 0) {
            damage = STRIKE_CAP;
        }
        n = BTL_REACT_NULL;
        if ((g_btl_actors[g_btl_hit_slot].flags & STRIKE_BLOCKED) == 0) {
            n = BtlApplyAffinity(&damage, element,
                                 g_btl_actors[g_btl_hit_slot].c.resist);
        }
        damage = damage < 0 ? 0 : damage > STRIKE_CAP ? STRIKE_CAP : damage;

        switch (n) {
        case BTL_REACT_REPEL:
            BtlSePlay(STRIKE_SE_SLOT, STRIKE_SE_REPEL);
            pos[0] = (g_btl_actors[g_btl_hit_slot].obj->col2 * ENEMY_GRID_X
                      + ENEMY_GRID_LEFT) << 16;
            pos[1] = (g_btl_actors[g_btl_hit_slot].obj->row * ENEMY_GRID_Y
                      + ENEMY_GRID_TOP) << 16;
            if (g_btl_hit_slot < BTL_PARTY) {
                pos[1] += STRIKE_MEMBER_DROP;
            }
            pos[2] = 0;
            /* The strike is reached back through `child`, not a local of its
               own: cse then uses the call's answer where it is, where a local
               copies it out of v0 first. */
            if (a->move == 0) {
                o->child = BtlSpawnStrike(1, model, pos);
                o->child->z -= STRIKE_LIFT;
            } else {
                o->child = BtlSpawnMoveStrike(a->move, 1, pos);
            }
            g_btl_actors[g_btl_hit_slot].hit_amount = damage;
            g_btl_actors[g_btl_hit_slot].c.hp += damage;
            g_btl_actors[g_btl_hit_slot].c.hp =
                g_btl_actors[g_btl_hit_slot].c.hp
                        > g_btl_actors[g_btl_hit_slot].c.hp_max
                    ? g_btl_actors[g_btl_hit_slot].c.hp_max
                    : g_btl_actors[g_btl_hit_slot].c.hp;
            g_btl_actors[g_btl_hit_slot].obj->motion = STRIKE_MOTION_HEAL;
            break;
        case BTL_REACT_NULL:
            o->attr |= BTL_OBJ_CARRIED;
            BtlSoundClose(STRIKE_VOICE);
            BtlSoundOpen(g_btl_slot_banks, STRIKE_VOICE, (o->tpage >> 1) - 5);
            BtlSePlay(STRIKE_SE_SLOT, STRIKE_SE_NULL);
            g_btl_clut_fading |= 1 << g_btl_hit_slot;
            i = 1;
            do {
                g_btl_actor_clut[g_btl_hit_slot * FX_CLUT_COLORS + i] =
                    FX_CLUT_WHITE;
                i++;
            } while (i < FX_CLUT_COLORS);
            if (a->move == 0) {
                o->child = BtlSpawnStrike(0, model, &o->x);
                o->child->z -= STRIKE_LIFT;
            } else {
                o->child = BtlSpawnMoveStrike(a->move, 0, &o->x);
            }
            a->hit_amount = damage;
            a->c.hp -= damage;
            if (a->c.hp <= 0) {
                o->actor->resume_motion = o->motion;
                o->actor->resume_phase = 0x20;
                a->c.hp = 0;
                o->motion = STRIKE_MOTION_DOWN;
                BtlSePlay(STRIKE_VOICE, 1);
            } else {
                o->actor->resume_motion = o->motion;
                o->actor->resume_phase = o->phase + 1;
                o->motion = STRIKE_MOTION_REEL;
                BtlSePlay(STRIKE_VOICE, 0);
            }
            o->phase = 0xFF;
            break;
        default:
            if ((g_btl_actors[g_btl_hit_slot].flags & BTL_ACTOR_FLINCHED) != 0) {
                damage /= 2;
            }
            pos[0] = (g_btl_actors[g_btl_hit_slot].obj->col2 * ENEMY_GRID_X
                      + ENEMY_GRID_LEFT) << 16;
            pos[1] = (g_btl_actors[g_btl_hit_slot].obj->row * ENEMY_GRID_Y
                      + ENEMY_GRID_TOP) << 16;
            if (g_btl_hit_slot < BTL_PARTY) {
                pos[1] += STRIKE_MEMBER_DROP;
            }
            pos[2] = 0;
            if (a->move == 0) {
                o->child = BtlSpawnStrike(1, model, pos);
                o->child->z -= STRIKE_LIFT;
            } else {
                o->child = BtlSpawnMoveStrike(a->move, 1, pos);
            }
            if (a->move == ENEMY_MOVE_SELF
                && g_btl_actors[g_btl_hit_slot].c.hp - damage <= 0) {
                g_btl_actors[g_btl_hit_slot].hit_amount =
                    g_btl_actors[g_btl_hit_slot].c.hp - 1;
                g_btl_actors[g_btl_hit_slot].c.hp = 1;
            } else {
                g_btl_actors[g_btl_hit_slot].hit_amount = damage;
                g_btl_actors[g_btl_hit_slot].c.hp -= damage;
            }
            if (damage != 0) {
                if (crit != 0) {
                    BtlSePlay(STRIKE_SE_SLOT, STRIKE_SE_CRITICAL);
                }
                if (a->move == ENEMY_MOVE_WOUND) {
                    g_btl_actors[g_btl_hit_slot].flags |= BTL_ACTOR_WOUND;
                }
                if (g_btl_debug_flags[1] != 0
                    && g_btl_actors[g_btl_hit_slot].c.hp <= 0) {
                    g_btl_actors[g_btl_hit_slot].c.hp = 1;
                }
                if (g_btl_actors[g_btl_hit_slot].c.hp <= 0) {
                    g_btl_actors[g_btl_hit_slot].c.hp = 0;
                    g_btl_actors[g_btl_hit_slot].obj->motion = STRIKE_MOTION_DOWN;
                    BtlSePlay(STRIKE_VOICE, 1);
                } else {
                    if (a->move != ENEMY_MOVE_SELF
                        && (signed char)g_btl_actors[g_btl_hit_slot].c.status
                               == BTL_STATUS_COUNTER
                        && g_btl_actors[g_btl_hit_slot].counter == 0
                        && (g_btl_actors[g_btl_hit_slot].flags
                            & BTL_ACTOR_REFUSED)
                               == 0) {
                        BtlInsertTurn(g_btl_hit_slot);
                        g_btl_actors[g_btl_hit_slot].counter = 1;
                        g_btl_actors[g_btl_hit_slot].counter_slot =
                            a->obj->mark_num;
                        if (g_btl_actors[g_btl_hit_slot].action == 0) {
                            g_btl_actors[g_btl_hit_slot].counter_turn = 1;
                        } else {
                            g_btl_actors[g_btl_hit_slot].counter_turn = 0;
                        }
                        g_btl_actors[g_btl_hit_slot].action = 0xFF;
                    }
                    g_btl_actors[g_btl_hit_slot].obj->motion = STRIKE_MOTION_HURT;
                    BtlSePlay(STRIKE_VOICE, 0);
                    if (g_spell_data[a->move].ailment != 0 && (rand() & 7) == 0) {
                        BtlInflictStatus(&g_btl_actors[g_btl_hit_slot],
                                         g_spell_data[a->move].ailment);
                    }
                    if (crit != 0 && damage >= t->c.hp_max / 4
                        && rand() % 3 == 0) {
                        BtlInflictStatus(t, STATUS_TERROR);
                    }
                }
                g_btl_actors[g_btl_hit_slot].obj->phase = 0;
            } else {
                o->timer = 0x1E;
            }
            break;
        }
    } else {
        BtlSePlay(STRIKE_SE_SLOT, STRIKE_SE_MISS);
        pos[0] = g_btl_actors[g_btl_hit_slot].obj->x;
        pos[1] = g_btl_actors[g_btl_hit_slot].obj->y;
        pos[2] = 0;
        if (a->move == 0) {
            o->child = BtlSpawnStrike(1, model, pos);
            o->child->z -= STRIKE_LIFT;
        } else {
            o->child = BtlSpawnMoveStrike(a->move, 1, pos);
        }
        o->timer = 0x1E;
        if (g_btl_effect_obj == 0
            || g_btl_effect_actor != &g_btl_actors[g_btl_hit_slot]) {
            g_btl_actors[g_btl_hit_slot].obj->motion = STRIKE_MOTION_MISS;
            pos[2] = g_btl_actors[g_btl_hit_slot].obj->z - STRIKE_LIFT;
            miss = BtlSpawnMiss(pos);
            miss->mark_num = 0xFF;
            for (p = g_btl_obj_pool; p != 0; p = p->next) {
                if (p->kind == MARK_KIND_STILL && p->mark_num == o->mark_num) {
                    BtlObjMoveBefore(p, miss);
                    break;
                }
            }
            miss->mark_num = o->mark_num;
        }
    }
    o->phase++;
}
