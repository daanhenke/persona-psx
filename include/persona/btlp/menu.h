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

/* What a picker answers besides a slot: the second key backs out, the third
   abandons the whole thing. */
#define BTL_PICK_CANCEL (-1)
#define BTL_PICK_ABORT  (-2)

/* The click a picker makes when the cursor moves. */
#define PICK_SE_BANK 1
#define PICK_SE_MOVE 3

extern void BtlPartyResetGfx(void);

extern BtlMenuCell g_btl_menu_cells[];
extern int g_btl_menu_state;
extern int g_btl_menu_index;
extern int g_btl_menu_count;
extern int g_btl_menu_slide;
extern int g_btl_menu_slide_frames;
extern int g_btl_menu_choice;

extern void BtlMenuUpdate(void);

#endif
