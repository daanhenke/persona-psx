/* Persona 1 (JP) - the board a won fight is shown on, and two more of the
 * battle's fixed boards.  BTLP only.
 *   0x800A913C BtlOpenBoard1F  0x800A9440 BtlCloseBoard1F
 *   0x800A9468 BtlOpenBoard1D  0x800A94A0 BtlCloseBoard1D
 *
 * Board 0x1F goes up once a fight is won, and its open fills it in first. The
 * two columns of marks start grey. Each of the five party records with a key
 * draws the experience it won and the total at +0x78, and each column's mark
 * is lit where BtlBattleResults raised its flag - the level, and Char.unk56 -
 * and left as two spaces where it did not. A record without a key has its
 * numbers and marks blanked. Then the money won is drawn and added to the
 * party's, held between nought and 999,999,999, the item left behind is named
 * and handed over, and the board goes up.
 *
 * The pairs 0x1F and 0x1D are named the way every pair in boards.c is: after
 * the index the window record takes, which is the picture the board is drawn
 * from. Board 0x1D is the only one of the family that does not stand where the
 * rest do: its position table puts it higher and well to the left.
 *
 * BtlOpenBoard1F is the image to the byte, and objdiff still counts two rows
 * against it: the level column's clut is reached as `text[row - 5]`, which
 * gcc folds into a relocation against g_btl_board1F_text with addend -0x33,
 * and splat can only name that address D_800DCFF1. Three things in the loop
 * are load-bearing. The first loop counts up, and gcc reverses it. `row` is
 * worked out from the counter rather than stepped beside it, which orders the
 * steps at the bottom of the loop. And the level column's clut goes through a
 * local written in both arms: the two stores still merge, and the extra
 * reference is what ranks that row's register where the image has it.
 */
#include <decomp/types.h>
#include <persona/common/item.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/board.h>
#include <persona/btlp/number.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>

/* The party records the board has a row for. */
#define BOARD1F_MEMBERS 5

/* The text rows of the two columns of marks: ten from the level column's
   first, and the second column five rows after the first. */
#define BOARD1F_MARK_ROWS  10
#define BOARD1F_LEVEL_ROW  15
#define BOARD1F_UNK56_ROW  20
#define BOARD1F_COLUMN     5

#define BOARD1F_CLUT_GREY  0x23
#define BOARD1F_CLUT_LIT   0x21

/* The mark's two glyphs, the space that stands in for it, and a blanked run. */
#define BOARD1F_MARK_0     0xBA
#define BOARD1F_MARK_1     0xB5
#define BOARD1F_SPACE      0xCC
#define BOARD1F_BLANK      0xFF

#define BOARD1F_EXP_CELLS   7
#define BOARD1F_EXP_WIDTH   6
#define BOARD1F_MARK_CELLS  2
#define BOARD1F_MONEY_WIDTH 5

#define MONEY_MAX 999999999

extern BtlObj            *g_btl_board1D;
extern BtlObj            *g_btl_board1F;

extern const BtlBoardDef  g_btl_board1D_defs[];
extern const long         g_btl_board1D_pos[];
extern BtlBoardDef        g_btl_board1F_defs[];
extern const long         g_btl_board1F_pos[];

/* Where board 0x1F reads what it draws: its text rows, the two numbers per
   member, the two columns of marks, the money and the item's name. */
extern BtlGfxText         g_btl_board1F_text[];
extern u_char             g_btl_board1F_exp_cells[][BOARD1F_EXP_CELLS];
extern u_char             g_btl_board1F_unk78_cells[][BOARD1F_EXP_CELLS];
extern u_char             g_btl_board1F_level_marks[][BOARD1F_MARK_CELLS];
extern u_char             g_btl_board1F_unk56_marks[][BOARD1F_MARK_CELLS];
extern u_char             g_btl_board1F_money_cells[];
extern BtlNameCells       g_btl_board1F_item_name;

void BtlOpenBoard1F(void)
{
    BtlActor *a;
    int       i;
    int       row;
    int       space;
    int       blank;
    int       clut;

    for (i = 0; i < BOARD1F_MARK_ROWS; i++) {
        g_btl_board1F_text[BOARD1F_LEVEL_ROW + i].clut = BOARD1F_CLUT_GREY;
    }
    i = 0;
    space = BOARD1F_SPACE;
    blank = BOARD1F_BLANK;
    do {
        a = &g_btl_actors[i];
        row = i + BOARD1F_UNK56_ROW;
        if (a->c.key != 0) {
            BtlDrawNumber(g_btl_board1F_exp_cells[i], a->unk74,
                          BOARD1F_EXP_WIDTH);
            BtlDrawNumber(g_btl_board1F_unk78_cells[i], a->unk78,
                          BOARD1F_EXP_WIDTH);
            if (a->level_up != 0) {
                g_btl_board1F_level_marks[i][0] = BOARD1F_MARK_0;
                g_btl_board1F_level_marks[i][1] = BOARD1F_MARK_1;
                clut = BOARD1F_CLUT_LIT;
                g_btl_board1F_text[row - BOARD1F_COLUMN].clut = clut;
            } else {
                g_btl_board1F_level_marks[i][0] = space;
                g_btl_board1F_level_marks[i][1] = space;
                clut = BOARD1F_CLUT_GREY;
                g_btl_board1F_text[row - BOARD1F_COLUMN].clut = clut;
            }
            if (a->unk56_up != 0) {
                g_btl_board1F_unk56_marks[i][0] = BOARD1F_MARK_0;
                g_btl_board1F_unk56_marks[i][1] = BOARD1F_MARK_1;
                g_btl_board1F_text[row].clut = BOARD1F_CLUT_LIT;
            } else {
                g_btl_board1F_unk56_marks[i][0] = space;
                g_btl_board1F_unk56_marks[i][1] = space;
                g_btl_board1F_text[row].clut = BOARD1F_CLUT_GREY;
            }
        } else {
            g_btl_board1F_exp_cells[i][0] = blank;
            g_btl_board1F_unk78_cells[i][0] = blank;
            g_btl_board1F_level_marks[i][0] = blank;
            g_btl_board1F_unk56_marks[i][0] = blank;
        }
        i++;
    } while (i < BOARD1F_MEMBERS);

    BtlDrawNumber(g_btl_board1F_money_cells, g_btl_won_money,
                  BOARD1F_MONEY_WIDTH);
    G_MONEY += g_btl_won_money;
    G_MONEY = G_MONEY < 0 ? 0 : G_MONEY > MONEY_MAX ? MONEY_MAX : G_MONEY;
    g_btl_board1F_item_name =
        *(BtlNameCells *)g_item_defs[g_btl_drop_item].name;
    if (g_btl_drop_item != 0) {
        BtlGiveItem(g_btl_drop_item);
    }
    g_btl_board1F = BtlBoardOpen(g_btl_board1F_defs, g_btl_board1F_pos);
}

void BtlCloseBoard1F(void)
{
    BtlBoardShut(g_btl_board1F);
}

void BtlOpenBoard1D(void)
{
    g_btl_board1D = BtlBoardOpen(g_btl_board1D_defs, g_btl_board1D_pos);
}

void BtlCloseBoard1D(void)
{
    BtlBoardShut(g_btl_board1D);
}
