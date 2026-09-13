/* Persona 1 (JP) - the debug page's fighter editor.  BTLP only.
 *   0x8009F9B8 BtlDebugEditMember
 *
 * Row 8 of g_btl_debug_actions: pick a party member and rewrite them. It has
 * three pages, taken in turn on g_btl_step.
 *
 * The pick doubles as the editor for who the member is. With start held, up
 * and down step the member's Char key round the eight test characters,
 * passing over any key another member already has, and either one writes that
 * character's name into the record and has the markers built again. A member
 * chosen with L2 held goes to the equipment page, anyone else to the stats
 * page.
 *
 * On the stats page square and circle step the level, and the end key puts it
 * at 99 - and, for the hero, every base stat at 99 with it. Any of the three
 * rebuilds the member from the level as a level-up would have: hp_max from
 * the character's row of g_char_hp_growth, and each base stat from its column
 * of g_char_stat_growth, clamped to 999 and 99. The hero has no row in either
 * table and is left as set. The directions walk a cursor over the five base
 * stats and nudge the one under it, and the shoulder buttons nudge unk3A and
 * unk3C. Every frame the Persona is applied again and the stats recalculated,
 * so the board shows the member as they would fight.
 *
 * On the equipment page up and down pick a slot and left and right step what
 * is in it; the end key takes every slot to the last thing the member may
 * hold. Cancel leaves either page for the pick again.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/common/char.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/board.h>
#include <persona/btlp/debug.h>
#include <persona/btlp/input.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/stage.h>

#define PAD_L2     0x1
#define PAD_R2     0x2
#define PAD_L1     0x4
#define PAD_R1     0x8
#define PAD_CIRCLE 0x20
#define PAD_SQUARE 0x80
#define PAD_START  0x800
#define PAD_UP     0x1000
#define PAD_DOWN   0x4000
#define PAD_DIRS   0xF000

/* The three pages. */
#define EDIT_PICK  0
#define EDIT_STATS 1
#define EDIT_EQUIP 2

/* The hero's key, and the test characters' keys either side of it. */
#define EDIT_HERO      1
#define EDIT_KEY_FIRST 2
#define EDIT_KEY_END   10

/* What a member is rebuilt from, and the shape of the growth tables: a row of
   fifty per character in each, and five of them per character in the stats'
   table. */
#define EDIT_HP_START   24
#define EDIT_STAT_START 6
#define EDIT_HP_ROW     50
#define EDIT_STAT_ROW   250
#define EDIT_STAT_COL   50

#define EDIT_LEVEL_MAX 99
#define EDIT_STAT_MAX  99
#define EDIT_NUM_MAX   999

#define EDIT_CLAMP(v, max) ((v) != 0 ? ((v) < (max) + 1 ? (v) : (max)) : 1)

/* The cursor: how many cells it is drawn with, how far left of a stat's spot
   it stands, and where it stands on the equipment page. */
#define EDIT_CURSOR_CELLS 14
#define EDIT_CURSOR_NUDGE 7
#define EDIT_EQUIP_X      (-0x3F)
#define EDIT_EQUIP_Y      (-0x28)
#define EDIT_EQUIP_PITCH  12

#define EDIT_EQUIP_SLOTS 7
#define EDIT_STAT_ROWS   5

/* The stat cursor walks five rows, each naming the row above it and the
   row below, and stands at a spot per row. */
#define NAV_UP   0
#define NAV_DOWN 1

u_char g_btl_edit_stat_nav[EDIT_STAT_ROWS][2] = {
    { 4, 1 }, { 0, 2 }, { 1, 3 }, { 2, 4 }, { 3, 0 },
};

BtlMenuSpot g_btl_edit_stat_spots[EDIT_STAT_ROWS] = {
    { -0x78, 0x04 }, { -0x78, 0x10 }, { -0x78, 0x1C },
    { -0x78, 0x28 }, { -0x78, 0x34 },
};

/* Which slot and which stat the two pages' cursors are on. */
extern short g_btl_edit_equip_row;
extern short g_btl_edit_stat_row;

int BtlDebugEditMember(void)
{
    BtlActor *a;
    u_char   *stat;
    BtlObj   *board;
    u_char   *row;
    int       keys;
    int       i;
    int       j;
    int       free;
    int       key;
    BtlGfxCell *cell;

    BtlDrawFrame();
    BtlRefreshMarkers();
    BtlSetPartyPickable();
    g_btl_target_slot = 0;
    for (;;) {
        switch (g_btl_step) {
        case EDIT_PICK:
            keys = BtlPickMember(&g_btl_target_slot);
            a = &g_btl_actors[g_btl_target_slot];
            if (a->c.key >= EDIT_KEY_FIRST) {
                if ((g_btl_pad1 & PAD_START) && (g_btl_pad1_edge & PAD_UP)) {
                    do {
                        a->c.key++;
                        free = 1;
                        if (a->c.key >= EDIT_KEY_END) {
                            a->c.key = EDIT_KEY_FIRST;
                        }
                        for (i = 0; i < BTL_PARTY; i++) {
                            if (i != g_btl_target_slot
                                && g_btl_actors[i].c.key == a->c.key) {
                                free = 0;
                                break;
                            }
                        }
                    } while (!free);
                }
                if ((g_btl_pad1 & PAD_START) && (g_btl_pad1_edge & PAD_DOWN)) {
                    do {
                        a->c.key--;
                        free = 1;
                        if (a->c.key < EDIT_KEY_FIRST) {
                            a->c.key = EDIT_KEY_END - 1;
                        }
                        for (i = 0; i < BTL_PARTY; i++) {
                            if (i != g_btl_target_slot
                                && g_btl_actors[i].c.key == a->c.key) {
                                free = 0;
                                break;
                            }
                        }
                    } while (!free);
                }
                if ((g_btl_pad1 & PAD_START)
                    && (g_btl_pad1_edge & (PAD_UP | PAD_DOWN))) {
                    g_btl_leave_round = 1;
                    memcpy(a->c.name, g_btl_test_party_names[a->c.key],
                           sizeof(a->c.name));
                    BtlBuildMarkers();
                }
            }
            if (keys == BTL_PICK_WAIT) {
                break;
            }
            if (keys == -1) {
                BtlRetractMarkers();
                BtlPartyResetGfx();
                return 0;
            }
            if (g_btl_leave_round != 0) {
                break;
            }
            BtlPartyResetGfx();
            stat = g_btl_actors[g_btl_target_slot].c.stat_base;
            a = &g_btl_actors[g_btl_target_slot];
            BtlRetractMarkers();
            if (g_btl_pad1 & PAD_L2) {
                BtlEditEquipNames(a);
                BtlOpenBoard1D();
                BtlFillEditBoard(a);
                board = BtlObjLast(BtlOpenEditBoard());
                g_btl_edit_equip_row = 0;
                g_btl_step = EDIT_EQUIP;
            } else {
                BtlFillEditBoard(a);
                BtlOpenEditBoard();
                g_btl_step++;
            }
            break;

        case EDIT_STATS:
            keys = BtlMenuKey();
            if ((keys & (PAD_SQUARE | PAD_CIRCLE))
                || (g_btl_pad1_edge & g_btl_key_end)) {
                if (g_btl_pad1_edge & g_btl_key_end) {
                    a->c.level = EDIT_LEVEL_MAX;
                    if (a->c.key == EDIT_HERO) {
                        for (i = 0; i < CHAR_STATS; i++) {
                            stat[i] = EDIT_STAT_MAX;
                        }
                    }
                }
                if (keys & PAD_SQUARE) {
                    a->c.level--;
                }
                if (keys & PAD_CIRCLE) {
                    a->c.level++;
                }
                a->c.unk56 = a->c.level = EDIT_CLAMP(a->c.level, EDIT_LEVEL_MAX);
                if (a->c.key >= EDIT_KEY_FIRST) {
                    row = g_char_hp_growth
                        + (a->c.key - EDIT_KEY_FIRST) * EDIT_HP_ROW;
                    a->c.hp_max = EDIT_HP_START;
                    if (a->c.level != 1) {
                        for (i = 0; i < a->c.level - 1; i++) {
                            a->c.hp_max += row[i / 2];
                        }
                        a->c.hp_max = a->c.hp_max > 0
                                          ? (a->c.hp_max < EDIT_NUM_MAX + 1
                                                 ? a->c.hp_max
                                                 : EDIT_NUM_MAX)
                                          : 1;
                    }
                    for (i = 0; i < CHAR_STATS; i++) {
                        key = a->c.key;
                        stat[i] = EDIT_STAT_START;
                        row = g_char_stat_growth + key * EDIT_STAT_ROW
                            + i * EDIT_STAT_COL;
                        if (a->c.level != 1) {
                            for (j = 0; j < a->c.level - 1; j++) {
                                stat[i] += row[j / 2];
                            }
                        }
                        stat[i] = EDIT_CLAMP(stat[i], EDIT_STAT_MAX);
                    }
                }
                a->c.hp = a->c.hp_max;
            }
            if (keys & PAD_DIRS) {
                BtlSePlay(1, 0);
            }
            if (keys & PAD_UP) {
                g_btl_edit_stat_row = g_btl_edit_stat_nav[g_btl_edit_stat_row][NAV_UP];
            }
            if (keys & PAD_DOWN) {
                g_btl_edit_stat_row = g_btl_edit_stat_nav[g_btl_edit_stat_row][NAV_DOWN];
            }
            if (keys & PAD_LEFT) {
                stat[g_btl_edit_stat_row]--;
            }
            if (keys & PAD_RIGHT) {
                stat[g_btl_edit_stat_row]++;
            }
            stat[g_btl_edit_stat_row] =
                EDIT_CLAMP(stat[g_btl_edit_stat_row], EDIT_STAT_MAX);
            BtlApplyPersona(a);
            BtlRecalcStats(a);
            BtlFillEditBoard(a);
            for (i = 0, cell = g_btl_menu_cursor; i < EDIT_CURSOR_CELLS; i++, cell++) {
                cell->x = g_btl_edit_stat_spots[g_btl_edit_stat_row].x - EDIT_CURSOR_NUDGE;
                cell->y = g_btl_edit_stat_spots[g_btl_edit_stat_row].y;
            }
            if (g_btl_pad1_edge & g_btl_key_cancel) {
                BtlRefreshMarkers();
                BtlShutEditBoard();
                g_btl_step--;
            }
            if (keys & PAD_L1) {
                a->c.unk3A--;
            }
            if (keys & PAD_R1) {
                a->c.unk3A++;
            }
            if (keys & PAD_L2) {
                a->c.unk3C--;
            }
            if (keys & PAD_R2) {
                a->c.unk3C++;
            }
            a->c.unk3A = EDIT_CLAMP(a->c.unk3A, EDIT_NUM_MAX);
            a->c.unk3C = EDIT_CLAMP(a->c.unk3C, EDIT_NUM_MAX);
            break;

        case EDIT_EQUIP:
            keys = BtlMenuKey();
            if (keys & PAD_DIRS) {
                BtlSePlay(1, 0);
            }
            if (keys & PAD_UP) {
                g_btl_edit_equip_row--;
            }
            if (keys & PAD_DOWN) {
                g_btl_edit_equip_row++;
            }
            g_btl_edit_equip_row =
                g_btl_edit_equip_row < 0
                    ? 0
                    : (g_btl_edit_equip_row < EDIT_EQUIP_SLOTS
                           ? g_btl_edit_equip_row
                           : EDIT_EQUIP_SLOTS - 1);
            if (keys & PAD_LEFT) {
                BtlDebugStepEquip(&a->c, g_btl_edit_equip_row, -1);
            }
            if (keys & PAD_RIGHT) {
                BtlDebugStepEquip(&a->c, g_btl_edit_equip_row, 1);
            }
            if (g_btl_pad1_edge & g_btl_key_end) {
                for (i = 0; i < EDIT_EQUIP_SLOTS; i++) {
                    BtlDebugStepEquip(&a->c, i, 0);
                }
            }
            BtlEditEquipNames(a);
            BtlApplyPersona(a);
            BtlRecalcStats(a);
            BtlFillEditBoard(a);
            if (g_btl_pad1_edge & g_btl_key_cancel) {
                BtlRefreshMarkers();
                BtlCloseBoard1D();
                BtlShutEditBoard();
                g_btl_step = EDIT_PICK;
            }
            for (i = 0, cell = g_btl_menu_cursor; i < EDIT_CURSOR_CELLS; i++, cell++) {
                cell->x = EDIT_EQUIP_X;
                cell->y = g_btl_edit_equip_row * EDIT_EQUIP_PITCH + EDIT_EQUIP_Y;
            }
            board->attr |= BTL_OBJ_HIDDEN;
            break;
        }
        BtlDrawFrame();
    }
}
