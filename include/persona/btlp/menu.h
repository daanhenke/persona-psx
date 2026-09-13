/* Persona 1 (JP) - the battle's choice box.
 *
 * The box holds up to a handful of entries, each one an eight-byte cell: the
 * text and where in the box it goes. BtlMenuOpen2 and its siblings fill the
 * cells in and upload the glyphs; BtlMenuUpdate runs the box for a frame and
 * leaves the answer in g_btl_menu_choice.
 */
#ifndef PERSONA_BTLP_MENU_H
#define PERSONA_BTLP_MENU_H

#include <decomp/types.h>
#include <persona/btlp/object.h>

/* One entry: the text, and where it goes. */
typedef struct {
    /* 0x0 */ const u_char *text;
    /* 0x4 */ short         x;    /* in cells, across the box */
    /* 0x6 */ u_short       y;    /* in pixels, down from its top */
} BtlMenuCell;                    /* 8 bytes */

/* g_btl_menu_state. The box is closed on 0, resting on 1, taking the pad on 2,
   and 3 to 5 are the three slides - in, out of the way, and away. */
#define BTL_MENU_SHUT  0
#define BTL_MENU_IDLE  1
#define BTL_MENU_LIVE  2
#define BTL_MENU_SLIDE_IN   3
#define BTL_MENU_SLIDE_ASIDE 4
#define BTL_MENU_SLIDE_OUT  5

/* g_btl_menu_choice while the player has not answered, and the same answer
   from a picker. */
#define BTL_MENU_WAIT (-0x100)
#define BTL_PICK_WAIT (-0x100)

/* Where a menu item's cursor goes, relative to the board it is on. */
typedef struct {
    short x;
    short y;
} BtlMenuSpot;

/* The cells the menu cursor is drawn with, which each menu places. */
extern BtlGfxCell g_btl_menu_cursor[];

/* The two menus whose cursor walks a table of neighbours: the debug board's,
   and the placement menu's layout picker. Both answer an item on a confirm,
   -1 on a cancel and BTL_PICK_WAIT otherwise. menunav.c. */
extern int BtlDebugUpdate(void);
extern int BtlPresetMenuUpdate(int on_field);
/* The orders menu's picker, which answers -1 for either way out, and the
   tactics page, which answers 0 on a cancel and -2 on the abort key. */
extern int BtlOrdersMenuUpdate(void);
extern int BtlTacticsMenuUpdate(void);
/* The debug board's spell list, a page of ten at a time on `board`, which
   is set turning when the cursor steps off either end. Answers the spell
   on a confirm, -1 on a cancel. */
struct BtlObj;
extern int BtlSpellMenuUpdate(struct BtlObj *board);
/* Redraws the tactics page's rows from each member's BtlActor.tactic.
   menuboards.c. */
extern void BtlRefreshTacticsLines(void);

/* What a picker answers besides a slot: the second key backs out, the third
   abandons the whole thing. */
#define BTL_PICK_CANCEL (-1)
#define BTL_PICK_ABORT  (-2)

/* The click a picker makes when the cursor moves. */
#define PICK_SE_BANK 1
#define PICK_SE_MOVE 3

extern void BtlPartyResetGfx(void);
/* The same for one member, whatever state it is in. partyresetgfx.c. */
extern void BtlMemberResetGfx(int slot);

/* What each row of the command list runs, and the two that stand in it on
   their own: changing the Persona and refusing. Each answers 1 once the member
   has an order, 0 when the command came to nothing and -2 on the third key.
   commandpersona.c. */
extern int (*g_btl_command_fn[])(void);
extern int BtlCommandChangePersona(void);
extern int BtlCommandRefuse(void);

/* One frame of the Persona swap board. personamenu.c. */
extern int BtlPersonaSwapUpdate(short *row);

/* The list board the spell and item boards share: which of the two it is
   showing, the first spell slot on its page, and where the menu has got to
   in the inventory; the two builders that fill a page, and the steps to the
   next and previous usable item. objmotion6.c scrolls it. */
extern u_char   g_btl_list_open;
extern int      g_btl_spell_slot;
extern u_short *g_btl_item_at;

extern void     BtlBuildSpellLines(int spell);
extern void     BtlBuildItemLines(u_short *from);
extern u_short *BtlNextUsableItem(u_short *slot);
extern u_short *BtlPrevUsableItem(u_short *slot);

/* Lights one member at full colour with their marker chosen, and sends the
   rest of the living party toward the background with theirs. */
extern void BtlSingleOutMember(int slot);

extern BtlMenuCell g_btl_menu_cells[];

/* The three windows the choice box types its entries into, the row of the
   choice table the box was opened on, and the directory slot each of its
   three entries reads its text from. */
extern struct BtlWindow g_btl_choice_windows[];
extern u_short *g_btl_choice_at;
extern u_int    g_btl_choice_lines[];
extern int g_btl_menu_state;
extern int g_btl_menu_index;
extern int g_btl_menu_count;
extern int g_btl_menu_slide;
extern int g_btl_menu_slide_frames;
extern int g_btl_menu_choice;

extern void BtlMenuUpdate(void);

#endif
