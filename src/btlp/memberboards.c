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
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/board.h>
#include <persona/btlp/object.h>
#include <persona/common/char.h>

#define MEMBER_BOARDS 5

/* Where the first board stands and how far the next one is below it. */
#define BOARD_X    0xA00000
#define BOARD_Y    0x2C0000
#define BOARD_STEP 0x280000

/* The picture the first board is drawn from; the rest follow it. */
#define BOARD_FIRST_PICTURE 5

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

/* Not matched: the whole routine is the right instructions in the right
   order, and what is left is the register allocator - the sprite block is
   walked as two pointers here where the image walks one and adds the two bar
   offsets to it, and the description's address is rematerialised from the
   picture byte's rather than loaded. A permuter job rather than a reading
   one. */
#ifdef NON_MATCHING
void BtlOpenMemberBoards(void)
{
    long    pos[3];
    BtlObj *obj;
    long    y;
    int     i;

    i = 0;
    y = BOARD_Y;
    do {
        if (g_btl_actors[i].c.key != 0) {
            g_btl_member_board_gfx[i]->count = BOARD_CELLS_FULL;
            BtlSetGaugeColour(&g_btl_actors[i].c,
                              &g_btl_member_board_sprites[i][BOARD_HP_BAR],
                              &g_btl_member_board_sprites[i][BOARD_SP_BAR]);
        } else {
            g_btl_member_board_gfx[i]->count = BOARD_CELLS_EMPTY;
        }
        pos[0] = BOARD_X;
        pos[1] = y;
        pos[2] = 0;
        g_btl_member_board_defs[1].index = i + BOARD_FIRST_PICTURE;
        obj = BtlBoardOpen(g_btl_member_board_defs, pos);
        g_btl_member_boards[i] = obj;
        obj->unk58 = (long)&g_btl_member_boards[i];
        BtlObjSetTimer(g_btl_member_boards[i], i * BOARD_DELAY);
        BtlObjSetAttr(g_btl_member_boards[i], BTL_OBJ_HIDDEN);
        BtlObjFree(BtlObjLast(g_btl_member_boards[i]));
        y += BOARD_STEP;
        g_btl_obj_prev->attached = 0;
        i++;
    } while (i < MEMBER_BOARDS);
}
#else
INCLUDE_ASM("btlp/nonmatchings/memberboards", BtlOpenMemberBoards);
#endif

void BtlCloseMemberBoards(void)
{
    int i;

    i = 0;
    do {
        BtlBoardShut(g_btl_member_boards[i]);
        i++;
    } while (i < MEMBER_BOARDS);
}
