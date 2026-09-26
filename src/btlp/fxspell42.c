/* Persona 1 (JP) - move 0x43's two records, and the moves that bring a fallen
 * fighter back.  BTLP only.
 *   0x800BB2EC BtlFxStart43  0x800BB420 BtlFxStart42  0x800BB5BC BtlFxStep42
 *
 * BtlFxStart43 is move 0x43's start handler: two records on the spot the
 * fighter aimed at stands on, the first out of the staged artwork's first
 * script table and marked FX_43_FIRST, the second out of the second, chained
 * to the first and answered.
 *
 * BtlFxStart42 and BtlFxStep42 run moves 0x42, 0x6B and 0x6C. The start stands
 * the fighter aimed at back on the field - on the cell its own record stood on
 * when a Persona took the turn in place, otherwise on the one the acting
 * fighter picked - puts it on its model's getting-up script, and hides it with
 * its colour already walked to half grey; an effect record is opened on it and
 * shown straight away.
 *
 * The step waits that record's timer out. Move 0x6B, and 0x6C when no Persona
 * is acting, can miss: one time in thirty-two the record is hidden, a message
 * goes up and the move goes straight to its end. Otherwise the fighter is shown
 * again with its palette whitened to fade back, the record dims, and once it is
 * dark the move lands. 0x6B brings the fighter back on a quarter of its hp and
 * 0x6C on all of it; 0x42 leaves it on none and unable to act. Each clears the
 * ailment's level and puts the fighter in the formation again - except that
 * 0x6C, taken in place by a Persona, stands it on its own cell, starts the
 * Persona over at its first rank and ends the Persona's turn. The fighter
 * then takes its wards from the first member still standing, and the acting
 * fighter's count at unkD0 goes up.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/model.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>
#include <persona/btlp/stats.h>
#include <persona/btlp/text.h>

/* The mark move 0x43's first record carries. */
#define FX_43_FIRST 1

/* The three moves, and the act kind that says a Persona took the turn in the
   fighter's place. */
#define FX_42_STILL    0x42
#define FX_42_SOME     0x6B
#define FX_42_ALL      0x6C
#define FX_42_IN_PLACE 3

/* The getting-up script in a member's row of g_btl_member_scripts, how fast
   the fighter's colour walks when it is stood up, and how long the effect
   record stands. */
#define SCRIPT_RISE   5
#define FX_42_AT_ONCE 0xFF
#define FX_42_HOLD    0x3C

/* The miss: its roll, the message and where it goes up, how long it stays,
   and the phase the move is sent to. */
#define FX_42_MISS_ROLL 0x1F
#define FX_42_MSG_FLAGS 1
#define FX_42_MSG_STYLE 1
#define FX_42_MSG_X     0x10
#define FX_42_MSG_Y     0xC
#define FX_42_MSG_TIME  0x3C
#define FX_42_MISSED    3

/* How long a washed fighter - every entry of its palette but nought - is
   left to fade back, and how fast and how long the record dims. */
#define FX_42_BRIGHT   0x1E
#define FX_42_DIM_FADE 8
#define FX_42_DIM      0x1E

/* What a Persona is started over with, and the actor flag its turn leaves. */
#define FX_42_FIRST_SLOTS 1
#define FX_42_PERSONA_ACT 0x10000000

/* The message move 0x6B and 0x6C put up when they miss. */
extern u_char g_btl_msg_revive_failed[];

BtlObj *BtlFxStart43(void)
{
    BtlObj *o;
    BtlObj *n;
    long    pos[3];

    pos[0] = g_btl_actors[g_btl_fx_target].obj->x;
    pos[1] = g_btl_actors[g_btl_fx_target].obj->y;
    pos[2] = 0;
    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0, pos,
                    FX_OBJ_CD, FX_OBJ_CE);
    o->mark_num = FX_43_FIRST;
    o->attr = FX_OBJ_ATTR;
    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[1];
    n = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0, pos,
                    FX_OBJ_CD, FX_OBJ_CE);
    n->attr = FX_OBJ_ATTR;
    n->attached = o;
    n->mark_num = 0;
    return n;
}

BtlObj *BtlFxStart42(void)
{
    BtlObj *o;
    BtlObj *n;
    const u_char *row;

    o = g_btl_actors[g_btl_fx_target].obj;
    if (g_btl_act_kind == FX_42_IN_PLACE) {
        BtlPlaceMember(o->mark_num, o->col2 >> 1, (u_char)o->row);
    } else {
        BtlPlaceMember(o->mark_num, g_btl_actors[g_btl_actor_turn].place_col,
                       g_btl_actors[g_btl_actor_turn].place_row);
    }
    /* The member's run of scripts is taken as a row first and the shape
       indexed off it: summed into one index, whatever the order of the
       terms, the table's address and the pick trade registers. */
    row = &g_btl_member_scripts[SCRIPT_RISE
        + g_btl_actors[g_btl_fx_target].c.key * MEMBER_SCRIPT_MODEL];
    BtlObjSetScript(o, (BtlSeqStep *)o->scripts[
        row[o->actor->script_pick * MEMBER_SCRIPT_PICK]]);
    o->fade = FX_42_AT_ONCE;
    o->rgb_to[0] = FX_GREY;
    o->rgb_to[1] = FX_GREY;
    o->rgb_to[2] = FX_GREY;
    g_btl_actors[g_btl_fx_target].obj->attr |= BTL_OBJ_HIDDEN;
    n = BtlOpenFxObj2(g_btl_fx_target, FX_TIMER);
    n->timer = FX_42_HOLD;
    n->attr &= ~(BTL_OBJ_HIDDEN | BTL_OBJ_STATIC);
    return n;
}

/* The miss is written out for each of the two moves that can miss.
   Cross-jumping folds the second copy into the first after scheduling, so
   the image shows one; before it, the copies mean the message's argument
   registers are set twice in the routine, and sched1 only pulls a register
   set once down to its use - which is why the image sets them ahead of the
   attribute's read. 0x42's hp written ahead of its status is what lets 0x6B
   share the tail from the ailment's level on, the way the image has it; the
   other two cases want theirs the other way. */
void BtlFxStep42(BtlObj *o)
{
    BtlObj   *t;
    BtlStats *p;
    int       i;

    t = g_btl_actors[g_btl_fx_target].obj;
    switch (o->phase) {
    case 0:
        if (o->timer != 0) {
            break;
        }
        if (g_btl_fx_move == FX_42_SOME) {
            if ((rand() & FX_42_MISS_ROLL) == 0) {
                o->attr |= BTL_OBJ_HIDDEN;
                BtlOpenMessage(FX_42_MSG_FLAGS, FX_42_MSG_STYLE,
                               g_btl_msg_revive_failed, FX_42_MSG_X,
                               FX_42_MSG_Y);
                o->timer = FX_42_MSG_TIME;
                g_btl_msg_timer = FX_42_MSG_TIME;
                o->phase = FX_42_MISSED;
            }
        } else if (g_btl_fx_move == FX_42_ALL && g_btl_act_kind == 0) {
            if ((rand() & FX_42_MISS_ROLL) == 0) {
                o->attr |= BTL_OBJ_HIDDEN;
                BtlOpenMessage(FX_42_MSG_FLAGS, FX_42_MSG_STYLE,
                               g_btl_msg_revive_failed, FX_42_MSG_X,
                               FX_42_MSG_Y);
                o->timer = FX_42_MSG_TIME;
                g_btl_msg_timer = FX_42_MSG_TIME;
                o->phase = FX_42_MISSED;
            }
        }
        if (o->phase != 0) {
            break;
        }
        t->attr &= ~(BTL_OBJ_HIDDEN | BTL_OBJ_NO_SHADOW);
        g_btl_clut_fading |= 1 << g_btl_fx_target;
        i = 1;
        do {
            g_btl_actor_clut[g_btl_fx_target * FX_CLUT_COLORS + i] = FX_CLUT_WHITE;
            i++;
        } while (i < FX_CLUT_COLORS);
        o->timer = FX_42_BRIGHT;
        o->phase++;
        break;
    case 1:
        if (o->timer != 0) {
            break;
        }
        o->fade = FX_42_DIM_FADE;
        o->rgb_to[0] = 0;
        o->rgb_to[1] = 0;
        o->rgb_to[2] = 0;
        o->timer = FX_42_DIM;
        o->phase++;
        break;
    case 2:
        if (o->timer != 0) {
            break;
        }
        switch (g_btl_fx_move) {
        case FX_42_SOME:
            g_btl_actors[g_btl_fx_target].c.hp =
                g_btl_actors[g_btl_fx_target].c.hp_max / 4;
            g_btl_actors[g_btl_fx_target].c.status = 0;
            g_btl_actors[g_btl_fx_target].c.ail_level = 0;
            g_btl_formation[g_btl_actors[g_btl_actor_turn].place_row * FX_GRID_W
                            + g_btl_actors[g_btl_actor_turn].place_col] =
                g_btl_fx_target;
            break;
        case FX_42_ALL:
            g_btl_actors[g_btl_fx_target].c.hp =
                g_btl_actors[g_btl_fx_target].c.hp_max;
            g_btl_actors[g_btl_fx_target].c.status = 0;
            g_btl_actors[g_btl_fx_target].c.ail_level = 0;
            if (g_btl_act_kind == FX_42_IN_PLACE) {
                g_btl_formation[t->row * FX_GRID_W + (t->col2 >> 1)] =
                    g_btl_fx_target;
                p = &g_btl_personas[BtlActorPersona(g_btl_fx_target)];
                p->slots = FX_42_FIRST_SLOTS;
                p->exp = 0;
                p->rank_exp = 0;
                p->rank_left = g_btl_persona_rank_exp[0];
                p->no_growth = 1;
                BtlEnemyDeriveStats(p);
                g_btl_actors[g_btl_fx_target].flags &= ~FX_42_PERSONA_ACT;
                g_btl_act_kind = 0;
                break;
            }
            g_btl_formation[g_btl_actors[g_btl_actor_turn].place_row * FX_GRID_W
                            + g_btl_actors[g_btl_actor_turn].place_col] =
                g_btl_fx_target;
            break;
        case FX_42_STILL:
            g_btl_actors[g_btl_fx_target].c.hp = 0;
            g_btl_actors[g_btl_fx_target].c.status = BTL_STATUS_NOINPUT;
            g_btl_actors[g_btl_fx_target].c.ail_level = 0;
            g_btl_formation[g_btl_actors[g_btl_actor_turn].place_row * FX_GRID_W
                            + g_btl_actors[g_btl_actor_turn].place_col] =
                g_btl_fx_target;
            break;
        }
        for (i = 0; i < BTL_PARTY; i++) {
            if (g_btl_actors[i].c.key != 0
                && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
                && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
                g_btl_actors[g_btl_fx_target].flags |=
                    g_btl_actors[i].flags & BTL_ACTOR_WARDS;
                break;
            }
        }
        g_btl_actors[g_btl_actor_turn].unkD0++;
        o->phase++;
        break;
    case 3:
        if (o->timer != 0) {
            break;
        }
        o->motion = 0;
        o->phase = 0;
        o->attr |= BTL_OBJ_HIDDEN;
        break;
    }
}
