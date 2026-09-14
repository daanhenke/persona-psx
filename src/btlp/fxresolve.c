/* Persona 1 (JP) - what a spell does to the fighter it reaches.  BTLP only.
 *   0x800C2FDC BtlFxResolveHit  0x800C42B4 BtlFxReopenVoices
 *
 * BtlFxResolveHit is the plain finish's resolve phase, run once for each
 * fighter the chain walks to. It rolls whether the spell lands - a member's
 * against the enemies, an enemy's the other way, and never for a scripted turn
 * - and a spell that misses, or one the fighter is not there to take, only
 * puts the fighter's colour back and moves the walk on.
 *
 * One that lands is worth, for the two kinds that can take a fighter outright,
 * all of its hp against the one species that has no answer to it and a share
 * of its most by the Persona rank it stands at otherwise; for move 0xE0 what
 * the grab has been set to; for 0xEC all but one of its hp; and for everything
 * else the caster's squared stat scaled by the spell's weight and divided down
 * by the fighter's own. A ward that turns the element aside sends half of it
 * back as healing; an affinity that shrugs it off washes the fighter white and
 * leaves the acting fighter holding it. Otherwise the fighter loses it - more
 * for a frozen or shocked fighter hit by the element that breaks it, all of it
 * on a lucky roll for the four moves that can - and either reels or goes down.
 * Then four moves try an ailment of their own on a fighter left reeling, and
 * the elements that carry one try it.
 *
 * BtlFxReopenVoices is what the plain and the ailment finishes call before
 * they walk: it opens the sound banks the next phase will speak from.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <rand.h>
#include <libsnd.h>
#include <persona/common/spell.h>
#include <persona/common/status.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/damage.h>
#include <persona/btlp/object.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sides.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/spellfx.h>
#include <persona/btlp/stats.h>
#include <persona/btlp/status.h>

/* The two kinds of spell that can take a fighter outright, the moves of each
   that are worked out as damage all the same, and the species each takes
   whole. */
#define FX_HIT_KIND_A     0x10
#define FX_HIT_KIND_B     0x18
#define FX_HIT_A_FIRST    0x31
#define FX_HIT_B_FIRST    0x4D
#define FX_HIT_SPARED     3
#define FX_HIT_SPECIES_A  4
#define FX_HIT_SPECIES_B  0x12
#define FX_HIT_KEY_SPARED 0x98

/* What a fighter is left carrying after either kind lands, and what keeps it
   from carrying either. */
#define FX_HIT_MARK_A     0x40000
#define FX_HIT_MARK_B     0x80000
#define FX_HIT_MARKS      (FX_HIT_MARK_A | FX_HIT_MARK_B)
#define FX_HIT_MARK_GUARD 0x400000

/* The moves with a rule of their own. */
#define FX_HIT_PIERCE 0x6E
#define FX_HIT_BIND   0xA0
#define FX_HIT_TAKE   0xE0
#define FX_HIT_SPARE  0xEC

/* What the two Personas that halve move 0x6E are, and what it does
   otherwise. */
#define FX_HIT_PIERCE_KEY_A 0x31
#define FX_HIT_PIERCE_KEY_B 0x60
#define FX_HIT_PIERCE_HALF  500
#define FX_HIT_PIERCE_FULL  999

/* The fighter whose ALT turn only takes a quarter. */
#define FX_HIT_ALT_KEY 0xB8

/* The wards that keep a spell off altogether, and the elements the other two
   turn back. */
#define FX_HIT_BLOCKED (BTL_ACTOR_WARD_8E | BTL_ACTOR_WARD_8F | BTL_ACTOR_5D)
#define FX_HIT_FIRE    0x12
#define FX_HIT_ICE     0x13
#define FX_HIT_ELEC    0x16
#define FX_HIT_NERVE   0x1B

#define FX_HIT_CAP 9999

/* The four moves that can take everything on a lucky roll, in two pairs. */
#define FX_HIT_LUCK_A1 0x0F
#define FX_HIT_LUCK_A2 0x27
#define FX_HIT_LUCK_B1 0x12
#define FX_HIT_LUCK_B2 0x2A
#define FX_HIT_LUCK_A  0xF
#define FX_HIT_LUCK_B  0x3F

/* The attribute a reeling fighter's object is given. */
#define FX_HIT_STRUCK 0x200000

/* The motions and sounds. */
#define FX_HIT_HEAL_MOTION 0xF
#define FX_HIT_HURT_MOTION 7
#define FX_HIT_DOWN_MOTION 8
#define FX_HIT_SE_SLOT     2
#define FX_HIT_SE_REPEL    0xB
#define FX_HIT_SE_NULL     0xA

/* The four moves that try an ailment of their own on a reeling fighter. */
#define FX_HIT_SLEEP    0xE9
#define FX_HIT_FREEZE   0xEB
#define FX_HIT_PALYZE   0xE6
#define FX_HIT_ANY      0xF0
#define FX_HIT_ROLL     0xFF
#define FX_HIT_LEVEL    2

/* The damage a spell does: the caster's stat squared, scaled by the spell's
   weight, over six times the target's own. */
#define FX_HIT_SCALED(d, a)                                                    \
    ((d) * ((g_spell_data[g_btl_fx_move].power / 2.0 + 5.0) / 10.0)          \
     / ((a)->unk3C * 6))

/* 98.62%: every instruction is in place but the saved registers are dealt
   out in another order - the target, the caster and the stat trade s1, s5
   and s2 for s2, s3 and s5 - and the formula's last two argument copies and
   the clear of `react` sit one slot apart. The switches, the formula split
   through `damage` and the reversed take arm were each worth a structural
   cluster; declaration order moves nothing. */
#ifdef NON_MATCHING
void BtlFxResolveHit(BtlObj *o)
{
    BtlActor *self;
    BtlActor *a;
    int       amount;
    int       hit;
    int       power;
    int       react;
    int       instant;
    int       reflect;
    int       voice;
    int       chance;
    int       n;
    int       i;
    double    damage;

    reflect = 0;
    amount = 0;
    self = o->actor;
    a = &g_btl_actors[g_btl_hit_slot];
    hit = 0;
    if ((self->flags & BTL_ACTOR_SCRIPT_DONE) != 0
        || (g_btl_fx_move == FX_HIT_PIERCE
            && BtlApplyAffinity(&amount, g_spell_data[FX_HIT_PIERCE].element,
                                a->c.unk5C) != 0)) {
        hit = 1;
    }
    if (SsVabTransCompleted(SS_IMMEDIATE) == 0) {
        return;
    }

    power = self->unk3A;
    if ((signed char)self->stage[6] != 0) {
        power += power * ((signed char)self->stage[6] + 1) / 8;
    }

    if ((self->obj->attr & BTL_OBJ_OTHER_SIDE) == 0) {
        if (BtlRollHit(g_btl_party_counted, self->melee_hit,
                       (signed char)self->c.status, g_btl_enemy_counted,
                       a->evade, (signed char)a->c.status)) {
            hit = 1;
        }
    } else if ((self->flags & BTL_ACTOR_SCRIPT_READY) == 0) {
        if ((self->flags & BTL_ACTOR_SCRIPT_ALT) != 0
            || g_btl_fx_move == FX_HIT_TAKE
            || BtlRollHit(g_btl_enemy_counted, self->melee_hit,
                          (signed char)self->c.status, g_btl_party_counted,
                          a->evade, (signed char)a->c.status)) {
            hit = 1;
        }
    }

    if ((g_spell_data[g_btl_fx_move].kind & SPELL_KIND_MASK) == FX_HIT_KIND_A
        || (g_spell_data[g_btl_fx_move].kind & SPELL_KIND_MASK)
               == FX_HIT_KIND_B) {
        if ((a->flags & BTL_ACTOR_5C) != 0 || a->c.key >= FX_HIT_KEY_SPARED) {
            hit = 0;
            a->unkDE = 1;
        }
    }
    if (g_btl_fx_move == FX_HIT_PIERCE
        && (g_btl_no_escape != 0 || (a->flags & BTL_ACTOR_5C) != 0)) {
        hit = 0;
        a->unkDE = 1;
    }
    if (g_btl_fx_move == FX_HIT_BIND && (a->flags & BTL_ACTOR_5C) != 0) {
        hit = 0;
        a->unkDE = 1;
    }
    if (g_btl_fx_move == FX_HIT_SPARE) {
        hit = g_btl_no_escape == 0;
    }
    if (g_btl_effect_obj != NULL && g_btl_effect_actor == a) {
        hit = 0;
    }

    if (a->unkD3 == 0 && (hit != 0 || g_btl_act_kind != 0)) {
        switch (g_spell_data[g_btl_fx_move].kind & SPELL_KIND_MASK) {
        case FX_HIT_KIND_A:
            if ((u_int)(g_btl_fx_move - FX_HIT_A_FIRST) < FX_HIT_SPARED) {
                instant = 0;
                damage = power * power;
                amount = FX_HIT_SCALED(damage, a);
                react = 0;
            } else {
                instant = 1;
                if (a->species == FX_HIT_SPECIES_A) {
                    amount = a->c.hp;
                    react = 1;
                } else {
                    switch (a->persona_rank) {
                    case 0:
                    case 1:
                    case 2:
                        amount = a->c.hp_max / 8;
                        react = 1;
                        break;
                    case 3:
                        amount = 0;
                        react = 1;
                        break;
                    case 4:
                        amount = a->c.hp_max / 2;
                        react = 1;
                        break;
                    }
                }
            }
            break;
        case FX_HIT_KIND_B:
            if ((u_int)(g_btl_fx_move - FX_HIT_B_FIRST) < FX_HIT_SPARED) {
                instant = 0;
                damage = power * power;
                amount = FX_HIT_SCALED(damage, a);
                react = 0;
            } else {
                instant = 1;
                if (a->species == FX_HIT_SPECIES_B) {
                    amount = a->c.hp;
                    react = 1;
                } else {
                    switch (a->persona_rank) {
                    case 0:
                    case 1:
                    case 2:
                        amount = a->c.hp_max / 8;
                        react = 1;
                        break;
                    case 3:
                        amount = a->c.hp_max / 2;
                        react = 1;
                        break;
                    case 4:
                        amount = 0;
                        react = 1;
                        break;
                    }
                }
            }
            break;
        default:
            if (g_btl_fx_move == FX_HIT_TAKE) {
            switch (self->unkD8) {
            case 0:
                amount = 0;
                break;
            case 1:
                amount = FX_HIT_PIERCE_FULL;
                break;
            default:
                if (self->c.hp >= a->c.hp) {
                    amount = a->c.hp - 1;
                } else {
                    amount = self->c.hp;
                }
                break;
            }
            react = 0;
        } else if (g_btl_fx_move == FX_HIT_SPARE) {
            amount = a->c.hp - 1;
            react = 1;
        } else {
            damage = power * power;
            amount = FX_HIT_SCALED(damage, a);
            react = 0;
        }
            break;
        }
    } else {
        BtlObjStatusTint(a->obj);
        o->timer = 0;
        o->phase++;
        return;
    }

    if ((a->flags & FX_HIT_BLOCKED) != 0) {
        react = BTL_REACT_NULL;
    }
    if (((a->flags & BTL_ACTOR_WARD_8D) != 0
         && g_spell_data[g_btl_fx_move].element == FX_HIT_FIRE)
        || ((a->flags & BTL_ACTOR_WARD_8C) != 0
            && g_spell_data[g_btl_fx_move].element == FX_HIT_ICE)) {
        react = BTL_REACT_REPEL;
        reflect = 1;
    }
    if (react == 0) {
        if (g_btl_fx_move != FX_HIT_TAKE) {
            if ((a->flags & BTL_ACTOR_FLINCHED) != 0) {
                amount /= 2;
            }
            amount += rand() & 3;
        }
        react = BtlApplyAffinity(&amount, g_spell_data[g_btl_fx_move].element,
                                 a->c.unk5C);
    }
    if (self->obj->motion != 3 && (signed char)self->c.status == STATUS_CLOSE
        && (signed char)self->c.ail_level > 0) {
        amount /= 2;
    }
    if (g_btl_fx_move == FX_HIT_PIERCE) {
        if (g_btl_act_kind != 0) {
            n = g_btl_personas[BtlActorPersona(g_btl_actor_turn)].key;
            if (n == FX_HIT_PIERCE_KEY_A || n == FX_HIT_PIERCE_KEY_B) {
                amount = FX_HIT_PIERCE_HALF;
            } else {
                amount = FX_HIT_PIERCE_FULL;
            }
        } else {
            amount = a->c.hp / 2;
            a->flags = (a->flags & ~FX_HIT_MARKS) | FX_HIT_MARK_GUARD;
        }
    }
    if ((self->flags & BTL_ACTOR_SCRIPT_DONE) != 0) {
        amount = a->c.hp;
        react = 0;
    }
    if ((self->flags & BTL_ACTOR_SCRIPT_ALT) != 0) {
        amount = self->c.key == FX_HIT_ALT_KEY ? a->c.hp / 4 : a->c.hp / 2;
        react = 0;
    }

    if (amount < 0) {
        amount = -amount;
    }
    if (amount >= 0) {
        n = amount;
        if (n > FX_HIT_CAP) {
            n = FX_HIT_CAP;
        }
    } else {
        n = 0;
    }
    amount = n;

    switch (react) {
    case BTL_REACT_REPEL:
        BtlSePlay(FX_HIT_SE_SLOT, FX_HIT_SE_REPEL);
        if (reflect) {
            amount /= 2;
        }
        a->c.hp += amount;
        a->hit_amount = amount;
        a->c.hp = a->c.hp > a->c.hp_max ? a->c.hp_max : a->c.hp;
        a->obj->motion = FX_HIT_HEAL_MOTION;
        a->obj->phase = 0;
        switch (g_spell_data[g_btl_fx_move].kind & SPELL_KIND_MASK) {
        case FX_HIT_KIND_A:
            a->flags &= ~FX_HIT_MARKS;
            if ((a->flags & FX_HIT_MARK_GUARD) == 0) {
                a->flags |= FX_HIT_MARK_A;
            }
            break;
        case FX_HIT_KIND_B:
            a->flags &= ~FX_HIT_MARKS;
            if ((a->flags & FX_HIT_MARK_GUARD) == 0) {
                a->flags |= FX_HIT_MARK_B;
            }
            break;
        }
        break;
    case BTL_REACT_NULL:
        BtlSePlay(FX_HIT_SE_SLOT, FX_HIT_SE_NULL);
        g_btl_clut_fading |= 1 << g_btl_hit_slot;
        i = 1;
        do {
            g_btl_actor_clut[g_btl_hit_slot * FX_CLUT_COLORS + i] =
                FX_CLUT_WHITE;
            i++;
        } while (i < FX_CLUT_COLORS);
        BtlObjStatusTint(a->obj);
        self->obj->attr |= BTL_OBJ_CARRIED;
        if (self->c.key >= BTL_KEY_DEMON) {
            amount /= 4;
        }
        self->hit_amount += amount;
        break;
    default:
        if (amount != 0) {
        switch (g_spell_data[g_btl_fx_move].kind & SPELL_KIND_MASK) {
        case FX_HIT_KIND_A:
            a->flags &= ~FX_HIT_MARKS;
            if ((a->flags & FX_HIT_MARK_GUARD) == 0) {
                a->flags |= FX_HIT_MARK_A;
                a->padCD[2] = instant;
            }
            break;
        case FX_HIT_KIND_B:
            a->flags &= ~FX_HIT_MARKS;
            if ((a->flags & FX_HIT_MARK_GUARD) == 0) {
                a->flags |= FX_HIT_MARK_B;
                a->padCD[2] = instant;
            }
            break;
        }
        if ((g_spell_data[g_btl_fx_move].element == FX_HIT_ICE
             && (signed char)a->c.status == STATUS_SHOCK)
            || (g_spell_data[g_btl_fx_move].element == FX_HIT_ELEC
                && (signed char)a->c.status == STATUS_FREEZE)) {
            switch ((signed char)a->c.ail_level) {
            case 0:
                amount += amount / 8;
                break;
            case 1:
                amount += amount / 4;
                break;
            case 2:
                amount += amount / 2;
                break;
            }
        }
        if (g_btl_no_escape == 0) {
            if ((g_btl_fx_move == FX_HIT_LUCK_A1
                 || g_btl_fx_move == FX_HIT_LUCK_A2)
                && a->c.level < self->c.level
                && (rand() & FX_HIT_LUCK_A) == 0) {
                amount = a->c.hp;
            }
            if ((g_btl_fx_move == FX_HIT_LUCK_B1
                 || g_btl_fx_move == FX_HIT_LUCK_B2)
                && a->c.level < self->c.level
                && (rand() & FX_HIT_LUCK_B) == 0) {
                amount = a->c.hp;
            }
        }
        if ((signed char)a->c.status == BTL_STATUS_LIFTED) {
            amount = amount * 150 / 100;
            a->c.status = 0;
            a->c.ail_level = 0;
        }
        a->c.hp -= amount;
        a->hit_amount = amount;
        self->damage_dealt += amount;
        if (g_btl_hit_slot < BTL_PARTY) {
            voice = g_btl_hit_slot + 7;
            if (g_btl_debug_flags[1] != 0 && a->c.hp <= 0) {
                a->c.hp = 1;
            }
        } else {
            voice = (a->obj->unkCD >> 1) + 2;
            BtlOfferScoreEnemy(g_btl_hit_slot - BTL_PARTY, amount);
        }
        if (a->c.hp <= 0) {
            self->damage_dealt += a->c.hp;
            a->c.hp = 0;
            a->obj->motion = FX_HIT_DOWN_MOTION;
            BtlSePlay(voice, 1);
            if (g_btl_hit_slot >= BTL_PARTY) {
                BtlRollDefeatDrop(self, a);
            }
        } else {
            a->obj->motion = FX_HIT_HURT_MOTION;
            a->obj->children = o->children;
            a->obj->attr |= FX_HIT_STRUCK;
            BtlSePlay(voice, 0);
        }
        }
        break;
    }

    a->obj->phase = 0;
    o->timer = 0;
    o->phase++;
    if (a->obj->motion != FX_HIT_HURT_MOTION) {
        return;
    }
    switch (g_btl_fx_move) {
    case FX_HIT_SLEEP:
        if ((rand() & FX_HIT_ROLL) < 100) {
            a->c.status = STATUS_SLEEP;
            a->c.ail_level = FX_HIT_LEVEL;
            a->ail_turns = FX_HIT_LEVEL;
            BtlObjSetScript(a->obj->mark,
                            *(const u_long **)(g_btl_actor_gfx
                                               + STATUS_SLEEP * 4
                                               + BTL_GFX_SCRIPTS));
            BtlObjSetScript(a->obj->mark->attached,
                            g_btl_ail_level_marks[(signed char)a->c.ail_level]);
            a->obj->mark->unkCE = STATUS_SLEEP + BTL_MARK_BIAS;
            a->obj->mark->motion = BTL_MARK_MOTION;
            a->obj->mark->timer = BTL_MARK_TIMER;
        }
        break;
    case FX_HIT_FREEZE:
        amount = 0;
        chance = BtlApplyAffinity(&amount, FX_HIT_ICE, a->c.unk5C) / 4;
        if (chance >= 0 && (rand() & FX_HIT_ROLL) < chance
            && (u_int)(a->c.status - STATUS_STONE) >= 2) {
            a->c.status = STATUS_FREEZE;
            a->c.ail_level = FX_HIT_LEVEL;
            a->ail_turns = FX_HIT_LEVEL;
            BtlObjSetScript(a->obj->mark,
                            *(const u_long **)(g_btl_actor_gfx
                                               + STATUS_FREEZE * 4
                                               + BTL_GFX_SCRIPTS));
            BtlObjSetScript(a->obj->mark->attached,
                            g_btl_ail_level_marks[(signed char)a->c.ail_level]);
            a->obj->mark->unkCE = STATUS_FREEZE + BTL_MARK_BIAS;
            a->obj->mark->motion = BTL_MARK_MOTION;
            a->obj->mark->timer = BTL_MARK_TIMER;
        }
        break;
    case FX_HIT_PALYZE:
        if ((rand() & FX_HIT_ROLL) < 0x4D) {
            a->c.status = STATUS_PALYZE;
            a->c.ail_level = FX_HIT_LEVEL;
            a->ail_turns = BTL_AIL_TURNS_LONG;
            BtlObjSetScript(a->obj->mark,
                            *(const u_long **)(g_btl_actor_gfx
                                               + STATUS_PALYZE * 4
                                               + BTL_GFX_SCRIPTS));
            BtlObjSetScript(a->obj->mark->attached,
                            g_btl_ail_level_marks[(signed char)a->c.ail_level]);
            a->obj->mark->unkCE = STATUS_PALYZE + BTL_MARK_BIAS;
            a->obj->mark->motion = BTL_MARK_MOTION;
            a->obj->mark->timer = BTL_MARK_TIMER;
        }
        break;
    case FX_HIT_ANY:
        if (rand() & 1) {
            BtlInflictStatus(a, rand() % 16 + 1);
        }
        break;
    default:
        switch (g_spell_data[g_btl_fx_move].element) {
        case FX_HIT_FIRE:
            if ((signed char)a->c.status == STATUS_FREEZE
                && --*(signed char *)&a->c.ail_level < 0) {
                a->c.status = 0;
                a->c.ail_level = 0;
                a->action = 0;
            }
            break;
        case FX_HIT_ICE:
            react /= 4;
            if (react >= 0 && (rand() & FX_HIT_ROLL) < react) {
                BtlInflictStatus(a, STATUS_FREEZE);
            }
            break;
        case FX_HIT_ELEC:
            react /= 4;
            if (react >= 0 && (rand() & FX_HIT_ROLL) < react) {
                BtlInflictStatus(a, STATUS_SHOCK);
            }
            break;
        case FX_HIT_NERVE:
            react /= 4;
            if (react >= 0 && (rand() & FX_HIT_ROLL) < react) {
                BtlInflictStatus(a, g_spell_data[g_btl_fx_move].ailment);
            }
            break;
        }
        break;
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/fxresolve", BtlFxResolveHit);
#endif

/* The effect slots a cast leaves the enemies' voices in, two apart. */
#define FX_VOICE_SLOT_FIRST 10
#define FX_VOICE_SLOT_LAST  15
#define FX_VOICE_SLOT_STEP  2

/* The slot a spell's own sound is kept in, and the encounter that keeps it. */
#define FX_VOICE_SPELL_SLOT 3
#define FX_VOICE_KEEP_SPELL 3

/* Where a member's voice goes. */
#define FX_VOICE_MEMBER_SLOT 7

void BtlFxReopenVoices(void)
{
    int slot;

    if (g_btl_actors[g_btl_actor_turn].move == FX_HIT_TAKE
        && g_btl_actors[g_btl_actor_turn].unkD8 < 2) {
        for (slot = FX_VOICE_SLOT_FIRST; slot < FX_VOICE_SLOT_LAST;
             slot += FX_VOICE_SLOT_STEP) {
            if (g_btl_slot_owner[slot] > 0) {
                BtlSoundOpen(g_btl_slot_banks, slot / 2 + 2, slot / 2 - 5);
            }
        }
        return;
    }
    if (g_btl_actor_turn < BTL_PARTY) {
        if (g_btl_encounter != FX_VOICE_KEEP_SPELL) {
            BtlSoundClose(FX_VOICE_SPELL_SLOT);
        }
        for (slot = FX_VOICE_SLOT_FIRST; slot < FX_VOICE_SLOT_LAST;
             slot += FX_VOICE_SLOT_STEP) {
            if (g_btl_slot_owner[slot] > 0) {
                BtlSoundOpen(g_btl_slot_banks, slot / 2 + 2, slot / 2 - 5);
            }
        }
        return;
    }
    BtlSoundClose(FX_VOICE_SPELL_SLOT);
    slot = 0;
    do {
        if (g_btl_actors[slot].c.key != 0
            && (signed char)g_btl_actors[slot].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[slot].flags & BTL_ACTOR_OUT) == 0) {
            BtlSoundOpen(g_btl_banks, slot + FX_VOICE_MEMBER_SLOT,
                         g_btl_actors[slot].c.key);
        }
        slot++;
    } while (slot < BTL_PARTY);
}
