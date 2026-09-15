/* Persona 1 (JP) - the attack, cast and item commands.  BTLP only.
 *   0x800A0D54 BtlCommandAttack  0x800A1608 BtlCommandCast
 *   0x800A2238 BtlCommandItem
 *
 * Entries 0 to 3 of g_btl_command_fn, the handler BtlCommandEntry gives a
 * member once a command is picked: the attack twice - the melee weapon and
 * the gun are one routine, told apart by which help row the menu was on - the
 * cast and the item. Each runs its own frame loop on g_btl_step and answers 1
 * once the member has an order and a target, 0 when the player backed out and
 * -2 on the third key.
 *
 * The cast's spell list, when it is the debug board's, and the item board both
 * stand the cursor on a page of ten spots kept here; the item board walks the
 * neighbour table beside them, and a step off either end turns its page.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/board.h>
#include <persona/btlp/clut.h>
#include <persona/btlp/input.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/round.h>
#include <persona/btlp/stage.h>
#include <persona/btlp/text.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/stats.h>
#include <persona/common/item.h>
#include <persona/common/persona.h>

/* How the swing is aimed, in the low nibble of ItemDef.swing: at the slowest
   enemy, at every enemy in reach, at the whole enemy side, or at the
   weakest. */
#define SWING_AIM_MASK    0xF
#define SWING_AIM_SLOWEST 1
#define SWING_AIM_REACH   2
#define SWING_AIM_SIDE    4
#define SWING_AIM_WEAKEST 8

/* What the enemies the swing reaches are drawn at, and what the rest are
   dimmed to. */
#define TARGET_MOTION 10
#define TARGET_DIM    0x20
#define TARGET_FADE   8
#define TARGET_LIT    0x80

/* More hp than any enemy has, to start the search for the weakest from. */
#define TARGET_HP_MAX 99999

int BtlCommandAttack(void)
{
    long           unused[4];
    ItemDef       *item;
    u_int          least;
    int            pick;
    int            i;

    for (;;) {
        switch (g_btl_step) {
        case 0:
            if (g_btl_pick_help_row2 == 0) {
                item = &g_item_defs[g_btl_actors[g_btl_actor_turn].c.equip[0]];
            } else {
                if (g_btl_actors[g_btl_actor_turn].c.equip[1] != 0) {
                    if (g_btl_actors[g_btl_actor_turn].c.equip[2] != 0) {
                        item = &g_item_defs[g_btl_actors[g_btl_actor_turn]
                                                .c.equip[1]];
                        goto aim;
                    }
                    BtlOpenMessage(0, 0, g_btl_msg_no_ammo, PICK_HELP_X,
                                   PICK_HELP_Y);
                    g_btl_step = 6;
                    break;
                }
                BtlOpenMessage(0, 0, g_btl_msg_no_gun, PICK_HELP_X, PICK_HELP_Y);
                g_btl_step = 6;
                break;
            }
        aim:
            if ((signed char)g_btl_actors[g_btl_actor_turn].c.status
                == BTL_STATUS_GUILT) {
                BtlOpenMessage(0, 0, g_btl_msg_guilt, PICK_HELP_X, PICK_HELP_Y);
                do {
                    BtlDrawFrame();
                } while (g_btl_pad1_edge == 0);
                if (g_btl_no_help != 0) {
                    BtlCloseMessage(0);
                }
                return 0;
            }
            if (BtlMarkMoveArea(&g_btl_actors[g_btl_actor_turn], item->area,
                                item->swing)
                < 0) {
                BtlOpenMessage(0, 0, g_btl_msg_out_of_reach, PICK_HELP_X, PICK_HELP_Y);
                BtlDimEnemies();
                g_btl_step++;
            } else {
                g_btl_step += 2;
            }
            g_btl_actors[g_btl_actor_turn].targets = 0;
            break;
        case 1:
            if (g_btl_pad1_edge == 0) {
                break;
            }
            if (g_btl_no_help != 0) {
                BtlCloseMessage(0);
            }
            goto reset;
        case 2:
            switch (item->swing & SWING_AIM_MASK) {
            case SWING_AIM_SLOWEST:
                g_btl_enemy_slot = BtlSlowestOrder();
                g_btl_step = 3;
                break;
            case SWING_AIM_REACH:
            case SWING_AIM_SIDE:
                for (i = 0; i < BTL_ENEMIES; i++) {
                    if (g_btl_combatants[i].c.key != 0) {
                        if (g_btl_combatants[i].pickable != 0) {
                            g_btl_combatants[i].obj->motion = TARGET_MOTION;
                            BtlTintActorClut(i + BTL_PARTY, g_btl_tint_pick_r[0],
                                             g_btl_tint_pick_r[1],
                                             g_btl_tint_pick_r[2]);
                        } else {
                            g_btl_combatants[i].obj->rgb_to[0] = TARGET_DIM;
                            g_btl_combatants[i].obj->rgb_to[1] = TARGET_DIM;
                            g_btl_combatants[i].obj->rgb_to[2] = TARGET_DIM;
                            g_btl_combatants[i].obj->fade = TARGET_FADE;
                        }
                    }
                }
                g_btl_step = 4;
                break;
            case SWING_AIM_WEAKEST:
                i = 0;
                least = TARGET_HP_MAX;
                for (; i < BTL_ENEMIES; i++) {
                    if (g_btl_combatants[i].c.key != 0) {
                        g_btl_combatants[i].obj->rgb_to[0] = TARGET_DIM;
                        g_btl_combatants[i].obj->rgb_to[1] = TARGET_DIM;
                        g_btl_combatants[i].obj->rgb_to[2] = TARGET_DIM;
                        g_btl_combatants[i].obj->fade = TARGET_FADE;
                        if (g_btl_combatants[i].pickable != 0) {
                            g_btl_combatants[i].pickable = 0;
                            if ((u_int)g_btl_combatants[i].c.hp < least) {
                                least = g_btl_combatants[i].c.hp;
                                g_btl_enemy_slot = i;
                            }
                        }
                    }
                }
                BtlTintActorClut(g_btl_enemy_slot + BTL_PARTY,
                                 g_btl_tint_pick_r[0], g_btl_tint_pick_r[1],
                                 g_btl_tint_pick_r[2]);
                g_btl_combatants[g_btl_enemy_slot].obj->rgb_to[0] = TARGET_LIT;
                g_btl_combatants[g_btl_enemy_slot].obj->rgb_to[1] = TARGET_LIT;
                g_btl_combatants[g_btl_enemy_slot].obj->rgb_to[2] = TARGET_LIT;
                g_btl_combatants[g_btl_enemy_slot].obj->fade = 0xFF;
                g_btl_combatants[g_btl_enemy_slot].obj->motion = TARGET_MOTION;
                g_btl_combatants[g_btl_enemy_slot].pickable = 1;
                g_btl_step = 3;
                break;
            }
            break;
        case 3:
            pick = BtlPickEnemy(&g_btl_enemy_slot);
            switch (pick) {
            case -2:
                goto abort;
            case BTL_PICK_WAIT:
                break;
            case -1:
                goto cancel;
            default:
                i = 0;
                g_btl_actors[g_btl_actor_turn].order = pick + BTL_PARTY;
                g_btl_actors[g_btl_actor_turn].targets = 0;
                for (; i < BTL_ENEMIES; i++) {
                    if (g_btl_combatants[i].c.key != 0 && g_btl_combatants[i].pickable != 0) {
                        g_btl_actors[g_btl_actor_turn].targets |=
                            1 << (i + BTL_PARTY);
                    }
                }
                g_btl_step = 5;
                break;
            }
            break;
        case 4:
            if (g_btl_pad1_edge & g_btl_key_confirm) {
                i = 0;
                g_btl_actors[g_btl_actor_turn].order =
                    BtlSlowestOrder() + BTL_PARTY;
                g_btl_actors[g_btl_actor_turn].targets = 0;
                for (; i < BTL_ENEMIES; i++) {
                    if (g_btl_combatants[i].c.key != 0 && g_btl_combatants[i].pickable != 0) {
                        g_btl_actors[g_btl_actor_turn].targets |=
                            1 << (i + BTL_PARTY);
                    }
                }
                BtlEnemiesResetGfx();
                g_btl_step = 5;
                break;
            }
            if (g_btl_pad1_edge & g_btl_key_cancel) {
            cancel:
                BtlSePlay(1, 2);
            reset:
                BtlEnemiesResetGfx();
                return 0;
            }
            if (g_btl_pad1_edge & g_btl_key_abort) {
            abort:
                BtlSePlay(1, 2);
                BtlEnemiesResetGfx();
                return -2;
            }
            break;
        case 5:
            if (g_btl_marker_shown[g_btl_actor_turn] == NULL
                && BtlObjChainAtMotion(g_btl_marker_obj[g_btl_actor_turn], 0)
                       != 0) {
                BtlEnemiesResetGfx();
                return 1;
            }
            break;
        case 6:
            if (g_btl_pad1_edge != 0) {
                if (g_btl_no_help != 0) {
                    BtlCloseMessage(0);
                }
                return 0;
            }
            break;
        }
        BtlDrawFrame();
    }
}

/* Spells the cast will not give a member: the first is turned away with the
   cancel sound, the second with a line while the fight is a tame one. */
#define CAST_SPELL_REFUSED 0x73
#define CAST_SPELL_UNTAMED 0xA2

/* The step the cast waits for the member's marker on. */
#define CAST_STEP_WAIT 10

/* Where the spell and item help lines go. */
#define COMMAND_HELP_Y 0x8C

/* Rows to a page of the item board, and the bits that say a page lies before
   or after this one. The board is turned back on motion 7 and on on 6. */
#define ITEM_ROWS      10
#define ITEM_PAGE_PREV 1
#define ITEM_PAGE_NEXT 2
#define ITEM_TURN_BACK 7
#define ITEM_TURN_ON   6

BtlMenuSpot g_btl_spell_spots[ITEM_ROWS] = {
    { -0x80, -0x1E }, { 0, -0x1E }, { -0x80, -0x12 }, { 0, -0x12 },
    { -0x80, -0x06 }, { 0, -0x06 }, { -0x80, 0x06 },  { 0, 0x06 },
    { -0x80, 0x12 },  { 0, 0x12 },
};

signed char g_btl_item_nav[ITEM_ROWS][BTL_NAV_WAYS] = {
    { -2, 2, 1 }, { -1, 3, 0 }, { 0, 4, 3 }, { 1, 5, 2 },  { 2, 6, 5 },
    { 3, 7, 4 },  { 4, 8, 7 },  { 5, 9, 6 }, { 6, 10, 9 }, { 7, 11, 8 },
};

BtlMenuSpot g_btl_item_spots[ITEM_ROWS] = {
    { -0x80, -0x1E }, { 0, -0x1E }, { -0x80, -0x12 }, { 0, -0x12 },
    { -0x80, -0x06 }, { 0, -0x06 }, { -0x80, 0x06 },  { 0, 0x06 },
    { -0x80, 0x12 },  { 0, 0x12 },
};

int g_btl_item_abort = 0;

int BtlCommandCast(void)
{
    BtlObj     *board;
    BtlGfxCell *cell;
    int         persona;
    int         prev;
    int         pick;
    int         spell;
    int         i;
    int         n;

    g_btl_cast_debug = 0;
    BtlDrawFrame();
    if (g_btl_debug_hud != 0
        && (g_btl_pad1 & (g_btl_key_select | g_btl_key_square | g_btl_key_r2))
               == (g_btl_key_select | g_btl_key_square | g_btl_key_r2)) {
        g_btl_cast_debug = 1;
    }
    g_btl_cast_abort = 0;
    for (;;) {
        switch (g_btl_step) {
        case 0:
            if (g_btl_actors[g_btl_actor_turn].c.entry == CHAR_NO_ENTRY) {
                BtlOpenMessage(0, 0, g_btl_msg_persona_blocked, PICK_HELP_X,
                               PICK_HELP_Y);
                while (g_btl_pad1_edge == 0) {
                    BtlDrawFrame();
                }
                if (g_btl_no_help != 0) {
                    BtlCloseMessage(0);
                }
                return 0;
            }
            if (g_btl_actors[g_btl_actor_turn].c.blocked != 0) {
                BtlOpenMessage(0, 0, g_btl_msg_cast_sealed, PICK_HELP_X,
                               PICK_HELP_Y);
                while (g_btl_pad1_edge == 0) {
                    BtlDrawFrame();
                }
                if (g_btl_no_help != 0) {
                    BtlCloseMessage(0);
                }
                return 0;
            }
            persona = BtlActorPersona(g_btl_actor_turn);
            if (g_btl_debug_free_sp == 0
                && g_btl_actors[g_btl_actor_turn].c.sp
                       < g_btl_personas[persona].sp_cost) {
                BtlOpenMessage(0, 0, g_btl_msg_short_sp, PICK_HELP_X,
                               PICK_HELP_Y);
                while (g_btl_pad1_edge == 0) {
                    BtlDrawFrame();
                }
                return 0;
            }
            if (((g_persona_defs[g_btl_personas[persona].key].bond
                  >> ((g_btl_actors[g_btl_actor_turn].c.key - 1)
                      * PERSONA_BOND_BITS))
                 & PERSONA_BOND_MASK)
                < PERSONA_BOND_WILLING) {
                BtlOpenMessage(0, 0, g_btl_msg_persona_unwilling, PICK_HELP_X,
                               PICK_HELP_Y);
                while (g_btl_pad1_edge == 0) {
                    BtlDrawFrame();
                }
                return 0;
            }
            if ((signed char)g_btl_actors[g_btl_actor_turn].c.status
                == BTL_STATUS_LIFTED) {
                BtlOpenMessage(0, 0, g_btl_msg_lifted, PICK_HELP_X,
                               PICK_HELP_Y);
                while (g_btl_pad1_edge == 0) {
                    BtlDrawFrame();
                }
                return 0;
            }
            if ((signed char)g_btl_actors[g_btl_actor_turn].c.status
                    == BTL_STATUS_GUILT
                && (signed char)g_btl_actors[g_btl_actor_turn].c.ail_level > 0) {
                BtlOpenMessage(0, 0, g_btl_msg_guilt, PICK_HELP_X, PICK_HELP_Y);
                while (g_btl_pad1_edge == 0) {
                    BtlDrawFrame();
                }
                return 0;
            }
            if ((signed char)g_btl_actors[g_btl_actor_turn].c.status
                    == BTL_STATUS_CLOSE
                && (signed char)g_btl_actors[g_btl_actor_turn].c.ail_level
                       == CHAR_AIL_LEVEL_MAX) {
                BtlOpenMessage(0, 0, g_btl_msg_closed, PICK_HELP_X,
                               PICK_HELP_Y);
                while (g_btl_pad1_edge == 0) {
                    BtlDrawFrame();
                }
                return 0;
            }
            if ((signed char)g_btl_actors[g_btl_actor_turn].c.status
                    == BTL_STATUS_BLIND
                && (signed char)g_btl_actors[g_btl_actor_turn].c.ail_level
                       == CHAR_AIL_LEVEL_MAX) {
                BtlOpenMessage(0, 0, g_btl_msg_blind, PICK_HELP_X, PICK_HELP_Y);
                while (g_btl_pad1_edge == 0) {
                    BtlDrawFrame();
                }
                return 0;
            }
            if ((signed char)g_btl_actors[g_btl_actor_turn].c.status
                == BTL_STATUS_NOINPUT) {
                BtlOpenMessage(0, 0, g_btl_msg_puppet, PICK_HELP_X,
                               PICK_HELP_Y);
                while (g_btl_pad1_edge == 0) {
                    BtlDrawFrame();
                }
                return 0;
            }
            prev = BtlUnreadyMemberPrev(g_btl_actor_turn);
            if (prev >= 0 && g_btl_marker_shown[prev] != NULL) {
                break;
            }
            BtlRetractMarkers();
            if (g_btl_cast_debug != 0) {
                BtlOpenSpellBoard();
                BtlObjLast(g_btl_list_board);
                board = g_btl_obj_prev;
            } else {
                BtlOpenPersonaBoard();
            }
            for (i = 0; i < BTL_STATS_SPELLS; i++) {
                if (g_btl_personas[persona].spell[i] != 0
                    && g_btl_personas[persona].spell[i]
                           == g_btl_actors[g_btl_actor_turn].move) {
                    g_btl_persona_spell_row = i;
                    break;
                }
            }
            if (i >= BTL_STATS_SPELLS
                || g_btl_personas[persona].spell[g_btl_persona_spell_row] == 0) {
                g_btl_persona_spell_row = 0;
            }
            g_btl_step++;
            break;
        case 1:
            if (g_btl_cast_debug != 0) {
                pick = BtlSpellMenuUpdate(board);
            } else {
                pick = BtlPersonaSpellUpdate(&g_btl_persona_spell_row);
            }
            if (g_btl_cast_abort != 0) {
                pick = BTL_PICK_ABORT;
            }
            switch (pick) {
            case BTL_PICK_CANCEL:
                BtlSePlay(1, 2);
                if (g_btl_cast_debug != 0) {
                    BtlCloseListBoard();
                } else {
                    BtlClosePersonaBoard();
                }
                BtlRefreshMarkers();
                BtlSingleOutMember(g_btl_actor_turn);
                return 0;
            case BTL_PICK_ABORT:
                BtlSePlay(1, 2);
                if (g_btl_cast_debug != 0) {
                    BtlCloseListBoard();
                } else {
                    BtlClosePersonaBoard();
                }
                BtlRefreshMarkers();
                return BTL_PICK_ABORT;
            case BTL_PICK_WAIT:
                break;
            default:
                BtlSePlay(1, 1);
                if (g_btl_cast_debug != 0) {
                    g_btl_actors[g_btl_actor_turn].move = pick;
                } else {
                    persona = BtlActorPersona(g_btl_actor_turn);
                    n = 0;
                    for (i = 0; i < BTL_STATS_SPELLS; i++) {
                        spell = g_btl_personas[persona].spell[i];
                        if (spell != 0) {
                            g_btl_cast_spells[n++] =
                                g_btl_personas[persona].spell[i];
                        }
                    }
                    spell = g_btl_cast_spells[pick];
                    if (spell == CAST_SPELL_REFUSED) {
                        BtlSePlay(1, 2);
                        break;
                    }
                    if (spell == CAST_SPELL_UNTAMED
                        && g_btl_ai_set == BTL_AI_SET_TAME) {
                        BtlOpenMessage(0, 0, g_btl_msg_spell_untamed,
                                       PICK_HELP_X, COMMAND_HELP_Y);
                        do {
                            BtlDrawFrame();
                        } while (g_btl_pad1_edge == 0);
                        BtlCloseMessage(0);
                        break;
                    }
                    g_btl_actors[g_btl_actor_turn].move = spell;
                    for (i = 0; i < BTL_STATS_SPELLS; i++) {
                        if (g_btl_personas[persona].raw[i] == spell) {
                            g_btl_actors[g_btl_actor_turn].obj->spell_slot = i;
                            break;
                        }
                    }
                    if (i >= BTL_STATS_SPELLS) {
                        g_btl_actors[g_btl_actor_turn].obj->spell_slot = 0;
                    }
                }
                g_btl_step = 2;
                break;
            }
            break;
        case 2:
            pick = BtlPickMoveTarget(&g_btl_actors[g_btl_actor_turn],
                                     g_btl_actors[g_btl_actor_turn].move);
            switch (pick) {
            case BTL_PICK_CANCEL:
                BtlSePlay(1, 2);
                g_btl_step = 1;
                break;
            case BTL_PICK_ABORT:
                g_btl_cast_abort = 1;
                BtlSePlay(1, 2);
                g_btl_step = 1;
                break;
            default:
                BtlEnemiesResetGfx();
                BtlPartyResetGfx();
                BtlRefreshMarkers();
                g_btl_step = CAST_STEP_WAIT;
                break;
            }
            break;
        case CAST_STEP_WAIT:
            if (BtlObjChainAtMotion(g_btl_marker_obj[g_btl_actor_turn], 0)
                != 0) {
                return 1;
            }
            break;
        }
        if (g_btl_cast_debug != 0) {
            for (i = 0, cell = g_btl_menu_cursor; i < BTL_CURSOR_CELLS;
                 i++, cell++) {
                cell->x = g_btl_spell_spots[g_btl_spell_row].x - BTL_CURSOR_NUDGE;
                cell->y = g_btl_spell_spots[g_btl_spell_row].y;
            }
        }
        BtlDrawFrame();
    }
}

int BtlCommandItem(void)
{
    BtlObj     *board;
    BtlGfxCell *cell;
    int         keys;
    int         pages;
    int         prev;
    int         row;
    int         pick;
    int         i;

    g_btl_item_abort = 0;
    BtlDrawFrame();
    for (;;) {
        switch (g_btl_step) {
        case 0:
            g_btl_item_at = BtlFirstUsableItem();
            if (g_btl_item_at == NULL) {
                BtlOpenMessage(0, 0, g_btl_msg_no_items, PICK_HELP_X,
                               PICK_HELP_Y);
                while (g_btl_pad1_edge == 0) {
                    BtlDrawFrame();
                }
                if (g_btl_no_help != 0) {
                    BtlCloseMessage(0);
                }
                return 0;
            }
            prev = BtlUnreadyMemberPrev(g_btl_actor_turn);
            if (prev >= 0 && g_btl_marker_shown[prev] != NULL) {
                break;
            }
            BtlRetractMarkers();
            BtlOpenItemBoard();
            BtlObjLast(g_btl_list_board);
            board = g_btl_obj_prev;
            g_btl_step++;
            g_btl_item_row = 0;
            if (g_btl_no_help == 0) {
                BtlOpenMessage(0, 0, g_item_help[g_btl_item_slots[0] & ITEM_ID],
                               PICK_HELP_X, COMMAND_HELP_Y);
            } else {
                BtlCloseMessage(0);
            }
            break;
        case 1:
            keys = (u_short)BtlMenuKey();
            if (board->motion == 0) {
                pages = 0;
                if ((g_btl_pad1 & PAD_DIRS) == 0) {
                    if (g_btl_no_help == 0) {
                        BtlOpenMessage(
                            0, 0,
                            g_item_help[g_btl_item_slots[g_btl_item_row]
                                        & ITEM_ID],
                            PICK_HELP_X, COMMAND_HELP_Y);
                    } else {
                        BtlCloseMessage(0);
                    }
                }
                if (BtlPrevUsableItem(g_btl_item_at) != NULL) {
                    pages |= ITEM_PAGE_PREV;
                }
                if (g_btl_item_slots[ITEM_ROWS] & ITEM_ID) {
                    pages |= ITEM_PAGE_NEXT;
                }
                row = g_btl_item_row;
                if (keys & PAD_UP) {
                    g_btl_item_row = g_btl_item_nav[row][BTL_NAV_UP];
                }
                if (keys & PAD_DOWN) {
                    g_btl_item_row = g_btl_item_nav[g_btl_item_row][BTL_NAV_DOWN];
                }
                if (keys & PAD_LEFT) {
                    g_btl_item_row = g_btl_item_nav[g_btl_item_row][BTL_NAV_SIDE];
                }
                if (keys & PAD_RIGHT) {
                    g_btl_item_row = g_btl_item_nav[g_btl_item_row][BTL_NAV_SIDE];
                }
                if (keys & (PAD_RIGHT | PAD_DOWN | PAD_LEFT)) {
                    if ((g_btl_item_slots[g_btl_item_row] & ITEM_ID) == 0
                        || (g_btl_item_slots[g_btl_item_row] >> ITEM_SHIFT)
                               == 0) {
                        if (g_btl_item_row == ITEM_ROWS + 1) {
                            g_btl_item_row = ITEM_ROWS;
                        } else {
                            g_btl_item_row = row;
                        }
                    }
                }
                if ((keys & PAD_DIRS) && (u_short)g_btl_item_row < ITEM_ROWS) {
                    BtlSePlay(1, 0);
                }
                if (g_btl_item_row < 0) {
                    g_btl_item_row += 2;
                    if (!(pages & ITEM_PAGE_PREV)) {
                        break;
                    }
                    board->motion = ITEM_TURN_BACK;
                    BtlSePlay(1, 0);
                    break;
                }
                if (g_btl_item_row >= ITEM_ROWS) {
                    g_btl_item_row -= 2;
                    if (!(pages & ITEM_PAGE_NEXT)) {
                        break;
                    }
                    board->motion = ITEM_TURN_ON;
                    BtlSePlay(1, 0);
                    break;
                }
            }
            if ((g_btl_pad1_edge & g_btl_key_confirm) && board->motion == 0) {
                if (g_btl_no_help == 0) {
                    BtlOpenMessage(0, 0,
                                   g_item_help[g_btl_item_slots[g_btl_item_row]
                                               & ITEM_ID],
                                   PICK_HELP_X, COMMAND_HELP_Y);
                } else {
                    BtlCloseMessage(0);
                }
                BtlSePlay(1, 1);
                g_btl_item_used = g_btl_item_slots[g_btl_item_row] & ITEM_ID;
                if (g_item_defs[g_btl_item_used].move != 0) {
                    pick = BtlPickMoveTarget(&g_btl_actors[g_btl_actor_turn],
                                             g_item_defs[g_btl_item_used].move);
                    switch (pick) {
                    case BTL_PICK_CANCEL:
                        break;
                    case BTL_PICK_ABORT:
                        g_btl_item_abort = 1;
                        break;
                    default:
                        BtlEnemiesResetGfx();
                        BtlPartyResetGfx();
                        BtlRefreshMarkers();
                        g_btl_actors[g_btl_actor_turn].ail_line = g_btl_item_used;
                        g_btl_actors[g_btl_actor_turn].move =
                            g_item_defs[g_btl_item_used].move;
                        g_btl_step++;
                        break;
                    }
                    break;
                }
                /* The rest are aimed by hand: four at one member, two at the
                   party and three at the enemy side, each given its move once
                   the target is chosen. */
                switch (g_btl_item_used) {
                case 0x1A:
                case 0x1B:
                case 0x1C:
                case 0x22:
                    BtlCloseItemBoard();
                    pick = BtlPickTargetMember(&g_btl_actors[g_btl_actor_turn]);
                    switch (pick) {
                    case BTL_PICK_CANCEL:
                        break;
                    case BTL_PICK_ABORT:
                        g_btl_item_abort = 1;
                        break;
                    default:
                        BtlEnemiesResetGfx();
                        BtlPartyResetGfx();
                        BtlRefreshMarkers();
                        g_btl_actors[g_btl_actor_turn].ail_line = g_btl_item_used;
                        switch (g_btl_item_used) {
                        case 0x1A:
                            g_btl_actors[g_btl_actor_turn].move = 0xF3;
                            break;
                        case 0x1C:
                            g_btl_actors[g_btl_actor_turn].move = 0xF1;
                            break;
                        case 0x1B:
                            g_btl_actors[g_btl_actor_turn].move = 0xED;
                            break;
                        case 0x22:
                            g_btl_actors[g_btl_actor_turn].move = 0xF6;
                            break;
                        }
                        g_btl_step++;
                        break;
                    }
                    break;
                case 0x1D:
                case 0x1E:
                    BtlCloseItemBoard();
                    pick = BtlPickTargetParty(&g_btl_actors[g_btl_actor_turn]);
                    switch (pick) {
                    case BTL_PICK_CANCEL:
                        break;
                    case BTL_PICK_ABORT:
                        g_btl_item_abort = 1;
                        break;
                    default:
                        if (g_btl_item_used == 0x1D) {
                            g_btl_actors[g_btl_actor_turn].move = 0xF4;
                        } else {
                            g_btl_actors[g_btl_actor_turn].move = 0xF5;
                        }
                        BtlEnemiesResetGfx();
                        BtlPartyResetGfx();
                        BtlRefreshMarkers();
                        g_btl_actors[g_btl_actor_turn].ail_line = g_btl_item_used;
                        g_btl_step++;
                        break;
                    }
                    break;
                case 0x1F:
                case 0x20:
                case 0x21:
                    pick = BtlPickTargetEnemies(&g_btl_actors[g_btl_actor_turn]);
                    switch (pick) {
                    case BTL_PICK_CANCEL:
                        break;
                    case BTL_PICK_ABORT:
                        g_btl_item_abort = 1;
                        break;
                    default:
                        BtlCloseItemBoard();
                        BtlEnemiesResetGfx();
                        BtlPartyResetGfx();
                        BtlRefreshMarkers();
                        g_btl_actors[g_btl_actor_turn].ail_line = g_btl_item_used;
                        if (g_btl_item_used == 0x1F) {
                            g_btl_actors[g_btl_actor_turn].move = 0xEC;
                        } else if (g_btl_item_used == 0x20) {
                            g_btl_actors[g_btl_actor_turn].move = 0xE5;
                        } else {
                            g_btl_actors[g_btl_actor_turn].move = 0xE8;
                        }
                        g_btl_step++;
                        break;
                    }
                    break;
                }
                break;
            }
            if ((g_btl_pad1_edge & g_btl_key_cancel) && board->motion == 0) {
                BtlSePlay(1, 2);
                BtlCloseItemBoard();
                BtlRefreshMarkers();
                return 0;
            }
            if (((g_btl_pad1_edge & g_btl_key_abort) && board->motion == 0)
                || g_btl_item_abort != 0) {
                BtlSePlay(1, 2);
                BtlCloseItemBoard();
                BtlRefreshMarkers();
                return BTL_PICK_ABORT;
            }
            break;
        case 2:
            if (BtlObjChainAtMotion(g_btl_marker_obj[g_btl_actor_turn], 0)
                != 0) {
                return 1;
            }
            break;
        }
        for (i = 0, cell = g_btl_menu_cursor; i < BTL_CURSOR_CELLS; i++, cell++) {
            cell->x = g_btl_item_spots[g_btl_item_row].x - BTL_CURSOR_NUDGE;
            cell->y = g_btl_item_spots[g_btl_item_row].y;
        }
        BtlDrawFrame();
    }
}
