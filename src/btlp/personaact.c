/* Persona 1 (JP) - the two ways a summoned Persona plays its move out.
 * BTLP only.
 *   0x800B12D0 BtlPersonaPlayMove   0x800B16BC BtlPersonaSpellMove
 *
 * BtlPersonaMotion02 picks between them by the move the caster is making: the
 * moves from 0xA3 up, bar 0xDB, take the spell below and everything else the
 * plain one.
 *
 * The plain move is one effect: it is started, the Persona is greyed while it
 * runs, the caster's own voice bank is opened again afterwards, and what the
 * move cost the caster is settled - the ailment it leaves, or the hp it pays,
 * with the fall or the reel that goes with it.
 *
 * The spell is the same move dealt out hit by hit. The caster's target mask is
 * walked a slot at a time, each hit opens the target's bank and is handed to
 * the arm that plays it, and the walk goes round again until the rolled number
 * of hits is spent or nothing is left to aim at.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libsnd.h>
#include <persona/common/spell.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/cast.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/spellfx.h>
#include <persona/btlp/status.h>
#include <persona/btlp/strike.h>

/* The move a Persona throws itself away with: the caster falls where it
   stands and the move is over. */
#define PERSONA_MOVE_SELF 0x6D

/* And the one an enemy's Persona spends its own life on, which the spell's
   tail settles the same way. */
#define PERSONA_MOVE_SPEND 0xDF

/* What the Persona is greyed to while the move's effect runs, how fast it
   walks there, and the motion the effect is set going on. */
#define PERSONA_MOVE_GREY   0x40
#define PERSONA_MOVE_FADE   2
#define PERSONA_FX_MOTION   2

/* The phases the spell's walk runs in: the target picked, the hit sounded,
   the hit played out, the hit closed, and the two the move ends on. */
#define PERSONA_PHASE_PICK  0x13
#define PERSONA_PHASE_SOUND 0x14
#define PERSONA_PHASE_PLAY  0x15
#define PERSONA_PHASE_SHUT  0x16
#define PERSONA_PHASE_END   0x17

/* How a move is aimed, in g_spell_data's aim byte: at the one fighter, at
   everything the mask covers once the walk runs out, and a hit that does not
   count against the number rolled. */
#define PERSONA_AIM_ONE   1
#define PERSONA_AIM_AGAIN 2
#define PERSONA_AIM_FREE  4

/* The motion the caster is left on once the move is done. */
#define PERSONA_MOTION_DONE 4

/* How long the fall is held before the turn goes on. */
#define PERSONA_FALL_HOLD 0x1E

/* The fall's first arm steps the phase itself and breaks, so only the
   increment is shared with the arms below it and the arm reads the phase
   before it jumps, as in the image. The reel's ailment turns are set after
   its hit amount. */
void BtlPersonaPlayMove(BtlObj *o)
{
    BtlActor *a;
    BtlObj   *obj;

    a = o->actor;
    obj = a->obj;
    if ((o->attr & BTL_OBJ_TRAIL) != 0) {
        return;
    }
    switch (o->phase) {
    case 0:
        if ((o->attr & BTL_OBJ_BUSY) != 0) {
            break;
        }
        g_btl_seq_catchup = 0;
        o->child = BtlStartMoveFx(o->actor->move);
        o->child->actor = g_btl_persona_obj->actor;
        o->child->motion = PERSONA_FX_MOTION;
        BtlObjSetRgb(o, PERSONA_MOVE_GREY, PERSONA_MOVE_GREY, PERSONA_MOVE_GREY);
        o->fade = PERSONA_MOVE_FADE;
        o->phase++;
        break;

    case 1:
        if (o->child->motion != 0) {
            break;
        }
        BtlObjFree(o->child);
        if (a->move == PERSONA_MOVE_SELF
            || (a->obj->attr & BTL_OBJ_CARRIED) != 0) {
            if (a->obj->mark_num < BTL_PARTY) {
                BtlSoundOpen(g_btl_banks, STRIKE_VOICE, a->c.key);
            } else {
                BtlSoundOpen(g_btl_slot_banks, STRIKE_VOICE,
                             (a->obj->tpage >> 1) - 5);
            }
        }
        o->phase++;
        break;

    case 2:
        if (SsVabTransCompleted(SS_IMMEDIATE) == 0) {
            break;
        }
        if (a->move == PERSONA_MOVE_SELF) {
            obj->attr |= BTL_OBJ_CARRIED;
            a->resume_motion = obj->motion;
            a->resume_phase = obj->phase;
            a->hit_amount = 0;
            a->c.hp = 0;
            obj->motion = STRIKE_MOTION_DOWN;
            BtlSePlay(STRIKE_VOICE, 1);
            obj->phase = 0;
            o->timer = PERSONA_FALL_HOLD;
            o->phase++;
            break;
        } else if ((a->obj->attr & BTL_OBJ_CARRIED) != 0
                   && (signed char)a->c.status != BTL_STATUS_NOINPUT) {
            if (a->unkCC != 0) {
                if (BtlInflictStatus(a, a->unkCC) != 0) {
                    if ((signed char)a->c.status == BTL_STATUS_DOWN) {
                        a->resume_motion = obj->motion;
                        a->resume_phase = obj->phase;
                        a->hit_amount = 0;
                        a->c.hp = 0;
                        obj->motion = STRIKE_MOTION_DOWN;
                        a->obj->phase = 0;
                        BtlSePlay(STRIKE_VOICE, 1);
                        obj->phase = 0;
                    } else {
                        a->unkDF = 1;
                        a->resume_motion = obj->motion;
                        a->resume_phase = obj->phase;
                        a->hit_amount = 0;
                        a->ail_turns = 2;
                        obj->motion = STRIKE_MOTION_REEL;
                        BtlSePlay(STRIKE_SE_SLOT, 5);
                        o->timer = PERSONA_FALL_HOLD;
                        obj->phase = 0;
                    }
                } else {
                    a->obj->attr &= ~BTL_OBJ_CARRIED;
                }
            } else {
                a->c.hp -= a->hit_amount;
                if (g_btl_debug_flags[1] != 0 && a->c.hp <= 0) {
                    a->c.hp = 1;
                }
                if (a->c.hp <= 0) {
                    a->resume_motion = obj->motion;
                    a->resume_phase = obj->phase;
                    a->c.hp = 0;
                    obj->motion = STRIKE_MOTION_DOWN;
                    BtlSePlay(STRIKE_VOICE, 1);
                    g_btl_hits_left = 0;
                } else {
                    a->unkDF = 1;
                    a->resume_motion = obj->motion;
                    a->resume_phase = obj->phase;
                    obj->motion = STRIKE_MOTION_REEL;
                    BtlSePlay(STRIKE_VOICE, 0);
                }
                o->timer = PERSONA_FALL_HOLD;
                obj->phase = 0;
            }
        }
        o->phase++;
        break;

    case 3:
        if (o->timer != 0) {
            break;
        }
        BtlSoundClose(STRIKE_VOICE);
        BtlObjSetMotion(o, PERSONA_MOTION_DONE);
        BtlObjSetPhase(o, 0);
        break;
    }
}

/* 99.80%. The walk, its shared tail and the second pass over the mask are the
   image's - the second pass steps its one counter past the test rather than
   before it, which is what the image does and what lets gcc reduce the same
   counter to the record's own stride. What is left is one branch: the arm that
   aims at a single slot tests the same three things the image tests, and gcc
   threads the jump to the shared "nothing to hit" tail into the branch where
   the image leaves it standing - written as three ors with the goto, as an
   and with the else, and with the flag set ahead of the test, all three come
   out the same, and so does `continue` with the goto after it. The table's
   name comes right when the rodata is carved, which
   waits on the match. */
#ifdef NON_MATCHING
void BtlPersonaSpellMove(BtlObj *o)
{
    BtlActor        *a;
    BtlObj          *obj;
    const SpellData *sp;
    u_short          targets;
    int              picked;
    u_short          held;
    int              i;
    int              slot;

    a = o->actor;
    obj = a->obj;
    sp = &g_spell_data[a->move];
    switch (o->phase) {
    case 0:
        if ((o->attr & BTL_OBJ_BUSY) != 0) {
            return;
        }
        obj->attr &= ~BTL_OBJ_CARRIED;
        g_btl_hit_slot = a->order;
        g_btl_hits_left = BtlRollHits(sp->cost);
        if (g_btl_actors[g_btl_hit_slot].c.key == 0) {
            if (BtlMarkMoveArea(a, sp->target, sp->aim) < 0) {
                o->phase = PERSONA_PHASE_END;
                return;
            }
            g_btl_hit_slot = BtlSlowestOrder();
            if (g_btl_hit_slot < 0) {
                o->phase = PERSONA_PHASE_END;
                return;
            }
            g_btl_hit_slot += BTL_PARTY;
            a->order = g_btl_hit_slot;
        }
        o->phase++;
        return;

    case 1:
        if ((o->attr & BTL_OBJ_BUSY) != 0) {
            return;
        }
        g_btl_hit_mask = 1;
        g_btl_hit_walk = 0;
        a->targets &= ~(1 << g_btl_hit_slot);
        o->phase = PERSONA_PHASE_SOUND;
        return;

    case PERSONA_PHASE_PICK:
        do {
            picked = 0;
            if ((sp->aim & PERSONA_AIM_ONE) != 0) {
                if (g_btl_actors[g_btl_hit_slot].c.key != 0
                    && (signed char)g_btl_actors[g_btl_hit_slot].c.status
                           != BTL_STATUS_DOWN
                    && (g_btl_actors[g_btl_hit_slot].flags & BTL_ACTOR_OUT)
                           == 0) {
                    picked = 1;
                } else {
                    goto none;
                }
            } else {
                for (; g_btl_hit_walk < BTL_ACTORS;
                     g_btl_hit_walk++, g_btl_hit_mask *= 2) {
                    if ((a->targets & g_btl_hit_mask) != 0
                        && g_btl_actors[g_btl_hit_walk].c.key != 0
                        && (signed char)g_btl_actors[g_btl_hit_walk].c.status
                               != BTL_STATUS_DOWN
                        && (g_btl_actors[g_btl_hit_walk].flags & BTL_ACTOR_OUT)
                               == 0) {
                        g_btl_hit_slot = g_btl_hit_walk;
                        picked = 1;
                        break;
                    }
                }
                if (g_btl_hit_walk < BTL_ACTORS) {
                    continue;
                }
                picked = 1;
                if ((sp->aim & PERSONA_AIM_AGAIN) != 0) {
                    a->targets |= 1 << a->order;
                    i = 0;
                    held = g_btl_hits_left;
                    g_btl_hits_left = 0;
                    do {
                        if (((a->targets >> i) & 1) != 0
                            && g_btl_actors[i].c.key != 0
                            && (signed char)g_btl_actors[i].c.status
                                   != BTL_STATUS_DOWN
                            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
                            picked = 0;
                            g_btl_hits_left = held;
                            g_btl_hit_walk = 0;
                            g_btl_hit_mask = 1;
                        }
                        i++;
                    } while (i < BTL_ACTORS);
                } else {
                none:
                    g_btl_hits_left = 0;
                    picked = 1;
                }
            }
        } while (picked == 0);
        o->phase++;
        /* fall through */

    case PERSONA_PHASE_SOUND:
        if (g_btl_hits_left <= 0) {
            BtlSoundClose(STRIKE_VOICE);
            o->phase = PERSONA_PHASE_END;
            return;
        }
        if (g_btl_hit_slot >= BTL_PARTY) {
            BtlSoundOpen(g_btl_slot_banks, STRIKE_VOICE,
                         (g_btl_actors[g_btl_hit_slot].obj->tpage >> 1) - 5);
        } else {
            BtlSoundOpen(g_btl_banks, STRIKE_VOICE,
                         g_btl_actors[g_btl_hit_slot].c.key);
        }
        o->phase++;
        return;

    case PERSONA_PHASE_PLAY:
        BtlPersonaSpell15(o);
        return;

    case PERSONA_PHASE_SHUT:
        if (g_btl_actors[g_btl_hit_slot].obj->motion != 0 || o->timer != 0) {
            return;
        }
        BtlSoundClose(STRIKE_VOICE);
        if ((sp->aim & PERSONA_AIM_FREE) == 0) {
            g_btl_hits_left--;
        }
        if (g_btl_hit_walk < 0) {
            g_btl_hit_mask = 1;
        } else {
            g_btl_hit_mask *= 2;
        }
        g_btl_hit_walk++;
        o->phase = PERSONA_PHASE_PICK;
        return;

    case PERSONA_PHASE_END:
        if (a->move == PERSONA_MOVE_SPEND) {
            a->resume_motion = obj->motion;
            a->resume_phase = obj->phase;
            a->hit_amount = 0;
            a->c.hp = 0;
            obj->motion = STRIKE_MOTION_DOWN;
            obj->phase = 0;
            obj->attr |= BTL_OBJ_CARRIED;
            o->timer = PERSONA_FALL_HOLD;
            o->phase++;
            return;
        }
        BtlObjSetMotion(o, PERSONA_MOTION_DONE);
        BtlObjSetPhase(o, 0);
        break;

    case PERSONA_PHASE_END + 1:
        if (o->timer != 0) {
            return;
        }
        BtlObjSetMotion(o, PERSONA_MOTION_DONE);
        BtlObjSetPhase(o, 0);
        break;
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/personaact", BtlPersonaSpellMove);
#endif
