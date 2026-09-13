/* Persona 1 (JP) - putting both sides back as the field shows them.
 * BTLP only.
 *   0x800C4B80 BtlEnemiesResetGfx  0x800C4E54 BtlPartyResetGfx
 *   0x800C5254 BtlMemberResetGfx
 *
 * BtlEnemiesResetGfx does for every occupied enemy slot what the party's does
 * for a member, with the enemy's own palettes and scripts: its flinch while it
 * is flagged as having flinched, the script it was spawned on otherwise. It
 * has no weak stance and no shadow to show, and it clears g_btl_reach too.
 * BtlMemberResetGfx is the party's for one member, whatever its state.
 *
 * The counterpart of BtlDimParty, and what a pick or a menu calls once it is
 * done with the party. Every member still in the fight gets its palette back
 * from the base copy - both the one drawn and the one it is walking toward -
 * and the script it stands in: its weak stance once its hp is down to a
 * quarter, its flinch while it is flagged as having flinched, its ordinary
 * stance otherwise. It is let go of whatever it was doing, walked back to full
 * colour along with its ailment marker, has its shadow shown again and is
 * tinted for its ailment. A member who is down or out of the fight is hidden
 * instead, marker and shadow with it.
 *
 * Last, every cell of g_btl_reach is cleared, so no reach stays lit once the
 * party is no longer being picked from.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/model.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>

/* A palette is 0x200 bytes, one page per actor. */
#define CLUT_BYTES 0x200

/* Three of a member's ten scripts: the one it stands in, the one it stands in
   once badly hurt, and its flinch. */
#define SCRIPT_STAND        0
#define SCRIPT_WEAK         6
#define SCRIPT_HIT          8

/* Full colour, and how fast a member is walked back to it. */
#define RESET_LEVEL 0x80
#define RESET_FADE  8

extern u_char *g_btl_actor_clut;
extern u_char *g_btl_actor_clut_to;
extern u_char *g_btl_actor_clut_base;
extern u_char *g_btl_enemy_clut;
extern u_char *g_btl_enemy_clut_to;
extern u_char *g_btl_enemy_clut_base;

void BtlEnemiesResetGfx(void)
{
    BtlObj *o;
    short   level;
    u_char  fade;
    u_char  script;
    u_char *cell;
    int     i;

    for (i = 0; i < BTL_ENEMIES; i++) {
        if (g_btl_combatants[i].c.key != 0) {
            o = g_btl_combatants[i].obj;
            memcpy(g_btl_enemy_clut + i * CLUT_BYTES,
                   g_btl_enemy_clut_base + i * CLUT_BYTES, CLUT_BYTES);
            memcpy(g_btl_enemy_clut_to + i * CLUT_BYTES,
                   g_btl_enemy_clut_base + i * CLUT_BYTES, CLUT_BYTES);
            if (o->actor->flags & BTL_ACTOR_FLINCHED) {
                script = g_btl_models[o->kind].hit;
            } else {
                script = g_btl_models[o->kind].spawn;
            }
            BtlObjSetScript(o, (BtlSeqStep *)o->scripts[script]);
            level = RESET_LEVEL;
            fade = RESET_FADE;
            o->motion = 0;
            o->fade = fade;
            o->rgb_to[0] = level;
            o->rgb_to[1] = level;
            o->rgb_to[2] = level;
            o->attr &= ~BTL_OBJ_PICKED;
            o->mark->fade = fade;
            o->mark->rgb_to[0] = level;
            o->mark->rgb_to[1] = level;
            o->mark->rgb_to[2] = level;
            BtlObjStatusTint(o);
        }
    }
    cell = (u_char *)g_btl_reach;
    for (i = 0; i < REACH_CELLS; i++) {
        *cell++ = 0;
    }
}

void BtlPartyResetGfx(void)
{
    BtlObj *o;
    short   level;
    u_char  fade;
    u_char *row;
    u_char *cell;
    int     i;

    for (i = 0; i < BTL_PARTY; i++) {
        if (g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
            o = g_btl_actors[i].obj;
            memcpy(g_btl_actor_clut + i * CLUT_BYTES,
                   g_btl_actor_clut_base + i * CLUT_BYTES, CLUT_BYTES);
            memcpy(g_btl_actor_clut_to + i * CLUT_BYTES,
                   g_btl_actor_clut_base + i * CLUT_BYTES, CLUT_BYTES);
            if (o->actor->c.hp_max / 4 >= o->actor->c.hp) {
                row = &g_btl_member_scripts[SCRIPT_WEAK
                                            + o->kind * MEMBER_SCRIPT_MODEL];
            } else if (o->actor->flags & BTL_ACTOR_FLINCHED) {
                row = &g_btl_member_scripts[SCRIPT_HIT
                                            + o->kind * MEMBER_SCRIPT_MODEL];
            } else {
                row = &g_btl_member_scripts[SCRIPT_STAND
                                            + o->kind * MEMBER_SCRIPT_MODEL];
            }
            BtlObjSetScript(o, (BtlSeqStep *)o->scripts[
                row[o->actor->script_pick * MEMBER_SCRIPT_PICK]]);
            /* Held in locals: written as constants, the fade is hoisted
               ahead of the colour and the two saved registers swap. */
            level = RESET_LEVEL;
            fade = RESET_FADE;
            o->motion = 0;
            o->phase = 0;
            o->fade = fade;
            o->rgb_to[0] = level;
            o->rgb_to[1] = level;
            o->rgb_to[2] = level;
            o->attr &= ~BTL_OBJ_PICKED;
            o->mark->fade = fade;
            o->mark->rgb_to[0] = level;
            o->mark->rgb_to[1] = level;
            o->mark->rgb_to[2] = level;
            BtlObjClearAttr(o->shadow, BTL_OBJ_HIDDEN);
            BtlObjStatusTint(o);
        } else if (g_btl_actors[i].c.key != 0) {
            o = g_btl_actors[i].obj;
            o->attr |= BTL_OBJ_HIDDEN;
            o->mark->attr |= BTL_OBJ_HIDDEN;
            o->shadow->attr |= BTL_OBJ_HIDDEN;
            o->motion = 0;
            o->phase = 0;
        }
    }
    cell = (u_char *)g_btl_reach;
    for (i = 0; i < REACH_CELLS; i++) {
        *cell++ = 0;
    }
}

void BtlMemberResetGfx(int slot)
{
    BtlObj *o;
    u_char *row;

    o = g_btl_actors[slot].obj;
    memcpy(g_btl_actor_clut + slot * CLUT_BYTES,
           g_btl_actor_clut_base + slot * CLUT_BYTES, CLUT_BYTES);
    memcpy(g_btl_actor_clut_to + slot * CLUT_BYTES,
           g_btl_actor_clut_base + slot * CLUT_BYTES, CLUT_BYTES);
    if (o->actor->c.hp_max / 4 >= o->actor->c.hp) {
        row = &g_btl_member_scripts[SCRIPT_WEAK + o->kind * MEMBER_SCRIPT_MODEL];
    } else if (o->actor->flags & BTL_ACTOR_FLINCHED) {
        row = &g_btl_member_scripts[SCRIPT_HIT + o->kind * MEMBER_SCRIPT_MODEL];
    } else {
        row = &g_btl_member_scripts[SCRIPT_STAND + o->kind * MEMBER_SCRIPT_MODEL];
    }
    BtlObjSetScript(o, (BtlSeqStep *)o->scripts[
        row[o->actor->script_pick * MEMBER_SCRIPT_PICK]]);
    o->motion = 0;
    o->phase = 0;
    o->fade = RESET_FADE;
    o->rgb_to[0] = RESET_LEVEL;
    o->rgb_to[1] = RESET_LEVEL;
    o->rgb_to[2] = RESET_LEVEL;
    o->attr &= ~BTL_OBJ_PICKED;
    o->mark->fade = RESET_FADE;
    o->mark->rgb_to[0] = RESET_LEVEL;
    o->mark->rgb_to[1] = RESET_LEVEL;
    o->mark->rgb_to[2] = RESET_LEVEL;
    BtlObjClearAttr(o->shadow, BTL_OBJ_HIDDEN);
    BtlObjStatusTint(o);
}
