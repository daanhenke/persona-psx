/* Persona 1 (JP) - a member using an item.  BTLP only.
 *   0x800B0984 BtlMemberMotion03
 *
 * Entry 3 of g_btl_member_motion, named for the motion like the rest of the
 * table. The item's own effect is a move id, so the whole of the turn past the
 * first phase is the one the cast runs: the move's artwork and the voice bank
 * are read in, the effect is started, and what it cost the member is settled
 * once the effect is done with.
 *
 * Nought aims the move and takes the item off the list; a turn the aim fizzles
 * on, and one whose item is not there to be taken, both jump straight to the
 * last phase. One and two read the voice bank and hand it to the SPU - one
 * falls into two rather than waiting a frame, so a bank that is already
 * resident opens on the same frame it was asked for. Three starts the effect
 * and four waits it out; a three that finds the drive still busy falls into
 * four the same way. Five settles the cost and six puts the field back.
 *
 * The cost is settled the way BtlPersonaPlayMove settles a Persona's: the move
 * that spends the caster outright, then the ailment the effect left, and
 * failing both the hp it took. The three ends differ in which record they
 * leave the motion on - the fall after the ailment is put on the member's own
 * object, the other two on this one - which is the image's own inconsistency,
 * not a distinction: for a member's item turn the two records are the same.
 */
#include <decomp/types.h>
#include <libcd.h>
#include <libsnd.h>
#include <persona/main/cd.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/load.h>
#include <persona/btlp/model.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pack.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/spellfx.h>
#include <persona/btlp/status.h>
#include <persona/btlp/strike.h>
#include <persona/common/item.h>

/* Which of the model's ten scripts the arm goes up in - the same one the cast
   calls SCRIPT_SUMMON - and the phase a turn that cannot be made jumps to. */
#define SCRIPT_SUMMON   3
#define ITEM_PHASE_END  6

/* What BtlAimMemberMove leaves on the object when the move cannot be made. */
#define AIM_FIZZLED 10

/* The move an item spends the user outright with: the member falls where it
   stands and the turn is over. */
#define ITEM_MOVE_SELF 0x6D

/* The motion the move's effect is set going on. */
#define ITEM_FX_MOTION 2

/* How long the ailment an item leaves is meant to last. */
#define ITEM_AIL_TURNS 2

/* What the arena is put back to as the turn closes. */
#define ITEM_SCENE_LIT 0x80

void BtlMemberMotion03(BtlObj *o)
{
    CdlLOC        loc;
    BtlSoundBank  bank;
    BtlActor     *a;
    const u_char *scripts;

    a = o->actor;
    switch (o->phase) {
    case 0:
        g_btl_hit_slot = a->order;
        BtlAimMemberMove(a, a->move);
        if (o->phase == AIM_FIZZLED || BtlTakeItem(a->ail_line) == 0) {
            o->phase = ITEM_PHASE_END;
            break;
        }
        scripts = &g_btl_member_scripts[SCRIPT_SUMMON
                                        + o->kind * MEMBER_SCRIPT_MODEL];
        BtlObjSetScript(o, (BtlSeqStep *)o->scripts[
            scripts[o->actor->script_pick * MEMBER_SCRIPT_PICK]]);
        CdIntToPos(g_btl_persona_sectors[BTL_MOVE_FILE_FIRST + o->actor->move]
                       + g_btl_move_gfx_base,
                   &loc);
        CdReadFileToAddrAsync(
            (CdlFILE *)&loc,
            g_btl_persona_sectors[BTL_MOVE_FILE_FIRST + o->actor->move + 1]
                - g_btl_persona_sectors[BTL_MOVE_FILE_FIRST + o->actor->move],
            BTL_LOAD_STAGE);
        o->phase++;
        break;

    case 1:
        if (g_cd_busy == -1) {
            CdIntToPos(g_btl_pack_sectors[o->actor->move] + g_btl_voice_base,
                       &loc);
            CdReadFileToAddrAsync((CdlFILE *)&loc,
                                  g_btl_pack_sectors[o->actor->move + 1]
                                      - g_btl_pack_sectors[o->actor->move],
                                  (u_long *)BTL_VOICE_BUFFER);
            o->phase++;
        }
        /* and straight on into the open, rather than a frame later */
    case 2:
        if (g_cd_busy != -1) {
            break;
        }
        bank.nsep = BTL_VOICE_SEPS;
        bank.vb   = g_btl_voice_vb;
        bank.vh   = g_btl_voice_vh;
        bank.seq  = g_btl_voice_seq;
        BtlSoundOpen(&bank, BTL_BGM_SLOT, 0);
        o->phase++;
        break;

    case 3:
        if (g_cd_busy == -1) {
            g_btl_half_rate = 1;
            o->child = BtlStartMoveFx(o->actor->move);
            o->child->actor = o->actor;
            o->child->motion = ITEM_FX_MOTION;
            o->phase++;
            break;
        }
        /* and straight on into the wait */
    case 4:
        if (o->child->motion != 0) {
            break;
        }
        g_btl_half_rate = 0;
        BtlObjFree(o->child);
        BtlSoundOpen(g_btl_banks, STRIKE_VOICE, a->c.key);
        o->phase++;
        break;

    case 5:
        if (SsVabTransCompleted(SS_IMMEDIATE) == 0) {
            break;
        }
        if (a->move == ITEM_MOVE_SELF) {
            o->attr |= BTL_OBJ_CARRIED;
            a->resume_motion = o->motion;
            a->resume_phase  = o->phase + 1;
            a->hit_amount    = 0;
            a->c.hp          = 0;
            a->obj->motion   = STRIKE_MOTION_DOWN;
            a->obj->phase    = 0;
            BtlSePlay(STRIKE_VOICE, 1);
            break;
        }
        if ((o->attr & BTL_OBJ_CARRIED) != 0) {
            if (a->unkCC != 0) {
                if (BtlInflictStatus(a, a->unkCC) != 0
                    && (signed char)a->c.status != BTL_STATUS_NOINPUT) {
                    if ((signed char)a->c.status == BTL_STATUS_DOWN) {
                        a->resume_motion = o->motion;
                        a->resume_phase  = o->phase + 1;
                        a->hit_amount    = 0;
                        a->c.hp          = 0;
                        a->obj->motion   = STRIKE_MOTION_DOWN;
                        a->obj->phase    = 0;
                        BtlSePlay(STRIKE_VOICE, 1);
                    } else {
                        a->resume_motion = o->motion;
                        a->resume_phase  = o->phase + 1;
                        a->ail_turns     = ITEM_AIL_TURNS;
                        a->hit_amount    = 0;
                        o->motion        = STRIKE_MOTION_REEL;
                        BtlSePlay(STRIKE_SE_SLOT, 5);
                        o->phase = 0;
                    }
                }
                a->unkCC = 0;
                o->attr &= ~BTL_OBJ_CARRIED;
                break;
            }
            if ((signed char)a->c.status != BTL_STATUS_NOINPUT) {
                a->c.hp -= a->hit_amount;
                if (g_btl_debug_flags[1] != 0 && a->c.hp <= 0) {
                    a->c.hp = 1;
                }
                if (a->c.hp <= 0) {
                    a->resume_motion = o->motion;
                    a->resume_phase  = o->phase + 1;
                    o->motion        = STRIKE_MOTION_DOWN;
                    BtlSePlay(STRIKE_VOICE, 1);
                    g_btl_hits_left = 0;
                    o->phase        = 0;
                } else {
                    a->resume_motion = o->motion;
                    a->resume_phase  = o->phase + 1;
                    o->motion        = STRIKE_MOTION_REEL;
                    BtlSePlay(STRIKE_VOICE, 0);
                    o->phase = 0;
                }
                break;
            }
        }
        o->phase++;
        break;

    case 6:
        if (g_cd_busy != -1) {
            break;
        }
        BtlBoxDismiss();
        BtlReadyNextTurn();
        g_btl_scene_rgb[0] = ITEM_SCENE_LIT;
        g_btl_scene_rgb[1] = ITEM_SCENE_LIT;
        g_btl_scene_rgb[2] = ITEM_SCENE_LIT;
        BtlSoundClose(STRIKE_VOICE);
        o->motion = 0;
        o->phase  = 0;
        break;
    }
}
