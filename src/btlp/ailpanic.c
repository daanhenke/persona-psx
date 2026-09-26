/* Persona 1 (JP) - what panic does to a fighter's turn.  BTLP only.
 *   0x80095318 BtlAilmentTurnPanic
 *
 * Entry 2 of g_btl_ailment_turn. A panicking fighter does something at random,
 * and the deeper the panic the fewer of the sensible choices are in the draw.
 *
 * A party member rolls one of seven - out of the first six at the shallowest
 * level, all seven one step down, and only the last five at the deepest:
 *
 *   0  shoot the slowest enemy in reach, if it carries a gun with rounds in it,
 *      and otherwise swing as for 1;
 *   1  swing at the slowest enemy in reach, or lose the turn if none is;
 *   2  turn the gun on another member, if it has one loaded, and otherwise
 *      swing at them as for 3;
 *   3  swing at another member, or lose the turn if there is none;
 *   4  find nothing to do;
 *   5  lose the turn;
 *   6  run: a random free cell of the formation is picked, one the four-cell
 *      rule allows, and the member glides there over PANIC_FRAMES frames.
 *
 * An enemy rolls one of four, and past the shallowest level never the first:
 * cast at a target of its own choosing, turn on another enemy, find nothing
 * to do, or lose the turn.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/status.h>
#include <persona/common/item.h>
#include <persona/common/spell.h>

/* The equipment slots of the gun and the rounds in it. */
#define PANIC_GUN  1
#define PANIC_AMMO 2

/* How long the run takes, and what the turn is answered with once it is
   under way. */
#define PANIC_FRAMES 20

/* 98.16%. Read off the image's registers:
     - one variable, n, is the roll, the slot a pick answers and the order
       worked out from the slowest enemy, as the image keeps all three in a2;
     - the enemy side's 1 << slot shifts a variable set to 1 before its roll,
       which the image keeps in s0 across rand;
     - the move's success path is inside its loop and leaves by `return`, so
       loop.c lifts and merges its constants with the loop's own: the divide
       by twenty after the pick reuses the divide by five's magic (s3), and one
       0xFF (fp) serves the free-cell test and the old cell's clearing.
   What is left: the image loads g_btl_formation's address in the middle of
   the second roll's division, where gcc here loads it after the index; and
   the enemy switch's `n == 1` compares against that s0 in the image, where
   gcc here puts 1 in v0 (cse cannot see s0's 1 past the join of the two
   rolls). */
#ifdef NON_MATCHING
void BtlAilmentTurnPanic(BtlActor *a, u_char *act)
{
    u_char *cell;
    int     empty;
    int     n;
    int     from;
    int     col;
    int     row;

    if ((a->obj->attr & BTL_OBJ_OTHER_SIDE) == 0) {
        switch ((signed char)a->c.ail_level) {
        case 0:
            n = rand() % 6;
            break;
        case 1:
            n = rand() % 7;
            break;
        case 2:
            n = rand() % 5 + 2;
            break;
        }
        switch (n) {
        case 0:
            if (a->c.equip[PANIC_GUN] != 0 && a->c.equip[PANIC_AMMO] != 0
                && BtlMarkMoveArea(a, g_item_defs[a->c.equip[PANIC_GUN]].area,
                                   g_item_defs[a->c.equip[PANIC_GUN]].swing)
                       >= 0) {
                n = BtlSlowestOrder() + BTL_PARTY;
                a->order = n;
                a->targets = BtlPickableMask();
                *act = AIL_ACT_GUN;
                return;
            }
            /* fall through */
        case 1:
            if (BtlMarkMoveArea(a, g_item_defs[a->c.equip[0]].area,
                                g_item_defs[a->c.equip[0]].swing) < 0) {
                *act = AIL_ACT_LOST;
                return;
            }
            n = BtlSlowestOrder() + BTL_PARTY;
            a->order = n;
            a->targets = BtlPickableMask();
            *act = AIL_ACT_AIMED;
            return;
        case 2:
            if (a->c.equip[PANIC_GUN] != 0 && a->c.equip[PANIC_AMMO] != 0) {
                n = BtlPickOtherMember(a->obj->mark_num);
                if (n >= 0) {
                    a->targets = 1 << n;
                    a->order = n;
                    *act = AIL_ACT_GUN;
                    return;
                }
            }
            /* fall through */
        case 3:
            n = BtlPickOtherMember(a->obj->mark_num);
            if (n < 0) {
                *act = AIL_ACT_LOST;
                return;
            }
            a->targets = 1 << n;
            a->order = n;
            *act = AIL_ACT_AIMED;
            return;
        case 6:
            for (from = 0; from < GRID_CELLS; from++) {
                if (g_btl_formation[from] == a->obj->mark_num) {
                    break;
                }
            }
            for (;;) {
                col = rand() % GRID_W;
                row = rand() % GRID_H;
                cell = &g_btl_formation[row * GRID_W + col];
                if (*cell == CELL_EMPTY && BtlFormationCellFree(col, row) != 0) {
                    a->obj->steps = PANIC_FRAMES;
                    a->obj->step_x = ((col * PLACE_COL_W + PLACE_COL_ORG) * PLACE_FIXED
                                      - a->obj->x) / PANIC_FRAMES;
                    a->obj->step_y = ((row * PLACE_ROW_H + PLACE_ROW_ORG) * PLACE_FIXED
                                      - a->obj->y) / PANIC_FRAMES;
                    a->obj->col2 = col * 2;
                    a->obj->row = row;
                    g_btl_formation[from] = CELL_EMPTY;
                    *cell = a->obj->mark_num;
                    *act = AIL_ACT_MOVED;
                    return;
                }
            }
        case 4:
            *act = AIL_ACT_NONE;
            return;
        case 5:
            *act = AIL_ACT_LOST;
            return;
        }
    } else {
        row = 1;
        if ((signed char)a->c.ail_level < 2) {
            n = rand() % 4;
        } else {
            n = rand() % 3 + 1;
        }
        switch (n) {
        case 0:
            if (BtlPickAiTarget(a, g_spell_data[0].target) >= 0) {
                a->move = 0;
                BtlAimMove(a);
                *act = AIL_ACT_AIMED;
            }
            return;
        case 1:
            n = BtlPickOtherEnemy(a->obj->mark_num);
            if (n < 0) {
                *act = AIL_ACT_LOST;
                return;
            }
            a->targets = row << n;
            a->move = 0;
            a->order = n;
            *act = AIL_ACT_AIMED;
            return;
        case 2:
            *act = AIL_ACT_NONE;
            return;
        case 3:
            *act = AIL_ACT_LOST;
            return;
        }
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/ailpanic", BtlAilmentTurnPanic);
#endif
