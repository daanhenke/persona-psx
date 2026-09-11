/* Persona 1 (JP) - the battle's fixed boards, opened and shut.  BTLP only.
 *   0x800AA1E0 BtlOpenDebugBoard  0x800AA220 BtlCloseDebugBoard
 *   0x800AA248 BtlOpenBoard0E     0x800AA280 BtlCloseBoard0E
 *   0x800AA2A8 BtlOpenBoard0F     0x800AA2FC BtlCloseBoard0F
 *   0x800AA324 BtlOpenBoard10     0x800AA368 BtlCloseBoard10
 *   0x800AA390 BtlOpenBoard11     0x800AA3D4 BtlCloseBoard11
 *   0x800AA3FC BtlOpenBoard18     0x800AA440 BtlCloseBoard18
 *   0x800AA468 BtlOpenFlagBoard   0x800AA4A0 BtlCloseFlagBoard
 *
 * Seven pairs, each the same two calls with a different table of records. The
 * only thing that differs between the tables is the index the window record
 * takes, which is which picture the board is drawn from, and that index is
 * what the names carry where there is nothing better to call one by.
 *
 * Four of the seven have no caller left in the image. They are here because
 * they are in it: the assembler laid them down between the two that are used,
 * and the space they take is part of the overlay.
 *
 * The two that are live belong to screens beside the battle rather than in
 * it - 0x12 is the frame the debug menu draws inside and 0x21 the one the
 * flag editor uses - which is why the debug board clears the row that menu
 * was left on before it puts the frame up.
 *
 * Each pair keeps the record BtlBoardOpen returned in a word of its own, so
 * the close has something to hand back; nothing else reads those words.
 */
#include <decomp/types.h>
#include <persona/btlp/board.h>
#include <persona/btlp/object.h>

/* The seven tables, out of the run of them at 0x800DFAF4. */
extern const BtlBoardDef g_btl_board0E_defs[];
extern const BtlBoardDef g_btl_board0F_defs[];
extern const BtlBoardDef g_btl_board10_defs[];
extern const BtlBoardDef g_btl_board11_defs[];
extern const BtlBoardDef g_btl_board18_defs[];
extern const BtlBoardDef g_btl_debug_board_defs[];
extern const BtlBoardDef g_btl_flag_board_defs[];

/* Where three of them stand; the other four carry a position on the stack. */
extern const long g_btl_board0E_pos[];
extern const long g_btl_debug_board_pos[];
extern const long g_btl_flag_board_pos[];

/* What each pair leaves behind for its own close to pick up. */
extern BtlObj *g_btl_board0E;
extern BtlObj *g_btl_board0F;
extern BtlObj *g_btl_board10;
extern BtlObj *g_btl_board11;
extern BtlObj *g_btl_board18;
extern BtlObj *g_btl_debug_board;
extern BtlObj *g_btl_flag_board;

/* Which line of the debug menu is up. */
extern short g_btl_debug_row;

/* Where the four that carry their own position stand. */
#define BOARD_X      0xA00000
#define BOARD_Y      0xC40000
#define BOARD_HIGH_Y 0x780000

void BtlOpenDebugBoard(void)
{
    g_btl_debug_row   = 0;
    g_btl_debug_board = BtlBoardOpen(g_btl_debug_board_defs, g_btl_debug_board_pos);
}

void BtlCloseDebugBoard(void)
{
    BtlBoardShut(g_btl_debug_board);
}

void BtlOpenBoard0E(void)
{
    g_btl_board0E = BtlBoardOpen(g_btl_board0E_defs, g_btl_board0E_pos);
}

void BtlCloseBoard0E(void)
{
    BtlBoardShut(g_btl_board0E);
}

/* The one board that is taken straight back out of the drawing pass, so
   whatever puts it up decides when it is seen. */
void BtlOpenBoard0F(void)
{
    long pos[3];

    pos[0] = BOARD_X;
    pos[1] = BOARD_HIGH_Y;
    pos[2] = 0;
    g_btl_board0F = BtlBoardOpen(g_btl_board0F_defs, pos);
    g_btl_board0F->attr |= BTL_OBJ_HIDDEN;
}

void BtlCloseBoard0F(void)
{
    BtlBoardShut(g_btl_board0F);
}

void BtlOpenBoard10(void)
{
    long pos[3];

    pos[0] = BOARD_X;
    pos[1] = BOARD_Y;
    pos[2] = 0;
    g_btl_board10 = BtlBoardOpen(g_btl_board10_defs, pos);
}

void BtlCloseBoard10(void)
{
    BtlBoardShut(g_btl_board10);
}

void BtlOpenBoard11(void)
{
    long pos[3];

    pos[0] = BOARD_X;
    pos[1] = BOARD_Y;
    pos[2] = 0;
    g_btl_board11 = BtlBoardOpen(g_btl_board11_defs, pos);
}

void BtlCloseBoard11(void)
{
    BtlBoardShut(g_btl_board11);
}

void BtlOpenBoard18(void)
{
    long pos[3];

    pos[0] = BOARD_X;
    pos[1] = BOARD_Y;
    pos[2] = 0;
    g_btl_board18 = BtlBoardOpen(g_btl_board18_defs, pos);
}

void BtlCloseBoard18(void)
{
    BtlBoardShut(g_btl_board18);
}

void BtlOpenFlagBoard(void)
{
    g_btl_flag_board = BtlBoardOpen(g_btl_flag_board_defs, g_btl_flag_board_pos);
}

void BtlCloseFlagBoard(void)
{
    BtlBoardShut(g_btl_flag_board);
}
