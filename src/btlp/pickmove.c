/* Persona 1 (JP) - aiming a member's move.  BTLP only.
 *   0x800A58D4 BtlPickMoveTarget  0x800A6D3C BtlAimMemberMove
 *
 * BtlPickMoveTarget is the pick the cast and item commands hand a move to once
 * it is chosen. Moves from 0x75 to 0x8B and from 0xA3 on - bar 0xDB - are aimed
 * the way SpellData.aim says: at the enemy the cursor picks, or at everything
 * the move's area reaches. Every other move goes by its kind. The kinds that
 * help the party pick a member or the whole party, and the three revivals
 * pick a fallen member and then the cell to raise them on; the rest pick an
 * enemy, the enemy side, or an enemy and whoever stands around it. The pick
 * answers the target mask it leaves on the actor, -1 when the player backed
 * out to the board, and -2 on the third key - which most of the picks also
 * leave in g_btl_pick_abort.
 *
 * BtlAimMemberMove makes the same aim again as the turn comes, without a
 * cursor: a target that is no longer there is replaced by the slowest enemy,
 * and a move left with nothing to reach puts the actor's object on phase 10.
 */
#include <decomp/types.h>
#include <persona/common/spell.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/board.h>
#include <persona/btlp/clut.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/input.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/text.h>

/* The moves aimed by SpellData.aim rather than by their kind: a run from
   0x75, and everything from 0xA3 on but the one that reaches another enemy. */
#define MOVE_AIMED_FIRST 0x75
#define MOVE_AIMED_COUNT 0x17
#define MOVE_AIMED_LATER 0xA3
#define MOVE_OTHER_ENEMY 0xDB

/* SpellData.aim, in its low nibble: at one enemy, or at what the area
   reaches. */
#define AIM_MASK   0xF
#define AIM_ONE    1
#define AIM_REACH  2
#define AIM_SIDE   4
#define AIM_PICKED 8

/* SpellData.target: one fighter, or the whole side. Anything else reaches an
   enemy and those around it. */
#define TARGET_ONE  0
#define TARGET_SIDE 4

/* How fast an enemy the area does not reach walks back to full colour. */
#define TARGET_FADE_NOW 0xFF

/* Where the move's lines go, and the help row the item command is on. */
#define MOVE_LINE_Y   0x8C
#define PICK_ROW_ITEM 2

/* A revival: the frames the grid takes to come up, the frames it takes to go
   again, where the grid's anchor stands for a cell, and the cell the placing
   starts from. */
#define PLACE_SHOW_FRAMES 30
#define PLACE_HIDE_FRAMES 60
#define PLACE_ANCHOR_X(col) (((col) * 16 + 0xE7) << 16)
#define PLACE_ANCHOR_Y(row) (((row) * 8 + 0x77) << 16)
#define PLACE_START       2

/* The phase a member's object is put on when the move has nothing left to
   reach. */
#define AIM_FIZZLED 10

int BtlPickMoveTarget(BtlActor *a, int move)
{
    int i;
    int aim;
    int other;
    int j;
    int pick;

    g_btl_pick_abort = 0;
    BtlDrawFrame();
    if ((move >= MOVE_AIMED_FIRST && move < MOVE_AIMED_FIRST + MOVE_AIMED_COUNT)
        || (move >= MOVE_AIMED_LATER && move != MOVE_OTHER_ENEMY)) {
        aim = g_spell_data[move].aim & AIM_MASK;
        switch (aim) {
        case AIM_ONE:
        case AIM_PICKED:
            BtlMarkMoveArea(a, g_spell_data[move].target, aim);
            g_btl_enemy_slot = BtlSlowestOrder();
            if (g_btl_enemy_slot < 0) {
                BtlOpenMessage(0, 0, g_btl_msg_out_of_reach, PICK_HELP_X,
                               MOVE_LINE_Y);
                while (g_btl_pad1_edge == 0) {
                    BtlDrawFrame();
                }
                if (g_btl_no_help == 0) {
                    return -1;
                }
                BtlCloseMessage(0);
                return -1;
            }
            for (;;) {
                pick = BtlPickEnemy(&g_btl_enemy_slot);
                if (pick >= 0) {
                    break;
                }
                if (pick == -1) {
                    return -1;
                }
                if (pick == -2) {
                    return -2;
                }
                BtlDrawFrame();
            }
            a->order = g_btl_enemy_slot + BTL_PARTY;
            a->targets = 1 << (g_btl_enemy_slot + BTL_PARTY);
            break;
        case AIM_REACH:
        case AIM_SIDE:
            BtlMarkMoveArea(a, g_spell_data[move].target, aim);
            if (BtlSlowestOrder() < 0) {
                BtlOpenMessage(0, 0, g_btl_msg_out_of_reach, PICK_HELP_X,
                               MOVE_LINE_Y);
                while (g_btl_pad1_edge == 0) {
                    BtlDrawFrame();
                }
                if (g_btl_no_help == 0) {
                    return -1;
                }
                BtlCloseMessage(0);
                return -1;
            }
            for (j = 0; j < BTL_ENEMIES; j++) {
                if (g_btl_combatants[j].c.key != 0) {
                    if (g_btl_combatants[j].pickable != 0) {
                        g_btl_combatants[j].obj->motion = BTL_TARGET_MOTION;
                        BtlTintActorClut(j + BTL_PARTY, g_btl_tint_pick_r[0],
                                         g_btl_tint_pick_r[1],
                                         g_btl_tint_pick_r[2]);
                    } else {
                        g_btl_combatants[j].obj->rgb_to[0] = BTL_TARGET_DIM;
                        g_btl_combatants[j].obj->rgb_to[1] = BTL_TARGET_DIM;
                        g_btl_combatants[j].obj->rgb_to[2] = BTL_TARGET_DIM;
                        g_btl_combatants[j].obj->fade = BTL_TARGET_FADE;
                    }
                }
            }
            for (;;) {
                if (g_btl_pad1_edge & g_btl_key_confirm) {
                    a->order = BtlSlowestOrder() + BTL_PARTY;
                    a->targets = BtlPickableMask();
                    break;
                }
                if (g_btl_pad1_edge & g_btl_key_cancel) {
                    BtlEnemiesResetGfx();
                    return -1;
                }
                if (g_btl_pad1_edge & g_btl_key_abort) {
                    BtlEnemiesResetGfx();
                    return -2;
                }
                BtlDrawFrame();
            }
            break;
        }
        goto done;
    }

    switch (g_spell_data[move].kind & SPELL_KIND_MASK) {
    case 0x14:
    case 0x1A:
    case 0x1C:
    case 0x1E:
    case 0x32:
        if ((move >= 0x53 && move <= 0x55) || move == 0x5B || move == 0x6E) {
            goto hostile;
        }
        if (move == 0x6B || move == 0x6C || move == 0x42) {
            for (i = 0; i < BTL_PARTY; i++) {
                if (g_btl_actors[i].c.key != 0
                    && (signed char)g_btl_actors[i].c.status
                           == BTL_STATUS_DOWN) {
                    break;
                }
            }
            if (i >= BTL_PARTY) {
                BtlOpenMessage(0, 0, g_btl_msg_none_down, PICK_HELP_X,
                               MOVE_LINE_Y);
                while (g_btl_pad1_edge == 0) {
                    BtlDrawFrame();
                }
                BtlCloseMessage(0);
                return -1;
            }
            if (g_btl_pick_help_row2 == PICK_ROW_ITEM) {
                BtlCloseItemBoard();
            } else if (g_btl_cast_debug != 0) {
                BtlCloseListBoard();
            } else {
                BtlClosePersonaBoard();
            }
            BtlRefreshMarkers();
            for (;;) {
                if (BtlMarkersIdle() != 0) {
                    break;
                }
                BtlDrawFrame();
            }
            BtlDrawFrame();
            BtlDrawFrame();
            BtlDimEnemies();
            BtlDimParty();
            g_btl_target_slot = BtlDownMemberNext(-1);
        fallen:
            for (;;) {
                pick = BtlPickDownMember(&g_btl_target_slot);
                if (pick >= 0) {
                    break;
                }
                if (pick == -2) {
                    g_btl_pick_abort = 1;
                }
                if (pick == -1 || g_btl_pick_abort != 0) {
                    i = 0;
                    do {
                        g_btl_marker_obj[i]->attr &= ~BTL_MARK_CHOSEN;
                        i++;
                    } while (i < BTL_PARTY);
                    if (g_btl_pick_abort == 0) {
                        BtlRetractMarkers();
                        if (g_btl_pick_help_row2 == PICK_ROW_ITEM) {
                            BtlOpenItemBoard();
                        } else if (g_btl_cast_debug != 0) {
                            BtlOpenSpellBoard();
                        } else {
                            BtlOpenPersonaBoard();
                        }
                    }
                    BtlSingleOutMember(g_btl_actor_turn);
                    BtlEnemiesResetGfx();
                    if (g_btl_pick_abort != 0) {
                        return -2;
                    }
                    return -1;
                }
                i = 0;
                do {
                    g_btl_marker_obj[i]->attr &= ~BTL_MARK_CHOSEN;
                    i++;
                } while (i < BTL_PARTY);
                g_btl_marker_obj[g_btl_target_slot]->attr |= BTL_MARK_CHOSEN;
                BtlDrawFrame();
            }
            BtlSePlay(2, 2);
            BtlSpawnPickGrid();
            BtlPlacePickCursors();
            g_btl_delay = PLACE_SHOW_FRAMES;
            for (;;) {
                BtlDrawFrame();
                if (g_btl_delay == 0) {
                    break;
                }
            }
            while (g_btl_pick_cursors[0]->motion != 0) {
                BtlDrawFrame();
            }
            g_btl_place_member = g_btl_target_slot;
            if (g_btl_actors[g_btl_place_member].revive_mark
                == BTL_REVIVE_STOOD) {
                g_btl_place_col = g_btl_actors[g_btl_place_member].place_col;
                g_btl_place_row = g_btl_actors[g_btl_place_member].place_row;
                g_btl_grid_anchor->x = PLACE_ANCHOR_X(g_btl_place_col);
                g_btl_grid_anchor->y = PLACE_ANCHOR_Y(g_btl_place_row);
                g_btl_grid_anchor->attr &= ~BTL_OBJ_HIDDEN;
                for (;;) {
                    BtlDrawFrame();
                    if (g_btl_pad1_edge & g_btl_key_confirm) {
                        goto placed;
                    }
                    if (g_btl_pad1_edge & g_btl_key_cancel) {
                        BtlDespawnPickGrid();
                        for (i = 0; i < PLACE_HIDE_FRAMES; i++) {
                            BtlDrawFrame();
                        }
                        BtlSePlay(2, 3);
                        BtlDrawFrame();
                        goto fallen;
                    }
                    if (g_btl_pad1_edge & g_btl_key_abort) {
                        BtlDespawnPickGrid();
                        for (i = 0; i < PLACE_HIDE_FRAMES; i++) {
                            BtlDrawFrame();
                        }
                        BtlSePlay(2, 3);
                        g_btl_pick_abort = 1;
                        BtlDrawFrame();
                        goto fallen;
                    }
                }
            }
            g_btl_place_row = PLACE_START;
            g_btl_place_col = PLACE_START;
            for (;;) {
                pick = BtlPlaceMoveStep(&g_btl_actors[g_btl_target_slot]);
                if (pick >= 0) {
                    goto placed;
                }
                switch (pick) {
                case BTL_PICK_WAIT:
                    break;
                case -2:
                case -1:
                    BtlDespawnPickGrid();
                    for (i = 0; i < PLACE_HIDE_FRAMES; i++) {
                        BtlDrawFrame();
                    }
                    BtlSePlay(2, 3);
                    if (pick == -2) {
                        g_btl_pick_abort = 1;
                    }
                    BtlDrawFrame();
                    goto fallen;
                }
                BtlDrawFrame();
            }
        placed:
            BtlSePlay(2, 3);
            BtlDespawnPickGrid();
            g_btl_marker_obj[g_btl_target_slot]->attr &= ~BTL_MARK_CHOSEN;
            BtlEnemiesResetGfx();
            a->order = g_btl_target_slot;
            a->targets = 1 << g_btl_target_slot;
            a->place_col = g_btl_actors[g_btl_target_slot].obj->col2 >> 1;
            a->place_row = g_btl_actors[g_btl_target_slot].obj->row;
            g_btl_actors[g_btl_target_slot].revive_mark = BTL_REVIVE_STOOD;
            a->revive_slot = g_btl_target_slot;
            a->revive_mark = BTL_REVIVE_CARRIED;
            break;
        }
        if (g_btl_pick_help_row2 == PICK_ROW_ITEM) {
            BtlCloseItemBoard();
        } else if (g_btl_cast_debug != 0) {
            BtlCloseListBoard();
        } else {
            BtlClosePersonaBoard();
        }
        BtlRefreshMarkers();
        switch (g_spell_data[move].target) {
        case TARGET_ONE:
            BtlDimEnemies();
            BtlSetPartyPickable();
            g_btl_target_slot = g_btl_actor_turn;
            if ((signed char)g_btl_actors[g_btl_target_slot].c.status
                == BTL_STATUS_NOINPUT) {
                g_btl_target_slot = BtlCursorNext(-1);
            }
            for (;;) {
                pick = BtlPickMember(&g_btl_target_slot);
                if (pick >= 0) {
                    break;
                }
                if (pick != -2) {
                    if (pick < -1) {
                        goto sweep;
                    }
                    if (pick != -1) {
                        goto sweep;
                    }
                    BtlSingleOutMember(g_btl_actor_turn);
                    BtlRetractMarkers();
                    if (g_btl_pick_help_row2 == PICK_ROW_ITEM) {
                        BtlOpenItemBoard();
                    } else if (g_btl_cast_debug != 0) {
                        BtlOpenSpellBoard();
                    } else {
                        BtlOpenPersonaBoard();
                    }
                }
                BtlEnemiesResetGfx();
                if (pick == -2) {
                    g_btl_pick_abort = 1;
                    return -2;
                }
                return -1;
            sweep:
                i = 0;
                do {
                    g_btl_marker_obj[i]->attr &= ~BTL_MARK_CHOSEN;
                    i++;
                } while (i < BTL_PARTY);
                g_btl_marker_obj[g_btl_target_slot]->attr |= BTL_MARK_CHOSEN;
                BtlDrawFrame();
            }
            BtlEnemiesResetGfx();
            a->order = g_btl_target_slot;
            a->targets = 1 << g_btl_target_slot;
            break;
        case TARGET_SIDE:
            BtlDimEnemies();
            BtlSetPartyPickable();
            BtlPartyResetGfx();
            for (j = 0; j < BTL_PARTY; j++) {
                g_btl_marker_obj[j]->attr &= ~(BTL_MARK_CHOSEN | BTL_OBJ_PICKED);
                g_btl_marker_obj[j]->rgb[0] = BTL_TARGET_LIT;
                g_btl_marker_obj[j]->rgb[1] = BTL_TARGET_LIT;
                g_btl_marker_obj[j]->rgb[2] = BTL_TARGET_LIT;
                if (g_btl_actors[j].pickable != 0
                    && (signed char)g_btl_actors[j].c.status
                           != BTL_STATUS_NOINPUT) {
                    g_btl_actors[j].obj->attr &= ~BTL_OBJ_PICKED;
                    g_btl_actors[j].obj->rgb[0] = BTL_TARGET_LIT;
                    g_btl_actors[j].obj->rgb[1] = BTL_TARGET_LIT;
                    g_btl_actors[j].obj->rgb[2] = BTL_TARGET_LIT;
                    g_btl_actors[j].obj->motion = BTL_TARGET_MOTION;
                    BtlTintActorClut(j, g_btl_tint_pick_r[0],
                                     g_btl_tint_pick_r[1],
                                     g_btl_tint_pick_r[2]);
                    g_btl_marker_obj[j]->attr |= BTL_MARK_CHOSEN;
                }
            }
            for (;;) {
                if (g_btl_pad1_edge & g_btl_key_confirm) {
                    a->order = g_btl_actor_turn;
                    a->targets = BtlPartyPickableMask();
                    BtlEnemiesResetGfx();
                    break;
                }
                if (g_btl_pad1_edge & g_btl_key_cancel) {
                    BtlSingleOutMember(g_btl_actor_turn);
                    BtlRetractMarkers();
                    if (g_btl_pick_help_row2 == PICK_ROW_ITEM) {
                        BtlOpenItemBoard();
                    } else if (g_btl_cast_debug != 0) {
                        BtlOpenSpellBoard();
                    } else {
                        BtlOpenPersonaBoard();
                    }
                    BtlEnemiesResetGfx();
                    return -1;
                }
                if (g_btl_pad1_edge & g_btl_key_abort) {
                    BtlSingleOutMember(g_btl_actor_turn);
                    BtlEnemiesResetGfx();
                    g_btl_pick_abort = 1;
                    return -2;
                }
                BtlDrawFrame();
            }
            break;
        }
        break;
    default:
    hostile:
        switch (g_spell_data[move].target) {
        case TARGET_ONE:
            BtlSetPickable();
            g_btl_enemy_slot = BtlSlowestOrder();
            for (;;) {
                pick = BtlPickEnemy(&g_btl_enemy_slot);
                if (pick >= 0) {
                    break;
                }
                switch (pick) {
                case BTL_PICK_WAIT:
                    break;
                case -1:
                    return -1;
                case -2:
                    g_btl_pick_abort = 1;
                    return -2;
                }
                BtlDrawFrame();
            }
            a->order = g_btl_enemy_slot + BTL_PARTY;
            a->targets = 1 << (g_btl_enemy_slot + BTL_PARTY);
            break;
        case TARGET_SIDE:
            BtlSetPickable();
            j = 0;
            do {
                if (g_btl_combatants[j].pickable != 0) {
                    g_btl_combatants[j].obj->motion = BTL_TARGET_MOTION;
                    BtlTintActorClut(j + BTL_PARTY, g_btl_tint_pick_r[0],
                                     g_btl_tint_pick_r[1],
                                     g_btl_tint_pick_r[2]);
                }
                j++;
            } while (j < BTL_ENEMIES);
            for (;;) {
                if (g_btl_pad1_edge & g_btl_key_confirm) {
                    if (move == MOVE_OTHER_ENEMY) {
                        other = BtlPickOtherEnemy(0);
                        if (other < 0) {
                            other = BtlSlowestOrder() + BTL_PARTY;
                        }
                        a->order = other;
                        a->targets = 1 << a->order;
                    } else {
                        a->order = BtlSlowestOrder() + BTL_PARTY;
                        a->targets = BtlPickableMask();
                    }
                    break;
                }
                if (g_btl_pad1_edge & g_btl_key_cancel) {
                    BtlEnemiesResetGfx();
                    return -1;
                }
                if (g_btl_pad1_edge & g_btl_key_abort) {
                    BtlEnemiesResetGfx();
                    g_btl_pick_abort = 1;
                    return -2;
                }
                BtlDrawFrame();
            }
            break;
        default:
            BtlSetPickable();
            g_btl_enemy_slot = BtlSlowestOrder();
            for (;;) {
                pick = BtlPickEnemyLit(&g_btl_enemy_slot);
                if (pick >= 0) {
                    break;
                }
                if (pick == -1) {
                    return -1;
                }
                if (pick == -2) {
                    g_btl_pick_abort = 1;
                    return -2;
                }
                BtlMarkEnemiesAround(&g_btl_combatants[g_btl_enemy_slot],
                                     g_spell_data[move].target);
                for (j = 0; j < BTL_ENEMIES; j++) {
                    if (g_btl_combatants[j].c.key != 0) {
                        if (g_btl_combatants[j].pickable != 0) {
                            if (g_btl_enemy_slot != j) {
                                g_btl_combatants[j].obj->motion = BTL_TARGET_MOTION;
                            } else {
                                g_btl_combatants[j].obj->motion =
                                    BTL_TARGET_MOTION_CURSOR;
                            }
                            BtlTintActorClut(j + BTL_PARTY,
                                             g_btl_tint_pick_r[0],
                                             g_btl_tint_pick_g[0],
                                             g_btl_tint_pick_b[0]);
                        } else {
                            *(BtlClutBlock *)(g_btl_enemy_clut
                                              + j * BTL_CLUT_ENTRIES) =
                                *(BtlClutBlock *)(g_btl_enemy_clut_base
                                                  + j * BTL_CLUT_ENTRIES);
                            g_btl_combatants[j].obj->motion = 0;
                            g_btl_combatants[j].obj->rgb_to[0] = BTL_TARGET_LIT;
                            g_btl_combatants[j].obj->rgb_to[1] = BTL_TARGET_LIT;
                            g_btl_combatants[j].obj->rgb_to[2] = BTL_TARGET_LIT;
                            g_btl_combatants[j].obj->fade = TARGET_FADE_NOW;
                            g_btl_combatants[j].obj->motion = 0;
                        }
                    }
                }
                BtlSetPickable();
                BtlDrawFrame();
            }
            BtlSetPickable();
            BtlMarkEnemiesAround(&g_btl_combatants[g_btl_enemy_slot],
                                 g_spell_data[move].target);
            a->order = g_btl_enemy_slot + BTL_PARTY;
            a->targets = BtlPickableMask();
            break;
        }
        break;
    }
done:
    if (g_btl_pick_help_row2 == PICK_ROW_ITEM) {
        BtlCloseItemBoard();
    } else if (g_btl_cast_debug != 0) {
        BtlCloseListBoard();
    } else {
        BtlClosePersonaBoard();
    }
    return a->targets;
}

void BtlAimMemberMove(BtlActor *a, int move)
{
    int aim;
    int slot;

    if ((move >= MOVE_AIMED_FIRST && move < MOVE_AIMED_FIRST + MOVE_AIMED_COUNT)
        || (move >= MOVE_AIMED_LATER && move != MOVE_OTHER_ENEMY)) {
        aim = g_spell_data[move].aim & AIM_MASK;
        switch (aim) {
        case AIM_ONE:
        case AIM_PICKED:
            if (g_btl_actors[g_btl_hit_slot].c.key != 0) {
                return;
            }
            if (BtlMarkMoveArea(a, g_spell_data[move].target, aim) < 0) {
                a->obj->phase = AIM_FIZZLED;
                return;
            }
            g_btl_enemy_slot = BtlSlowestOrder();
            a->order = g_btl_enemy_slot + BTL_PARTY;
            a->targets = 1 << (g_btl_enemy_slot + BTL_PARTY);
            return;
        case AIM_REACH:
        case AIM_SIDE:
            if (g_btl_actors[g_btl_hit_slot].c.key != 0) {
                return;
            }
            if (BtlMarkMoveArea(a, g_spell_data[move].target, aim) < 0) {
                a->obj->phase = AIM_FIZZLED;
                return;
            }
            a->order = BtlSlowestOrder() + BTL_PARTY;
            a->targets = BtlPickableMask();
            return;
        }
        return;
    }

    switch (g_spell_data[move].kind & SPELL_KIND_MASK) {
    case 0x14:
    case 0x1A:
    case 0x1C:
    case 0x1E:
    case 0x32:
        if ((move >= 0x53 && move <= 0x55) || move == 0x5B || move == 0x6E) {
            goto hostile;
        }
        if (move == 0x6B || move == 0x6C || move == 0x42) {
            if (g_btl_actors[g_btl_hit_slot].c.key == 0
                || (signed char)g_btl_actors[g_btl_hit_slot].c.status
                       == BTL_STATUS_DOWN
                || (g_btl_actors[g_btl_hit_slot].flags & BTL_ACTOR_OUT)) {
                return;
            }
            a->obj->phase = AIM_FIZZLED;
            return;
        }
        switch (g_spell_data[move].target) {
        case TARGET_ONE:
            if (g_btl_actors[g_btl_hit_slot].c.key != 0
                && (signed char)g_btl_actors[g_btl_hit_slot].c.status
                       != BTL_STATUS_DOWN
                && !(g_btl_actors[g_btl_hit_slot].flags & BTL_ACTOR_OUT)) {
                return;
            }
            a->obj->phase = AIM_FIZZLED;
            return;
        case TARGET_SIDE:
            BtlSetPartyPickable();
            a->targets = BtlPartyPickableMask();
            return;
        }
        return;
    default:
    hostile:
        switch (g_spell_data[move].target) {
        case TARGET_ONE:
            if (g_btl_actors[g_btl_hit_slot].c.key != 0
                && (signed char)g_btl_actors[g_btl_hit_slot].c.status
                       != BTL_STATUS_DOWN
                && !(g_btl_actors[g_btl_hit_slot].flags & BTL_ACTOR_OUT)
                && g_btl_hit_slot >= BTL_PARTY) {
                return;
            }
            BtlSetPickable();
            g_btl_enemy_slot = BtlSlowestOrder();
            a->order = g_btl_enemy_slot + BTL_PARTY;
            a->targets = 1 << (g_btl_enemy_slot + BTL_PARTY);
            return;
        case TARGET_SIDE:
            if (g_btl_actors[g_btl_hit_slot].c.key != 0
                && (signed char)g_btl_actors[g_btl_hit_slot].c.status
                       != BTL_STATUS_DOWN
                && !(g_btl_actors[g_btl_hit_slot].flags & BTL_ACTOR_OUT)
                && g_btl_hit_slot >= BTL_PARTY) {
                return;
            }
            if (move == MOVE_OTHER_ENEMY) {
                slot = BtlPickOtherEnemy(0);
                if (slot < 0) {
                    a->order = BtlSlowestOrder() + BTL_PARTY;
                    a->targets = 1 << a->order;
                } else {
                    a->order = slot;
                    a->targets = 1 << slot;
                }
            } else {
                BtlSetPickable();
                a->order = BtlSlowestOrder() + BTL_PARTY;
                a->targets = BtlPickableMask();
            }
            return;
        default:
            if (g_btl_actors[g_btl_hit_slot].c.key != 0
                && (signed char)g_btl_actors[g_btl_hit_slot].c.status
                       != BTL_STATUS_DOWN
                && !(g_btl_actors[g_btl_hit_slot].flags & BTL_ACTOR_OUT)
                && g_btl_hit_slot >= BTL_PARTY) {
                return;
            }
            BtlSetPickable();
            g_btl_enemy_slot = BtlSlowestOrder();
            BtlMarkEnemiesAround(&g_btl_combatants[g_btl_enemy_slot],
                                 g_spell_data[move].target);
            a->order = g_btl_enemy_slot + BTL_PARTY;
            a->targets = BtlPickableMask();
            return;
        }
    }
}
