/* Persona 1 (JP) - a fighter going down.  BTLP only.
 *   0x8008BB70 BtlActorMotion08
 *
 * Motion 8 of both fighter tables, run a phase at a time. Nothing happens until
 * the record is free to move and the sound bank has finished loading.
 *
 * Phase nought lands the blow that finished the fighter. A few enemies turn
 * aside first: the two that change shape go to phase 2 instead, two go to
 * phases 4 and 6 to be held a while, two stop the music, and the bosses stop
 * the round for their scene unless it has already been held. Then the fall
 * sounds, and
 *
 *  - an enemy wakes the encounter's partner where it has one, adds its hp,
 *    experience and money to what the fight has won, marks its Persona as won,
 *    goes on its death script and is taken off the grid and out of the fight;
 *  - a member clears its story flag in the fights that keep one, goes on its
 *    death script, leaves the formation and is put down - and when it was not
 *    being carried, a Persona of the right kind may be given the turn to act
 *    for it.
 *
 * Either way the amount goes up as a still number, the shadow and marker are
 * hidden, the palettes are put back, and the fighter fades out.
 *
 * Phase 1 waits for the fade and lets the fighter go - back to what carried
 * it, or hidden for good - except for the one enemy that rises again. The
 * others: 2 turns a shape-changer into its second form, 3 hands a carried
 * fighter back, 4 and 6 hold a fall for a second with its number up, 5 and 7
 * finish those two, 8 hands the round to the held scene, and 9 waits for a
 * script to run out.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <libsnd.h>
#include <rand.h>
#include <persona/common/persona.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/clut.h>
#include <persona/btlp/fieldmarks.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/model.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/stats.h>

/* The two attribute bits that mean the record is not free to move yet. */
#define DEATH_BUSY (BTL_OBJ_HELD | BTL_OBJ_TRACKING)

/* The enemies that do something of their own as they fall. */
#define KIND_SHAPE_A     0x9A    /* change shape: 0x9A into 0xBB, 0x9B into */
#define KIND_SHAPE_B     0x9B    /* 0xBC                                    */
#define KIND_SHAPE_A_TO  0xBB
#define KIND_SHAPE_B_TO  0xBC
#define KIND_HELD_A      0xC1    /* held, then put on its eleventh script   */
#define KIND_HELD_B      0xA9    /* held, then put on its first             */
#define KIND_MUSIC_A     0xAF
#define KIND_MUSIC_B     0xB3
#define KIND_BOSS_A      0xA3
#define KIND_BOSS_B      0xA8
#define KIND_BOSS_C      0xB1
#define KIND_BOSS_D      0xB6
#define KIND_BOSS_E      0xB7
#define KIND_RISES       0xB2
#define KIND_NO_PARTNER  0x7E

/* The phases the enemies above are sent to. */
#define PHASE_SHAPE   2
#define PHASE_HELD_A  4
#define PHASE_HELD_B  6
#define PHASE_SCENE   8

/* The encounter with a partner to wake, and which of its fighters that is. */
#define PARTNER_ENCOUNTER 3
#define PARTNER_SLOT      3

/* Species below this are Personas that can be won. */
#define KIND_PERSONAS 0x99

/* The fall's sound, and the grid and formation widths. */
#define FALL_BANK   2
#define FALL_SOUND  6
#define GRID_W      9
#define FORMATION_W 5

/* The member's row of scripts it dies in, and the scripts that finish a held
   fall and raise the enemy that rises. */
#define SCRIPT_DEATH      5
#define SCRIPT_HELD_A     11
#define SCRIPT_HELD_B     0
#define SCRIPT_RISEN      17

/* The music the story flags are cleared under. */
#define BGM_FALL_A 0x1D
#define BGM_FALL_B 0x1E

/* A Persona acting for its fallen member: the affinity it needs, the odds,
   and the Persona's own conditions. */
#define ACT_FOR_KIND     3
#define ACT_FOR_AFFINITY 3
#define ACT_FOR_ODDS     4
#define ACT_FOR_SLOTS    6
#define ACT_FOR_MASK     0xF0
#define ACT_FOR_TYPE     0x40

#define DEATH_NUMBER_Z     (-0x300000)
#define DEATH_NUMBER_ALONE 0xFF
#define DEATH_CELL_EMPTY   0xFF
#define DEATH_HOLD         60
#define DEATH_TPAGE_ABR    0x60
#define DEATH_TPAGE_SUB    0x20
#define DEATH_FADE         4
#define DEATH_FADE_FAST    8
#define RISEN_GREY         0x80
#define RISEN_FADE         0xFF
#define RISEN_MOTION       8
#define RISEN_PHASE        9

/* The story flag each member clears as it falls, by key. */
extern u_char g_btl_member_fall_flags[];
/* The script the partner is woken with. */
extern u_char g_btl_partner_script;

void BtlActorMotion08(BtlObj *o)
{
    BtlActor *a;
    BtlObj   *num;
    BtlObj   *p;
    BtlStats *persona;
    const u_char *row;
    u_int     kind;
    u_long    affinity;
    int       odds;

    a = o->actor;
    if ((o->attr & DEATH_BUSY) != 0) {
        return;
    }
    if ((short)SsVabTransCompleted(0) == 0) {
        return;
    }
    switch (o->phase) {
    case 0:
        if ((o->attr & BTL_OBJ_OTHER_SIDE) != 0) {
            switch (o->kind) {
            case KIND_SHAPE_A:
            case KIND_SHAPE_B:
                o->phase = PHASE_SHAPE;
                return;
            case KIND_HELD_A:
                o->phase = PHASE_HELD_A;
                return;
            case KIND_HELD_B:
                o->phase = PHASE_HELD_B;
                return;
            case KIND_MUSIC_A:
            case KIND_MUSIC_B:
                SsSepStop(g_btl_seq[0], 0);
                BtlSePlay(0, 1);
                break;
            case KIND_BOSS_A:
            case KIND_BOSS_B:
            case KIND_BOSS_C:
            case KIND_BOSS_D:
            case KIND_BOSS_E:
                if (g_btl_hold_done == 0) {
                    o->phase = PHASE_SCENE;
                    return;
                }
                break;
            }
        }
        if (g_btl_vab[FALL_BANK] >= 0 && g_btl_seq[FALL_BANK] >= 0) {
            BtlSePlay(FALL_BANK, FALL_SOUND);
        }
        if ((o->attr & BTL_OBJ_OTHER_SIDE) != 0) {
            if (g_btl_encounter == PARTNER_ENCOUNTER
                && o->kind != KIND_NO_PARTNER) {
                BtlObjSetScript(g_btl_enemies[PARTNER_SLOT].obj,
                    (BtlSeqStep *)g_btl_enemies[PARTNER_SLOT].obj->scripts[
                        g_btl_partner_script]);
            }
            g_btl_won_hp += o->actor->c.hp_max;
            g_btl_won_exp += g_persona_data[o->kind].exp;
            g_btl_won_unk10 += g_persona_data[o->kind].unk10;
            g_btl_won_money += g_persona_data[o->kind].price;
            kind = o->kind;
            if (kind < KIND_PERSONAS) {
                g_btl_persona_won[kind >> 5] |= 1 << (kind & 0x1F);
            }
            BtlObjSetScript(o,
                (BtlSeqStep *)o->scripts[g_btl_models[o->kind].death]);
            g_btl_grid[o->row * GRID_W + o->col2] = DEATH_CELL_EMPTY;
            g_btl_actors[o->mark_num].c.key = 0;
            g_btl_actors[o->mark_num].c.status = BTL_STATUS_DOWN;
            g_btl_actors[o->mark_num].flags = 0;
            if (o->actor->hit_amount != 0) {
                num = BtlSpawnHitNumber(g_btl_actors[o->mark_num].hit_amount,
                                        &o->x, HIT_NUMBER_STILL);
                num->z = (g_btl_models[o->kind].number_z << 16) + o->z;
            }
        } else {
            if (g_btl_bgm_index == BGM_FALL_A || g_btl_bgm_index == BGM_FALL_B) {
                BtlEventFlagClear(g_btl_member_fall_flags[a->c.key]);
                a->c.blocked = 1;
                BtlApplyPersona(a);
            }
            row = &g_btl_member_scripts[SCRIPT_DEATH
                                        + o->kind * MEMBER_SCRIPT_MODEL];
            BtlObjSetScript(o, (BtlSeqStep *)o->scripts[
                row[o->actor->script_pick * MEMBER_SCRIPT_PICK]]);
            g_btl_formation[o->row * FORMATION_W + o->col2 / 2] =
                DEATH_CELL_EMPTY;
            g_btl_actors[o->mark_num].c.status = BTL_STATUS_DOWN;
            g_btl_actors[o->mark_num].flags = 0;
            if (o->actor->hit_amount != 0) {
                num = BtlSpawnHitNumber(g_btl_actors[o->mark_num].hit_amount,
                                        &o->x, HIT_NUMBER_STILL);
                num->z = DEATH_NUMBER_Z;
            }
            BtlBuildMarkers();
            if ((o->attr & BTL_OBJ_CARRIED) == 0) {
                odds = ACT_FOR_ODDS;
                persona = &g_btl_personas[BtlActorPersona(o->mark_num)];
                affinity = (g_persona_defs[persona->key].unk1C
                            >> ((g_btl_actors[o->mark_num].c.key - 1) * 2)) & 3;
                if (g_btl_debug_act_for != 0) {
                    odds = 1;
                }
                if (g_btl_act_kind == 0 && affinity == ACT_FOR_AFFINITY
                    && persona->slots >= ACT_FOR_SLOTS
                    && (persona->unk41 & ACT_FOR_MASK) == ACT_FOR_TYPE
                    && (rand() & (odds - 1)) == 0
                    && o->actor->c.blocked == 0) {
                    g_btl_act_kind = ACT_FOR_KIND;
                    g_btl_act_actor = o->mark_num;
                }
            }
        }
        if (o->actor->hit_amount != 0) {
            num->mark_num = DEATH_NUMBER_ALONE;
            for (p = g_btl_obj_pool; p != NULL; p = p->next) {
                if (p->kind == MARK_KIND_STILL && p->mark_num == o->mark_num) {
                    BtlObjFree(p);
                    break;
                }
            }
            num->mark_num = o->mark_num;
        }
        o->shadow->attr |= BTL_OBJ_HIDDEN;
        o->mark->attr |= BTL_OBJ_HIDDEN;
        o->mark->attached->attr |= BTL_OBJ_HIDDEN;
        memcpy((u_char *)g_btl_actor_clut_to + o->mark_num * BTL_CLUT_BYTES,
               (u_char *)g_btl_actor_clut_base + o->mark_num * BTL_CLUT_BYTES,
               BTL_CLUT_BYTES);
        memcpy((u_char *)g_btl_actor_clut + o->mark_num * BTL_CLUT_BYTES,
               (u_char *)g_btl_actor_clut_base + o->mark_num * BTL_CLUT_BYTES,
               BTL_CLUT_BYTES);
        g_btl_tpage[o->unkCD] &= ~DEATH_TPAGE_ABR;
        g_btl_tpage[o->unkCD] |= DEATH_TPAGE_SUB;
        o->rgb_to[0] = 0;
        o->rgb_to[1] = 0;
        o->rgb_to[2] = 0;
        o->attr |= BTL_OBJ_NO_SHADOW;
        o->fade = g_btl_half_rate ? DEATH_FADE_FAST : DEATH_FADE;
        o->phase++;
        break;

    case 1:
        if ((o->rgb[0] | o->rgb[1] | o->rgb[2]) != 0) {
            return;
        }
        if ((o->attr & BTL_OBJ_CARRIED) != 0) {
            o->motion = o->actor->resume_motion;
            o->phase = o->actor->resume_phase;
            o->attr &= ~BTL_OBJ_CARRIED;
        } else {
            o->motion = 0;
            o->attr |= BTL_OBJ_HIDDEN;
            o->mark->attr |= BTL_OBJ_HIDDEN;
        }
        if ((o->attr & BTL_OBJ_OTHER_SIDE) != 0 && o->kind == KIND_RISES) {
            o->attr &= ~(BTL_OBJ_HIDDEN | BTL_OBJ_NO_SHADOW);
            o->rgb_to[0] = RISEN_GREY;
            o->rgb_to[1] = RISEN_GREY;
            o->rgb_to[2] = RISEN_GREY;
            o->fade = RISEN_FADE;
            BtlObjSetScript(o, (BtlSeqStep *)o->scripts[SCRIPT_RISEN]);
            o->motion = RISEN_MOTION;
            o->phase = RISEN_PHASE;
        }
        g_btl_se_off = 0;
        break;

    case 2:
        BtlObjSetScript(o, (BtlSeqStep *)o->scripts[g_btl_models[o->kind].death]);
        o->kind = (o->kind == KIND_SHAPE_A) ? KIND_SHAPE_A_TO : KIND_SHAPE_B_TO;
        o->actor->c.key = o->kind;
        g_btl_grid[o->row * GRID_W + o->col2] = o->kind;
        BtlLoadEnemyStats(o->mark_num - BTL_PARTY, o->kind);
        if (o->actor->hit_amount != 0) {
            num = BtlSpawnHitNumber(g_btl_actors[o->mark_num].hit_amount,
                                    &o->x, HIT_NUMBER_STILL);
            num->z = (g_btl_models[o->kind].number_z << 16) + o->z;
            num->mark_num = DEATH_NUMBER_ALONE;
            for (p = g_btl_obj_pool; p != NULL; p = p->next) {
                if (p->kind == MARK_KIND_STILL && p->mark_num == o->mark_num) {
                    BtlObjFree(p);
                    break;
                }
            }
            num->mark_num = o->mark_num;
        }
        memcpy((u_char *)g_btl_actor_clut_to + o->mark_num * BTL_CLUT_BYTES,
               (u_char *)g_btl_actor_clut_base + o->mark_num * BTL_CLUT_BYTES,
               BTL_CLUT_BYTES);
        memcpy((u_char *)g_btl_actor_clut + o->mark_num * BTL_CLUT_BYTES,
               (u_char *)g_btl_actor_clut_base + o->mark_num * BTL_CLUT_BYTES,
               BTL_CLUT_BYTES);
        o->phase++;
        break;

    case 3:
        if ((o->attr & BTL_OBJ_BUSY_MASK) == BTL_OBJ_BUSY) {
            return;
        }
        if ((o->attr & BTL_OBJ_CARRIED) != 0) {
            o->motion = o->actor->resume_motion;
            o->phase = o->actor->resume_phase;
            o->attr &= ~BTL_OBJ_CARRIED;
        } else {
            o->motion = 0;
        }
        break;

    case 4:
        BtlObjSetScript(o, (BtlSeqStep *)o->scripts[g_btl_models[o->kind].death]);
        o->timer = DEATH_HOLD;
        if (o->actor->hit_amount != 0) {
            num = BtlSpawnHitNumber(g_btl_actors[o->mark_num].hit_amount,
                                    &o->x, HIT_NUMBER_STILL);
            num->z = (g_btl_models[o->kind].number_z << 16) + o->z;
            num->mark_num = DEATH_NUMBER_ALONE;
            for (p = g_btl_obj_pool; p != NULL; p = p->next) {
                if (p->kind == MARK_KIND_STILL && p->mark_num == o->mark_num) {
                    BtlObjFree(p);
                    break;
                }
            }
            num->mark_num = o->mark_num;
        }
        o->phase++;
        break;

    case 5:
        if (o->timer != 0) {
            return;
        }
        BtlObjSetScript(o, (BtlSeqStep *)o->scripts[SCRIPT_HELD_A]);
        o->actor->c.status = BTL_STATUS_DOWN;
        o->motion = 0;
        o->phase = 0;
        break;

    case 6:
        BtlObjSetScript(o, (BtlSeqStep *)o->scripts[g_btl_models[o->kind].death]);
        o->timer = DEATH_HOLD;
        if (o->actor->hit_amount != 0) {
            num = BtlSpawnHitNumber(g_btl_actors[o->mark_num].hit_amount,
                                    &o->x, HIT_NUMBER_STILL);
            num->z = (g_btl_models[o->kind].number_z << 16) + o->z;
            num->mark_num = DEATH_NUMBER_ALONE;
            for (p = g_btl_obj_pool; p != NULL; p = p->next) {
                if (p->kind == MARK_KIND_STILL && p->mark_num == o->mark_num) {
                    BtlObjFree(p);
                    break;
                }
            }
            num->mark_num = o->mark_num;
        }
        o->phase++;
        break;

    case 7:
        if (o->timer != 0) {
            return;
        }
        BtlObjSetScript(o, (BtlSeqStep *)o->scripts[SCRIPT_HELD_B]);
        o->actor->c.status = BTL_STATUS_DOWN;
        o->motion = 0;
        o->phase = 0;
        break;

    case 8:
        g_btl_hold_markers = 1;
        g_btl_hold_obj = o;
        break;

    case 9:
        if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
            return;
        }
        o->motion = 0;
        o->phase = 0;
        break;
    }
}
