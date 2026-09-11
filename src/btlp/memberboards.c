/* Persona 1 (JP) - the five member boards along the bottom.  BTLP only.
 *   0x800A9EC4 BtlOpenMemberBoards  0x800AA024 BtlCloseMemberBoards
 *
 * One board per party member, put up as the command stage opens and taken
 * down as it closes. All five come from the same table of records with the
 * window's picture index rewritten between calls, which is how the five
 * boards are drawn from five different pictures out of one description.
 *
 * A slot nobody is in still gets a board: what changes is that its artwork
 * list is cut to a single cell, so only the frame is drawn, and the HP and SP
 * bars are left alone. A member who is there gets the full fifteen cells and
 * their two bars tinted for how much is left.
 *
 * Each board is started a few frames after the one before it, so they arrive
 * in order rather than all at once, and each is put straight out of the
 * drawing pass - whatever opens the stage brings them back in. The last
 * record of each board's chain is handed back as soon as it is built: the
 * board only needs the ones in front of it, and BtlObjLast leaves the one
 * before it in g_btl_obj_prev, whose attached link has to be cut or it would
 * still point at what was just freed.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/board.h>
#include <persona/btlp/object.h>
#include <persona/common/char.h>

#define MEMBER_BOARDS 5

/* Where the first board stands and how far the next one is below it. */
#define BOARD_X    0xA00000
#define BOARD_Y    0x2C0000
#define BOARD_STEP 0x280000

/* The picture the first board is drawn from; the rest follow it, and where
   that byte sits in the description. */
#define BOARD_FIRST_PICTURE 5
#define BOARD_PICTURE_AT    0xC

/* Frames between one board arriving and the next. */
#define BOARD_DELAY 4

/* Cells of the board's artwork that are drawn - all of them for a member who
   is there, and the frame alone for an empty slot. */
#define BOARD_CELLS_FULL  0xF
#define BOARD_CELLS_EMPTY 1

/* Where the two bars sit in a board's block of sprites. */
#define BOARD_HP_BAR 0x30
#define BOARD_SP_BAR 0x48
#define BOARD_SPRITES 0xB4

extern void BtlSetGaugeColour(const Char *c, u_char *hp, u_char *sp);

extern BtlBoardDef  g_btl_member_board_defs[];
extern u_char       g_btl_member_board_sprites[][BOARD_SPRITES];
extern BtlGfxList  *g_btl_member_board_gfx[];
extern BtlObj      *g_btl_member_boards[];

/* Three things here are the shape rather than the meaning, and all three had
   to be right before the routine came out.

   The sprite block is walked by hand rather than indexed, so the two bars are
   an offset off one pointer instead of two pointers of their own - one
   register moving by 0xB4 a turn, not two. That step is written after the
   counter's, which is what leaves the counter for the first call's delay slot
   and the step for BtlObjLast's, the way the image has them.

   The description is reached through the picture byte and not the other way
   round: the routine keeps the address of the byte it rewrites and takes the
   table's own address back off it, which is the one register the image
   carries for both. */
void BtlOpenMemberBoards(void)
{
    long    pos[3];
    BtlObj *obj;
    u_char *bars;
    u_char *picture;
    long    y;
    int     i;

    i       = 0;
    bars    = g_btl_member_board_sprites[0];
    picture = &g_btl_member_board_defs[1].index;
    y       = BOARD_Y;
    do {
        if (g_btl_actors[i].c.key != 0) {
            g_btl_member_board_gfx[i]->count = BOARD_CELLS_FULL;
            BtlSetGaugeColour(&g_btl_actors[i].c, bars + BOARD_HP_BAR,
                              bars + BOARD_SP_BAR);
        } else {
            g_btl_member_board_gfx[i]->count = BOARD_CELLS_EMPTY;
        }
        pos[0] = BOARD_X;
        pos[1] = y;
        pos[2] = 0;
        *picture = i + BOARD_FIRST_PICTURE;
        obj = BtlBoardOpen((BtlBoardDef *)(picture - BOARD_PICTURE_AT), pos);
        g_btl_member_boards[i] = obj;
        obj->unk58 = (long)&g_btl_member_boards[i];
        BtlObjSetTimer(g_btl_member_boards[i], i * BOARD_DELAY);
        BtlObjSetAttr(g_btl_member_boards[i], BTL_OBJ_HIDDEN);
        BtlObjFree(BtlObjLast(g_btl_member_boards[i]));
        y += BOARD_STEP;
        g_btl_obj_prev->attached = 0;
        i++;
        bars += BOARD_SPRITES;
    } while (i < MEMBER_BOARDS);
}

void BtlCloseMemberBoards(void)
{
    int i;

    i = 0;
    do {
        BtlBoardShut(g_btl_member_boards[i]);
        i++;
    } while (i < MEMBER_BOARDS);
}
