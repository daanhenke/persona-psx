/* Persona 1 (JP) - the moves an enemy plays out through an effect.  BTLP only.
 *   0x800B4B7C BtlEnemyPlayMove  0x800B52A8 BtlEnemyPersonaMove
 *
 * Entries 0 and 3 of g_btl_enemy_attack, which BtlEnemyMotion06 hands an
 * enemy's turn to.
 *
 * BtlEnemyPlayMove is a move with an effect of its own. The enemy is put on its
 * strike script and everyone else on the field is dimmed; an enemy under CLOSE
 * loses the move half the time, and otherwise the move's line goes up and its
 * SP is spent. The move's artwork and voice bank are read in, its effect is
 * started and waited out, the enemy's own sounds are opened again, and what the
 * move did to the enemy itself is settled: an ailment it takes, or the hp it
 * pays, with the reel or the fall that goes with it.
 *
 * BtlEnemyPersonaMove is the same move made through a Persona, the way a
 * member makes one: the summon circle goes up under the enemy, the Persona its
 * key calls up comes out and plays the move, and all of the Persona's records
 * are taken back off.
 */
#include <decomp/types.h>
#include <rand.h>
#include <libcd.h>
#include <libsnd.h>
#include <persona/main/cd.h>
#include <persona/common/spell.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/cast.h>
#include <persona/btlp/fieldmarks.h>
#include <persona/btlp/gfx.h>
#include <persona/btlp/load.h>
#include <persona/btlp/model.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pack.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/spellfx.h>
#include <persona/btlp/status.h>
#include <persona/btlp/text.h>

/* The one species whose third piece is put on a script of its own as the move
   starts. */
#define ENEMY_SPECIES_EXTRA 0x13

/* How the move's line is put up, and how long it stays. */
#define ENEMY_MSG_FLAGS 5
#define ENEMY_MSG_STYLE 2
#define ENEMY_MSG_X     0x68
#define ENEMY_MSG_Y     0xC
#define ENEMY_MSG_SLOW  0x5A
#define ENEMY_MSG_FAST  0x1E

/* The rest of the field fades down at this step while an enemy's move plays. */
#define ENEMY_DIM_FADE   2
#define ENEMY_ARENA_FADE 2

/* The moves from here on are the enemies' own, and cost no SP. */
#define ENEMY_MOVE_SPELLS 0x75

/* An enemy under CLOSE loses the move half the time: the line stays up this
   long, and phase 6 brings the field back. */
#define ENEMY_SEALED_FRAMES 0x78
#define ENEMY_PHASE_SEALED  6

/* The effect record the move is played in is set going on motion 2. */
#define ENEMY_FX_MOTION 2

/* The bank slot an enemy's own sounds are opened on, picked by its tpage byte:
   halved, from its tenth page on. */
#define ENEMY_VOICE_SLOT  6
#define ENEMY_VOICE_FIRST 5

/* Set on the enemy's record while what its move did to it is still to be
   settled, and the ailment that is never inflicted from here. */
#define ENEMY_SELF_HIT    0x20000
#define ENEMY_SELF_NO_AIL BTL_STATUS_GUILT

/* How an enemy reels or falls from what its own move cost it. */
#define ENEMY_MOTION_DOWN 8
#define ENEMY_MOTION_REEL 0x10
#define ENEMY_SE_HIT      6
#define ENEMY_SE_DOWN     1
#define ENEMY_SE_REEL     0
#define ENEMY_SE_AIL_SLOT 2
#define ENEMY_SE_AIL      5
#define ENEMY_AIL_TURNS   2

/* The two keys that call up a Persona of their own, and the Persona each
   calls - anyone else calls the third. The second also draws its circle on
   the other side. */
#define ENEMY_KEY_B5      0xB5
#define ENEMY_KEY_C2      0xC2
#define ENEMY_SUMMON_B5   0x66
#define ENEMY_SUMMON_C2   0x4A
#define ENEMY_SUMMON_ELSE 0x65

/* The circle's sound, and how long it stands before the Persona comes out. */
#define ENEMY_CIRCLE_SE_SLOT 5
#define ENEMY_CIRCLE_FRAMES  0x3C

/* The line an enemy under CLOSE puts up in place of its move. */
extern const u_char g_btl_msg_enemy_sealed[];

void BtlEnemyPlayMove(BtlObj *o)
{
    CdlLOC            loc;
    BtlSoundBank      bank;
    BtlActor         *a;
    const BtlSpecies *sp;
    int               speed;
    int               i;

    a = o->actor;
    sp = &g_btl_species[o->kind];
    switch (o->phase) {
    case 0:
        BtlObjSetScript(o, (BtlSeqStep *)o->scripts[sp->strike[o->spell_slot]]);
        if (o->kind == ENEMY_SPECIES_EXTRA) {
            BtlObjSetScript(o->attached, (BtlSeqStep *)o->scripts[sp->extra]);
        }
        for (i = BTL_PARTY; i < BTL_ACTORS; i++) {
            if (g_btl_actors[i].c.key != 0
                && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
                && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0
                && i != o->mark_num) {
                g_btl_actors[i].obj->rgb_to[0] = CAST_DIM;
                g_btl_actors[i].obj->rgb_to[1] = CAST_DIM;
                g_btl_actors[i].obj->rgb_to[2] = CAST_DIM;
                BtlObjSetFade(g_btl_actors[i].obj, ENEMY_DIM_FADE);
            }
        }
        g_btl_scene_rgb[0] = SUMMON_SCENE_DIM;
        g_btl_scene_rgb[1] = SUMMON_SCENE_DIM;
        g_btl_scene_rgb[2] = SUMMON_SCENE_DIM;
        g_btl_arena_fade = ENEMY_ARENA_FADE;
        if ((signed char)a->c.status == BTL_STATUS_CLOSE && (rand() & 1) != 0) {
            BtlOpenMessage(ENEMY_MSG_FLAGS, ENEMY_MSG_STYLE, g_btl_msg_enemy_sealed,
                           ENEMY_MSG_X, ENEMY_MSG_Y);
            o->timer = ENEMY_SEALED_FRAMES;
            o->phase = ENEMY_PHASE_SEALED;
            break;
        }
        if (g_btl_effect_obj == NULL && g_btl_msg_speed != 2) {
            BtlOpenMessage(ENEMY_MSG_FLAGS, ENEMY_MSG_STYLE, g_btl_move_lines[a->move],
                           ENEMY_MSG_X, ENEMY_MSG_Y);
            if (g_btl_msg_speed == 0) {
                speed = ENEMY_MSG_SLOW;
            } else {
                speed = ENEMY_MSG_FAST;
            }
            g_btl_msg_timer = speed;
        }
        if (a->move < ENEMY_MOVE_SPELLS) {
            a->c.sp -= g_spell_data[a->move].cost;
        }
        CdIntToPos(g_btl_persona_sectors[BTL_MOVE_FILE_FIRST + o->actor->move]
                       + g_btl_move_gfx_base, &loc);
        CdReadFileToAddrAsync((CdlFILE *)&loc,
                              g_btl_persona_sectors[BTL_MOVE_FILE_FIRST + o->actor->move + 1]
                                  - g_btl_persona_sectors[BTL_MOVE_FILE_FIRST
                                                          + o->actor->move],
                              BTL_LOAD_STAGE);
        o->phase++;
        break;

    case 1:
        if (g_cd_busy != -1) {
            break;
        }
        CdIntToPos(g_btl_pack_sectors[o->actor->move] + g_btl_voice_base, &loc);
        CdReadFileToAddrAsync((CdlFILE *)&loc,
                              g_btl_pack_sectors[o->actor->move + 1]
                                  - g_btl_pack_sectors[o->actor->move],
                              (u_long *)BTL_VOICE_BUFFER);
        o->phase++;
        break;

    case 2:
        if (g_cd_busy != -1) {
            break;
        }
        bank.nsep = BTL_VOICE_SEPS;
        bank.vb = g_btl_voice_vb;
        bank.vh = g_btl_voice_vh;
        bank.seq = g_btl_voice_seq;
        BtlSoundOpen(&bank, BTL_BGM_SLOT, 0);
        o->phase++;
        break;

    case 3:
        if ((o->attr & BTL_OBJ_BUSY_MASK) == BTL_OBJ_BUSY) {
            break;
        }
        g_btl_half_rate = 1;
        o->child = BtlStartMoveFx(o->actor->move);
        o->child->actor = o->actor;
        o->child->motion = ENEMY_FX_MOTION;
        o->phase++;
        break;

    case 4:
        if (o->child->motion != 0) {
            break;
        }
        g_btl_half_rate = 0;
        BtlObjFree(o->child);
        BtlSoundClose(BTL_BGM_SLOT);
        BtlSoundOpen(g_btl_slot_banks, ENEMY_VOICE_SLOT, (o->tpage >> 1) - ENEMY_VOICE_FIRST);
        o->phase++;
        break;

    case 5:
        if (SsVabTransCompleted(0) == 0) {
            break;
        }
        if ((o->attr & ENEMY_SELF_HIT) == 0) {
            o->phase++;
            break;
        }
        if (a->unkCC != 0) {
            if (a->unkCC != ENEMY_SELF_NO_AIL && BtlInflictStatus(a, a->unkCC) != 0) {
                if ((signed char)a->c.status == BTL_STATUS_DOWN) {
                    a->resume_motion = o->motion;
                    a->resume_phase = o->phase + 1;
                    a->hit_amount = 0;
                    a->c.hp = 0;
                    a->obj->motion = ENEMY_MOTION_DOWN;
                    a->obj->phase = 0;
                    BtlSePlay(ENEMY_SE_HIT, ENEMY_SE_DOWN);
                } else {
                    a->resume_motion = o->motion;
                    a->resume_phase = o->phase + 1;
                    a->ail_turns = ENEMY_AIL_TURNS;
                    a->hit_amount = 0;
                    o->motion = ENEMY_MOTION_REEL;
                    BtlSePlay(ENEMY_SE_AIL_SLOT, ENEMY_SE_AIL);
                }
            } else {
                o->attr &= ~ENEMY_SELF_HIT;
                o->phase++;
                break;
            }
        } else {
            a->c.hp -= a->hit_amount;
            if (a->c.hp <= 0) {
                a->resume_motion = o->motion;
                a->resume_phase = o->phase + 1;
                a->c.hp = 0;
                o->motion = ENEMY_MOTION_DOWN;
                BtlSePlay(ENEMY_SE_HIT, ENEMY_SE_DOWN);
            } else {
                a->resume_motion = o->motion;
                a->resume_phase = o->phase + 1;
                o->motion = ENEMY_MOTION_REEL;
                BtlSePlay(ENEMY_SE_HIT, ENEMY_SE_REEL);
            }
        }
        o->phase = 0;
        break;

    case ENEMY_PHASE_SEALED:
        if (o->timer != 0) {
            break;
        }
        BtlReadyNextTurn();
        for (i = BTL_PARTY; i < BTL_ACTORS; i++) {
            if (g_btl_actors[i].c.key != 0
                && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
                && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
                g_btl_actors[i].obj->rgb_to[0] = SUMMON_SCENE_LIT;
                g_btl_actors[i].obj->rgb_to[1] = SUMMON_SCENE_LIT;
                g_btl_actors[i].obj->rgb_to[2] = SUMMON_SCENE_LIT;
            }
        }
        g_btl_scene_rgb[0] = SUMMON_SCENE_LIT;
        g_btl_scene_rgb[1] = SUMMON_SCENE_LIT;
        g_btl_scene_rgb[2] = SUMMON_SCENE_LIT;
        o->phase++;
        break;

    case 7:
        if (o->timer != 0) {
            break;
        }
        BtlObjSetScript(o, (BtlSeqStep *)o->scripts[sp->after[o->actor->turn_slot]]);
        o->motion = 0;
        o->phase = 0;
        break;
    }
}

void BtlEnemyPersonaMove(BtlObj *o)
{
    CdlLOC        loc;
    BtlSoundBank  bank;
    BtlActor     *a;
    BtlObj       *p;
    int           speed;
    int           i;

    a = o->actor;
    switch (o->phase) {
    case 0:
        if (g_btl_msg_speed != 2) {
            BtlOpenMessage(ENEMY_MSG_FLAGS, ENEMY_MSG_STYLE, g_btl_move_lines[a->move],
                           ENEMY_MSG_X, ENEMY_MSG_Y);
            if (g_btl_msg_speed == 0) {
                speed = ENEMY_MSG_SLOW;
            } else {
                speed = ENEMY_MSG_FAST;
            }
            g_btl_msg_timer = speed;
        }
        a->c.sp -= g_spell_data[a->move].cost;
        if (g_btl_place_party == 0) {
            BtlAimMove(a);
        }
        BtlObjSetScript(o, (BtlSeqStep *)o->scripts[g_btl_species_now->strike[o->spell_slot]]);
        switch (a->c.key) {
        case ENEMY_KEY_B5:
            a->summon = ENEMY_SUMMON_B5;
            break;
        case ENEMY_KEY_C2:
            a->summon = ENEMY_SUMMON_C2;
            break;
        default:
            a->summon = ENEMY_SUMMON_ELSE;
            break;
        }
        for (i = BTL_PARTY; i < BTL_ACTORS; i++) {
            if (g_btl_actors[i].c.key != 0
                && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
                && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0
                && i != o->mark_num) {
                g_btl_actors[i].obj->rgb_to[0] = CAST_DIM;
                g_btl_actors[i].obj->rgb_to[1] = CAST_DIM;
                g_btl_actors[i].obj->rgb_to[2] = CAST_DIM;
                BtlObjSetFade(g_btl_actors[i].obj, ENEMY_DIM_FADE);
            }
        }
        CdIntToPos(g_btl_persona_sectors[CAST_FILE] + g_btl_move_gfx_base, &loc);
        CdReadFileToAddrAsync((CdlFILE *)&loc,
                              g_btl_persona_sectors[CAST_FILE + 1]
                                  - g_btl_persona_sectors[CAST_FILE],
                              BTL_LOAD_STAGE);
        o->phase++;
        break;

    case 1:
        if (g_cd_busy != -1) {
            return;
        }
        BtlUploadTim((u_long *)g_load_stage, CAST_TIM_PAGE, CAST_TIM_SLOT, 1, 0, 1);
        CdIntToPos(g_btl_persona_sectors[o->actor->summon] + g_btl_persona_gfx_base, &loc);
        CdReadFileToAddrAsync((CdlFILE *)&loc,
                              g_btl_persona_sectors[o->actor->summon + 1]
                                  - g_btl_persona_sectors[o->actor->summon],
                              BTL_LOAD_STAGE);
        o->phase++;
        break;

    case 2:
        if ((o->attr & BTL_OBJ_BUSY_MASK) == BTL_OBJ_BUSY) {
            return;
        }
        o->rgb_to[0] = 0xFF;
        o->rgb_to[1] = 0xFF;
        o->rgb_to[2] = 0xFF;
        g_btl_scene_rgb[0] = SUMMON_SCENE_DIM;
        g_btl_scene_rgb[1] = SUMMON_SCENE_DIM;
        g_btl_scene_rgb[2] = SUMMON_SCENE_DIM;
        o->fade = CAST_ARENA_FADE;
        g_btl_arena_fade = CAST_ARENA_FADE;
        g_btl_hud_obj = BtlSpawnCastCircle(a->c.key != ENEMY_KEY_C2, o->col2, o->row);
        BtlSePlay(ENEMY_CIRCLE_SE_SLOT, 0);
        o->timer = ENEMY_CIRCLE_FRAMES;
        o->phase++;
        break;

    case 3:
        if (o->timer != 0) {
            return;
        }
        if (g_cd_busy != -1) {
            return;
        }
        BtlCloseMessage(0);
        g_btl_persona_ready = 0;
        g_btl_persona_obj = BtlSpawnPersona(a->summon, o->col2, o->row, 0);
        g_btl_persona_obj->actor = a;
        BtlObjSetAttr(g_btl_persona_obj, CAST_PERSONA_ATTR);
        BtlObjSetScale(g_btl_persona_obj, CAST_SCALE_XY, CAST_SCALE_XY, CAST_SCALE_Z);
        BtlObjSetMotion(g_btl_persona_obj, 3);
        g_btl_half_rate = 1;
        o->phase++;
        break;

    case 4:
        CdIntToPos(g_btl_persona_sectors[BTL_MOVE_FILE_FIRST + o->actor->move]
                       + g_btl_move_gfx_base, &loc);
        CdReadFileToAddrAsync((CdlFILE *)&loc,
                              g_btl_persona_sectors[BTL_MOVE_FILE_FIRST + o->actor->move + 1]
                                  - g_btl_persona_sectors[BTL_MOVE_FILE_FIRST
                                                          + o->actor->move],
                              BTL_LOAD_STAGE);
        o->phase++;
        break;

    case 5:
        if (g_cd_busy != -1) {
            return;
        }
        CdIntToPos(g_btl_pack_sectors[o->actor->move] + g_btl_voice_base, &loc);
        CdReadFileToAddrAsync((CdlFILE *)&loc,
                              g_btl_pack_sectors[o->actor->move + 1]
                                  - g_btl_pack_sectors[o->actor->move],
                              (u_long *)BTL_VOICE_BUFFER);
        o->phase++;
        break;

    case 6:
        if (g_cd_busy != -1) {
            return;
        }
        bank.nsep = BTL_VOICE_SEPS;
        bank.vb = g_btl_voice_vb;
        bank.vh = g_btl_voice_vh;
        bank.seq = g_btl_voice_seq;
        BtlSoundOpen(&bank, BTL_BGM_SLOT, 0);
        o->phase++;
        g_btl_persona_ready = 1;
        break;

    case 7:
        if (g_btl_persona_obj->motion != 0) {
            return;
        }
        i = 0;
        p = g_btl_persona_obj;
        g_btl_half_rate = 0;
        g_btl_scene_rgb[0] = SUMMON_SCENE_LIT;
        g_btl_scene_rgb[1] = SUMMON_SCENE_LIT;
        g_btl_scene_rgb[2] = SUMMON_SCENE_LIT;
        do {
            BtlObjFree(p);
            p = p->attached;
            i++;
        } while (i < CAST_PERSONA_PIECES);
        BtlSoundClose(BTL_BGM_SLOT);
        BtlReadyNextTurn();
        o->phase++;
        break;

    case 8:
        BtlObjSetScript(o, (BtlSeqStep *)o->scripts[g_btl_species_now->after[o->actor->turn_slot]]);
        BtlObjFree(g_btl_hud_obj);
        BtlObjFree(g_btl_hud_obj->attached);
        g_btl_hud_obj = NULL;
        o->motion = 0;
        o->phase = 0;
        break;

    case 10:
        if (g_cd_busy != -1) {
            return;
        }
        BtlReadyNextTurn();
        BtlSoundClose(ENEMY_VOICE_SLOT);
        o->motion = 0;
        o->phase = 0;
        break;
    }
}
