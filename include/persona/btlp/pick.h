/* Persona 1 (JP) - the six-slot command picker.
 *
 * Each slot is a pair of objects. BtlPickSpawn makes the per-slot piece first
 * and the shared frame second, and it is the frame that ends up in
 * g_btl_pick_objs - so the per-slot piece is reached through its attached
 * link, and a setter called on the frame reaches both.
 *
 * The picker has several pages of six commands. g_btl_pick_page says which is
 * up, g_btl_pick_live says which of that page's slots may be chosen, and the
 * help line under the menu is that page's row of text.
 */
#ifndef PERSONA_BTLP_PICK_H
#define PERSONA_BTLP_PICK_H

#include <decomp/types.h>
#include <persona/btlp/object.h>

/* Slots to a page. */
#define BTL_PICK_SLOTS 6

/* How bright a slot is drawn, live or not, and how fast it gets there. */
#define PICK_DARK 0x20
#define PICK_LIVE 0x80
#define PICK_FADE 0xFF

/* Where the help line is put. */
#define PICK_HELP_X 0x10
#define PICK_HELP_Y 0x94

extern BtlObj      *g_btl_pick_objs[];
extern const u_char g_btl_pick_live[][BTL_PICK_SLOTS];
extern u_char       g_btl_pick_page;
extern const char  *g_btl_pick_help[];
extern short        g_btl_pick_help_row;
extern short        g_btl_pick_help_row2;
extern u_char       g_btl_no_help;

extern void BtlPickSpawn(void);
extern void BtlPickRefresh(void);
/* The grid the pick cursors stand on, put up and taken down. pickgrid.c. */
extern void BtlSpawnPickGrid(void);
extern void BtlDespawnPickGrid(void);

/* Puts every pick cursor back on its fighter. pickgrid.c. */
extern void BtlRefreshPickCursors(void);
extern int  BtlPickUpdate(short *row);
extern void BtlPickHighlight(int chosen);
extern int  BtlPickShowPage(int page);

/* Puts the picker back where it stands between choices. */
extern void BtlPickSettle(void);

/* Runs the cursor over the nine enemy slots. The answer is the slot on a
   confirm, -1 on a cancel and BTL_PICK_WAIT while nothing has been decided;
   the slot the cursor is on is left in *slot either way. */
extern int BtlPickEnemy(short *slot);
extern int BtlPickEnemyLit(short *slot);

/* The enemy that cursor is on, which the analysis view reads as well. */
extern short g_btl_enemy_slot;

/* The same pick over the party, and the slot it leaves its cursor on.
   pickmember.c. */
extern int   BtlPickMember(short *slot);
extern short g_btl_target_slot;

/* The pickers the item command runs - a member with a cursor, the whole party
   and the enemy side - and the one a move is aimed with, which the cast and
   most items use. Each answers -2, -1 or the pick. targetpick.c, and the last
   in asm. */
struct BtlActor;
extern int BtlPickTargetMember(struct BtlActor *a);
extern int BtlPickTargetParty(struct BtlActor *a);
extern int BtlPickTargetEnemies(struct BtlActor *a);
extern int BtlPickMoveTarget(struct BtlActor *a, int move);

/* How the fighters a pick can reach are drawn: lit and set moving on
   BTL_TARGET_MOTION - the one under the cursor on the motion after it - and the
   rest dimmed to BTL_TARGET_DIM, walking there at BTL_TARGET_FADE. */
#define BTL_TARGET_LIT           0x80
#define BTL_TARGET_MOTION        10
#define BTL_TARGET_MOTION_CURSOR 11
#define BTL_TARGET_DIM           0x20
#define BTL_TARGET_FADE          8

/* The fallen-member picker a revival runs, the abort the move pick leaves for
   the command that ran it, and the party's pick cursors stood on their
   members' squares. pickdown.c, pickmove.c and pickplace.c. */
extern int     BtlPickDownMember(short *slot);
extern int     g_btl_pick_abort;
extern BtlObj *g_btl_pick_cursors[];
extern void    BtlPlacePickCursors(void);

#endif
