/* Persona 1 (JP) - the battle's message box.
 *
 * A row of textured quads transformed by the GTE, so the box's open and close
 * animations are moves of the scale its matrix is built from. 0x1000 is full
 * size on an axis and zero is nothing.
 */
#ifndef PERSONA_BTLP_BOX_H
#define PERSONA_BTLP_BOX_H

#include <types.h>

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

extern u_short g_btl_box_flags;
extern u_char  g_btl_box_step;
extern u_char  g_btl_box_hold;
extern short   g_btl_box_cols;
extern short   g_btl_box_ox;
extern short   g_btl_box_oy;

#endif
