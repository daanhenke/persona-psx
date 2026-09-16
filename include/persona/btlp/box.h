/* Persona 1 (JP) - the battle's message box.
 *
 * A row of textured quads transformed by the GTE, so the box's open and close
 * animations are moves of the scale its matrix is built from. 0x1000 is full
 * size on an axis and zero is nothing.
 */
#ifndef PERSONA_BTLP_BOX_H
#define PERSONA_BTLP_BOX_H

#include <decomp/types.h>
#include <libgte.h>

/* g_btl_box_flags */
#define BTL_BOX_LOADED      0x0020  /* the graphics are in VRAM             */
#define BTL_BOX_SLIDE       0x2000  /* close by sliding shut rather than    */
                                    /* collapsing                           */
#define BTL_BOX_FRAME       0x8000  /* draw the frame at all                */
#define BTL_BOX_STYLE       0x00C0
#define BTL_BOX_STYLE_SHIFT 6

/* g_btl_box_step - which step of the animation BtlBoxTick is on. */
#define BTL_BOX_ZOOM         1  /* grow wide, then tall                     */
#define BTL_BOX_OPEN         2  /* full width, then ramp the height         */
#define BTL_BOX_OPEN_NOW     3  /* straight to full                         */
#define BTL_BOX_CLOSE_NOW    4  /* straight to nothing                      */
#define BTL_BOX_CLOSE        5  /* ramp the height back down                */
#define BTL_BOX_COLLAPSE_STEP 6 /* halve the height, then the width         */
#define BTL_BOX_HOLD      0x20  /* count g_btl_box_hold down, then idle     */

/* There is no frame tile for a box outside this. */
#define BTL_BOX_COLS_MIN 3
#define BTL_BOX_COLS_MAX 17

/* Scratch the graphics are unpacked into: the palette first, the frame's tiles
   0x200 bytes in. Reached by hardcoded address rather than through a symbol. */
#define BTL_BOX_CLUT  ((u_char *)0x8014AA00)
#define BTL_BOX_TILES ((u_long *)0x8014AC00)

/* Where each lands in VRAM. The tiles share the message windows' page column,
   so they move with it. */
#define BTL_BOX_PAGE0    11
#define BTL_BOX_PAGE_W   3
#define BTL_BOX_COL      0x40
#define BTL_BOX_TILES_Y  0x188
#define BTL_BOX_TILES_W  0x2C
#define BTL_BOX_TILES_H  0x20
#define BTL_BOX_CLUT_Y   0x1FB
#define BTL_BOX_CLUT_W   0x100

/* Where the box starts from, and how far away it sits. */
#define BTL_BOX_START_X 0x10
#define BTL_BOX_START_Y 0x40
#define BTL_BOX_DIST    100

/* Full size on an axis, how far a ramp step moves, and the sliver the
   collapse stops the height at. */
#define BTL_BOX_FULL 0x1000
#define BTL_BOX_RAMP 0x180
#define BTL_BOX_THIN 0x40

/* The packed graphics, and the three values the box's matrix is built from.
   They sit back to back in this order, which is how the text window's drawer
   reaches all three off the scale's own address. */
extern u_char *g_btl_box_pack;
extern VECTOR  g_btl_box_pos;
extern SVECTOR g_btl_box_rot;
extern VECTOR  g_btl_box_scale;

extern u_short g_btl_box_flags;
extern u_char  g_btl_box_step;
extern u_char  g_btl_box_hold;
extern short   g_btl_box_cols;
extern short   g_btl_box_ox;
extern short   g_btl_box_oy;

/* g_btl_box_step, handed back as a byte. A stage waits on this rather than
   on the variable so it does not have to know what the box is doing. */
extern char BtlBoxState(void);
extern void BtlBoxOpen(short cols, short x, short y, int style);

#endif
