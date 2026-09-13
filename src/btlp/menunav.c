/* Persona 1 (JP) - two menus whose cursor walks a table of neighbours.
 * BTLP only.
 *   0x800A4EE8 BtlDebugUpdate       0x800A507C BtlPresetMenuUpdate
 *   0x800A5290 BtlOrdersMenuUpdate  0x800A5428 BtlTacticsMenuUpdate
 *   0x800A5620 BtlSpellMenuUpdate
 *
 * Neither menu moves its cursor by arithmetic. Each item has a row of four in
 * a neighbour table - the item up, down, left and right of it - and a spot the
 * cursor is drawn at. Every frame the pad's directions step the row through
 * the table, a tick is played for any of them, and all fourteen cells of the
 * cursor's artwork are put at the item's spot, a little to its left.
 *
 * BtlDebugUpdate runs the debug board's fifteen items, three across and five
 * down, and answers the item on a confirm, -1 on a cancel and BTL_PICK_WAIT
 * otherwise - the contract BtlPickUpdate keeps.
 *
 * BtlPresetMenuUpdate runs the placement menu's picker over the eight stored
 * layouts and the live grid, three by three, and shows whichever layout the
 * cursor is on as it goes: on the menu's own cursors when `on_field` is clear,
 * and on the fighters themselves when it is set. While R1 is held the live grid
 * is stood instead, so the player can compare. A confirm on the live grid does
 * nothing; cancel answers -1 and the abort key -2.
 *
 * BtlOrdersMenuUpdate runs the orders menu's three items, left and right only,
 * and keeps the item's help line up while help is on - it is taken down again
 * when help is off, and on a confirm. Either way out answers -1.
 *
 * BtlTacticsMenuUpdate walks the five members up and down and cycles the one
 * the cursor is on through the three tactics with left and right, redrawing
 * the page every frame. It has nothing to confirm: a cancel answers 0 and the
 * abort key -2. Its two kinds of step tick differently.
 *
 * BtlSpellMenuUpdate runs the debug board's list of every spell, ten to a
 * page in two columns. Its neighbour table has one sideways column for both
 * directions, and the rows off the top and bottom of the page are numbered
 * past it: stepping onto one puts the row back and, if the list goes on that
 * way, sets the board turning a page instead. Nothing is taken while the
 * board is still moving. A confirm is only answered for a spell that has an
 * effect to play.
 */
#include <decomp/types.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/input.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/spellfx.h>
#include <persona/btlp/text.h>

/* The pad's four directions, and the neighbour-table column each one reads. */
#define PAD_DIRS  0xF000
#define PAD_UP    0x1000
#define PAD_RIGHT 0x2000
#define PAD_DOWN  0x4000
#define PAD_LEFT  0x8000

#define NAV_UP    0
#define NAV_DOWN  1
#define NAV_LEFT  2
#define NAV_RIGHT 3

/* The cursor's artwork, and how far left of the item it is drawn. */
#define CURSOR_CELLS 14
#define CURSOR_LEFT  7

#define DEBUG_ITEMS  15
#define PRESET_ITEMS 9
#define ORDERS_ITEMS 3
#define TACTICS      3

/* The two-way tables' columns. */
#define NAV_BACK 0
#define NAV_NEXT 1

/* The tick each kind of step plays. */
#define TICK_MOVE   0
#define TICK_PICK   1
#define TICK_BACK   2
#define TICK_CHANGE 3

/* The spell list: ten to a page, a row is two across, and the last page
   ends before this spell. */
#define SPELL_ITEMS 10
#define SPELL_ROW   2
#define SPELL_END   0xF3
#define NAV_SIDE    2

/* Which ways the list goes on from this page, and what the board is set
   doing to turn to the page before and the page after. */
#define SPELL_MORE_BACK 1
#define SPELL_MORE_NEXT 2
#define SPELL_TURN_BACK 7
#define SPELL_TURN_NEXT 6

/* Where an item's cursor goes. */
typedef struct {
    short x;
    short y;
} BtlMenuSpot;

extern short      g_btl_debug_row;
extern short      g_btl_preset_row;
extern short      g_btl_orders_row;
extern short      g_btl_tactics_row;
extern short      g_btl_spell_row;
extern int        g_btl_spell_slot;
extern u_char     g_btl_orders_line0[];
extern u_char     g_btl_orders_line1[];
extern u_char     g_btl_orders_line2[];
extern BtlGfxCell g_btl_menu_cursor[CURSOR_CELLS];

u_char g_btl_debug_nav[DEBUG_ITEMS][4] = {
    { 0x0C, 0x03, 0x02, 0x01 }, { 0x0D, 0x04, 0x00, 0x02 },
    { 0x0E, 0x05, 0x01, 0x00 }, { 0x00, 0x06, 0x05, 0x04 },
    { 0x01, 0x07, 0x03, 0x05 }, { 0x02, 0x08, 0x04, 0x03 },
    { 0x03, 0x09, 0x08, 0x07 }, { 0x04, 0x0A, 0x06, 0x08 },
    { 0x05, 0x0B, 0x07, 0x06 }, { 0x06, 0x0C, 0x0B, 0x0A },
    { 0x07, 0x0D, 0x09, 0x0B }, { 0x08, 0x0E, 0x0A, 0x09 },
    { 0x09, 0x00, 0x0E, 0x0D }, { 0x0A, 0x01, 0x0C, 0x0E },
    { 0x0B, 0x02, 0x0D, 0x0C },
};

BtlMenuSpot g_btl_debug_spots[DEBUG_ITEMS] = {
    { -0x88, -0x1C }, { -0x28, -0x1C }, { 0x38, -0x1C },
    { -0x88, -0x10 }, { -0x28, -0x10 }, { 0x38, -0x10 },
    { -0x88, -0x04 }, { -0x28, -0x04 }, { 0x38, -0x04 },
    { -0x88, 0x08 },  { -0x28, 0x08 },  { 0x38, 0x08 },
    { -0x88, 0x14 },  { -0x28, 0x14 },  { 0x38, 0x14 },
};

u_char g_btl_preset_nav[PRESET_ITEMS][4] = {
    { 0x06, 0x03, 0x02, 0x01 }, { 0x07, 0x04, 0x00, 0x02 },
    { 0x08, 0x05, 0x01, 0x00 }, { 0x00, 0x06, 0x05, 0x04 },
    { 0x01, 0x07, 0x03, 0x05 }, { 0x02, 0x08, 0x04, 0x03 },
    { 0x03, 0x00, 0x08, 0x07 }, { 0x04, 0x01, 0x06, 0x08 },
    { 0x05, 0x02, 0x07, 0x06 },
};

BtlMenuSpot g_btl_preset_spots[PRESET_ITEMS] = {
    { -0x88, -0x14 }, { -0x28, -0x14 }, { 0x38, -0x14 },
    { -0x88, -0x04 }, { -0x28, -0x04 }, { 0x38, -0x04 },
    { -0x88, 0x0C },  { -0x28, 0x0C },  { 0x38, 0x0C },
};

u_char *g_btl_orders_help[ORDERS_ITEMS] = {
    g_btl_orders_line0,
    g_btl_orders_line1,
    g_btl_orders_line2,
};

u_char g_btl_orders_nav[ORDERS_ITEMS][2] = {
    { 0x02, 0x01 }, { 0x00, 0x02 }, { 0x01, 0x00 },
};

BtlMenuSpot g_btl_orders_spots[ORDERS_ITEMS] = {
    { -0x88, -0x06 }, { -0x28, -0x06 }, { 0x38, -0x06 },
};

u_char g_btl_tactics_nav[BTL_PARTY][2] = {
    { 0x04, 0x01 }, { 0x00, 0x02 }, { 0x01, 0x03 }, { 0x02, 0x04 },
    { 0x03, 0x00 },
};

u_char g_btl_tactic_cycle[TACTICS][2] = {
    { 0x02, 0x01 }, { 0x00, 0x02 }, { 0x01, 0x00 },
};

BtlMenuSpot g_btl_tactics_spots[BTL_PARTY] = {
    { -0x78, -0x18 }, { -0x78, -0x0C }, { -0x78, 0x00 }, { -0x78, 0x0C },
    { -0x78, 0x18 },
};

signed char g_btl_spell_nav[SPELL_ITEMS][3] = {
    { -2, 2, 1 }, { -1, 3, 0 },  { 0, 4, 3 },  { 1, 5, 2 },  { 2, 6, 5 },
    { 3, 7, 4 },  { 4, 8, 7 },   { 5, 9, 6 },  { 6, 10, 9 }, { 7, 11, 8 },
};

int BtlDebugUpdate(void)
{
    BtlGfxCell *cell;
    int         keys;
    int         row;
    int         i;

    keys = BtlMenuKey();
    if (keys & PAD_DIRS) {
        BtlSePlay(1, 0);
    }
    if (keys & PAD_UP) {
        g_btl_debug_row = g_btl_debug_nav[g_btl_debug_row][NAV_UP];
    }
    if (keys & PAD_DOWN) {
        g_btl_debug_row = g_btl_debug_nav[g_btl_debug_row][NAV_DOWN];
    }
    if (keys & PAD_LEFT) {
        g_btl_debug_row = g_btl_debug_nav[g_btl_debug_row][NAV_LEFT];
    }
    if (keys & PAD_RIGHT) {
        g_btl_debug_row = g_btl_debug_nav[g_btl_debug_row][NAV_RIGHT];
    }

    i = 0;
    row = g_btl_debug_row;
    cell = g_btl_menu_cursor;
    do {
        cell->x = g_btl_debug_spots[row].x - CURSOR_LEFT;
        cell->y = g_btl_debug_spots[row].y;
        i++;
        cell++;
    } while (i < CURSOR_CELLS);

    if (g_btl_pad1_edge & g_btl_key_confirm) {
        return g_btl_debug_row;
    }
    if (g_btl_pad1_edge & g_btl_key_cancel) {
        return -1;
    }
    return BTL_PICK_WAIT;
}

int BtlPresetMenuUpdate(int on_field)
{
    BtlGfxCell *cell;
    int         keys;
    int         row;
    int         i;

    keys = (u_short)BtlMenuKey();
    if (keys & PAD_DIRS) {
        BtlSePlay(1, 0);
    }
    if (keys & PAD_UP) {
        g_btl_preset_row = g_btl_preset_nav[g_btl_preset_row][NAV_UP];
    }
    if (keys & PAD_DOWN) {
        g_btl_preset_row = g_btl_preset_nav[g_btl_preset_row][NAV_DOWN];
    }
    if (keys & PAD_LEFT) {
        g_btl_preset_row = g_btl_preset_nav[g_btl_preset_row][NAV_LEFT];
    }
    if (keys & PAD_RIGHT) {
        g_btl_preset_row = g_btl_preset_nav[g_btl_preset_row][NAV_RIGHT];
    }

    i = 0;
    row = g_btl_preset_row;
    cell = g_btl_menu_cursor;
    do {
        cell->x = g_btl_preset_spots[row].x - CURSOR_LEFT;
        cell->y = g_btl_preset_spots[row].y;
        i++;
        cell++;
    } while (i < CURSOR_CELLS);

    if (g_btl_pad1 & g_btl_key_r1) {
        BtlStandPreset(PRESET_LIVE);
        return BTL_PICK_WAIT;
    }
    if ((g_btl_pad1_edge & g_btl_key_confirm)
        && g_btl_preset_row != PRESET_LIVE) {
        return g_btl_preset_row;
    }
    if (g_btl_pad1_edge & g_btl_key_cancel) {
        return -1;
    }
    if (g_btl_pad1_edge & g_btl_key_abort) {
        return -2;
    }
    if (on_field == 0) {
        BtlPlacePreset(g_btl_preset_row);
    } else {
        BtlStandPreset(g_btl_preset_row);
    }
    return BTL_PICK_WAIT;
}

int BtlOrdersMenuUpdate(void)
{
    BtlGfxCell *cell;
    int         keys;
    int         row;
    int         i;

    keys = (u_short)BtlMenuKey();
    if (keys & PAD_LEFT) {
        g_btl_orders_row = g_btl_orders_nav[g_btl_orders_row][NAV_BACK];
    }
    if (keys & PAD_RIGHT) {
        g_btl_orders_row = g_btl_orders_nav[g_btl_orders_row][NAV_NEXT];
    }
    if (keys & (PAD_LEFT | PAD_RIGHT)) {
        BtlSePlay(1, TICK_MOVE);
    }
    if (g_btl_no_help == 0) {
        BtlOpenMessage(0, 0, g_btl_orders_help[g_btl_orders_row], PICK_HELP_X,
                       PICK_HELP_Y);
    } else {
        BtlCloseMessage(0);
    }

    i = 0;
    row = g_btl_orders_row;
    cell = g_btl_menu_cursor;
    do {
        cell->x = g_btl_orders_spots[row].x - CURSOR_LEFT;
        cell->y = g_btl_orders_spots[row].y;
        i++;
        cell++;
    } while (i < CURSOR_CELLS);

    if (g_btl_pad1_edge & g_btl_key_confirm) {
        BtlCloseMessage(0);
        return g_btl_orders_row;
    }
    if (g_btl_pad1_edge & (g_btl_key_cancel | g_btl_key_abort)) {
        return -1;
    }
    return BTL_PICK_WAIT;
}

int BtlTacticsMenuUpdate(void)
{
    BtlGfxCell *cell;
    int         keys;
    int         row;
    int         i;

    keys = (u_short)BtlMenuKey();
    if (keys & (PAD_UP | PAD_DOWN)) {
        BtlSePlay(1, TICK_MOVE);
    }
    if (keys & (PAD_LEFT | PAD_RIGHT)) {
        BtlSePlay(1, TICK_CHANGE);
    }
    if (keys & PAD_UP) {
        g_btl_tactics_row = g_btl_tactics_nav[g_btl_tactics_row][NAV_BACK];
    }
    if (keys & PAD_DOWN) {
        g_btl_tactics_row = g_btl_tactics_nav[g_btl_tactics_row][NAV_NEXT];
    }
    if (keys & PAD_LEFT) {
        g_btl_actors[g_btl_tactics_row].tactic =
            g_btl_tactic_cycle[g_btl_actors[g_btl_tactics_row].tactic][NAV_BACK];
    }
    if (keys & PAD_RIGHT) {
        g_btl_actors[g_btl_tactics_row].tactic =
            g_btl_tactic_cycle[g_btl_actors[g_btl_tactics_row].tactic][NAV_NEXT];
    }

    i = 0;
    row = g_btl_tactics_row;
    cell = g_btl_menu_cursor;
    do {
        cell->x = g_btl_tactics_spots[row].x - CURSOR_LEFT;
        cell->y = g_btl_tactics_spots[row].y;
        i++;
        cell++;
    } while (i < CURSOR_CELLS);
    BtlRefreshTacticsLines();

    if (g_btl_pad1_edge & g_btl_key_cancel) {
        return 0;
    }
    if (g_btl_pad1_edge & g_btl_key_abort) {
        return -2;
    }
    return BTL_PICK_WAIT;
}

int BtlSpellMenuUpdate(BtlObj *board)
{
    int   keys;
    int   more;
    short row;
    int   slot;

    keys = (u_short)BtlMenuKey();
    if (board->motion == 0) {
        more = g_btl_spell_slot != 0;
        if (g_btl_spell_slot + SPELL_ITEMS < SPELL_END) {
            more |= SPELL_MORE_NEXT;
        }
        if (keys & PAD_UP) {
            g_btl_spell_row = g_btl_spell_nav[g_btl_spell_row][NAV_UP];
        }
        if (keys & PAD_DOWN) {
            g_btl_spell_row = g_btl_spell_nav[g_btl_spell_row][NAV_DOWN];
        }
        if (keys & PAD_LEFT) {
            g_btl_spell_row = g_btl_spell_nav[g_btl_spell_row][NAV_SIDE];
        }
        if (keys & PAD_RIGHT) {
            g_btl_spell_row = g_btl_spell_nav[g_btl_spell_row][NAV_SIDE];
        }
        if ((keys & PAD_DIRS) && (u_short)g_btl_spell_row < SPELL_ITEMS) {
            BtlSePlay(1, TICK_MOVE);
        }

        row = g_btl_spell_row;
        if (row < 0) {
            g_btl_spell_row = row + SPELL_ROW;
            if ((more & SPELL_MORE_BACK) == 0) {
                return BTL_PICK_WAIT;
            }
            board->motion = SPELL_TURN_BACK;
        } else if (row >= SPELL_ITEMS) {
            g_btl_spell_row = row - SPELL_ROW;
            if ((more & SPELL_MORE_NEXT) == 0) {
                return BTL_PICK_WAIT;
            }
            board->motion = SPELL_TURN_NEXT;
        } else {
            goto take;
        }
        BtlSePlay(1, TICK_MOVE);
        return BTL_PICK_WAIT;
    }

take:
    if ((g_btl_pad1_edge & g_btl_key_confirm) && board->motion == 0) {
        slot = g_btl_spell_row + g_btl_spell_slot;
        if (g_btl_spell_fx[slot].start == 0) {
            return BTL_PICK_WAIT;
        }
        BtlSePlay(1, TICK_PICK);
        return slot;
    }
    if ((g_btl_pad1_edge & g_btl_key_cancel) && board->motion == 0) {
        BtlSePlay(1, TICK_BACK);
        return -1;
    }
    return BTL_PICK_WAIT;
}
