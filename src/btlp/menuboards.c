/* Persona 1 (JP) - the boards the round's menus stand in.  BTLP only.
 *   0x800AB95C BtlOpenOrdersBoard     0x800AB994 BtlCloseOrdersBoard
 *   0x800AB9BC BtlOpenTacticsBoard    0x800AB9FC BtlCloseTacticsBoard
 *   0x800ABA24 BtlRefreshTacticsLines
 *   0x800ABAE0 BtlOpenConfigBoard     0x800ABB18 BtlCloseConfigBoard
 *   0x800ABB40 BtlOpenBoard15         0x800ABB78 BtlCloseBoard15
 *   0x800ABBA0 BtlOpenStatusBoard     0x800ABBD8 BtlCloseStatusBoard
 *
 * Five of the boards BtlBoardOpen builds, each belonging to one of the pages
 * BtlStageCommand opens the round with: the standing-orders menu, the
 * settings page, the per-member tactics page the settings page opens on top
 * of itself, and the status view FUN_8009F638 turns the frame over inside.
 * The fifth has no page that is named yet, so it carries the index of the
 * picture it is drawn from, 0x15, the way the ones in boards.c do.
 *
 * The tactics page is the only one with anything to fill in. Its board is
 * twenty-one lines: a heading and then five rows four columns wide, one row
 * per party member. BtlRefreshTacticsLines writes the member's name into the
 * first column and puts every column in the live colour, or writes EMPTY and
 * greys the whole row for a slot nobody is in; the column standing for the
 * order that member is under is then put in the picked colour. That is why
 * opening the board refreshes the lines first - the board goes up already
 * saying what the party is doing.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/board.h>
#include <persona/btlp/object.h>

/* Party members, and how far apart the four columns of a row are. */
#define TACTICS_ROWS    5
#define TACTICS_COLUMN  5

/* The three colours a tactics line is drawn in: the member is here, the slot
   is empty, and this is the column that member's order is under. */
#define TACTICS_CLUT_LIVE   0x20
#define TACTICS_CLUT_EMPTY  0x23
#define TACTICS_CLUT_PICKED 0x21

extern const BtlBoardDef g_btl_orders_board_defs[];
extern const BtlBoardDef g_btl_tactics_board_defs[];
extern const BtlBoardDef g_btl_config_board_defs[];
extern const BtlBoardDef g_btl_board15_defs[];
extern const BtlBoardDef g_btl_status_board_defs[];

extern const long g_btl_orders_board_pos[];
extern const long g_btl_tactics_board_pos[];
extern const long g_btl_config_board_pos[];
extern const long g_btl_board15_pos[];
extern const long g_btl_status_board_pos[];

extern BtlObj *g_btl_orders_board;
extern BtlObj *g_btl_tactics_board;
extern BtlObj *g_btl_config_board;
extern BtlObj *g_btl_board15;
extern BtlObj *g_btl_status_board;

/* The board's lines, and the two things a row's first column is filled from. */
extern BtlGfxText  g_btl_tactics_lines[];
extern u_char      g_btl_member_names[][8];
extern const u_char g_btl_name_empty[];

void BtlRefreshTacticsLines(void);

void BtlOpenOrdersBoard(void)
{
    g_btl_orders_board = BtlBoardOpen(g_btl_orders_board_defs, g_btl_orders_board_pos);
}

void BtlCloseOrdersBoard(void)
{
    BtlBoardShut(g_btl_orders_board);
}

void BtlOpenTacticsBoard(void)
{
    BtlRefreshTacticsLines();
    g_btl_tactics_board = BtlBoardOpen(g_btl_tactics_board_defs, g_btl_tactics_board_pos);
}

void BtlCloseTacticsBoard(void)
{
    BtlBoardShut(g_btl_tactics_board);
}

/* One row per member, four columns each, laid out column after column - so
   the row for member i is lines[i], lines[i + 5], lines[i + 10], lines[i +
   15], and the order that member is under picks one of the last three. */
void BtlRefreshTacticsLines(void)
{
    BtlGfxText *row;
    int         i;

    row = g_btl_tactics_lines;
    i   = 0;
    do {
        if (g_btl_actors[i].c.key != 0) {
            row[0].text                 = g_btl_member_names[i];
            row[0].clut                 = TACTICS_CLUT_LIVE;
            row[TACTICS_COLUMN].clut    = TACTICS_CLUT_LIVE;
            row[TACTICS_COLUMN * 2].clut = TACTICS_CLUT_LIVE;
            row[TACTICS_COLUMN * 3].clut = TACTICS_CLUT_LIVE;
            row[(g_btl_actors[i].unkC8 + 1) * TACTICS_COLUMN].clut = TACTICS_CLUT_PICKED;
        } else {
            row[0].text                 = g_btl_name_empty;
            row[0].clut                 = TACTICS_CLUT_EMPTY;
            row[TACTICS_COLUMN].clut    = TACTICS_CLUT_EMPTY;
            row[TACTICS_COLUMN * 2].clut = TACTICS_CLUT_EMPTY;
            row[TACTICS_COLUMN * 3].clut = TACTICS_CLUT_EMPTY;
        }
        i++;
        row++;
    } while (i < TACTICS_ROWS);
}

void BtlOpenConfigBoard(void)
{
    g_btl_config_board = BtlBoardOpen(g_btl_config_board_defs, g_btl_config_board_pos);
}

void BtlCloseConfigBoard(void)
{
    BtlBoardShut(g_btl_config_board);
}

void BtlOpenBoard15(void)
{
    g_btl_board15 = BtlBoardOpen(g_btl_board15_defs, g_btl_board15_pos);
}

void BtlCloseBoard15(void)
{
    BtlBoardShut(g_btl_board15);
}

void BtlOpenStatusBoard(void)
{
    g_btl_status_board = BtlBoardOpen(g_btl_status_board_defs, g_btl_status_board_pos);
}

void BtlCloseStatusBoard(void)
{
    BtlBoardShut(g_btl_status_board);
}
