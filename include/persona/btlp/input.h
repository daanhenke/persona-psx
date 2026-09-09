#ifndef PERSONA_BTLP_INPUT_H
#define PERSONA_BTLP_INPUT_H

/* Persona 1 (JP) - what the battle reads from the pad, and the cursor it
 * moves with it.
 *
 * Three things arrive from the pad and they are not interchangeable.
 * g_btl_pad1 is the raw held state, refreshed once a frame. g_btl_pad1_edge is
 * what was newly pressed, and it is consumed - code that acts on a press
 * clears it so the next reader does not see it again. g_btl_input is what
 * BtlPadRepeat leaves behind: the held state on the frame of the press and
 * every third frame after the hold counter sticks, and otherwise only the new
 * presses with the directions masked out. Menus read that one, through
 * BtlInputKeys.
 *
 * The button masks are variables rather than constants because the control
 * scheme owns them, and the four directions are the part no scheme changes.
 *
 * The cursor is a textured quad, not an arrow. BtlCursorPlace writes its four
 * corners around a point and BtlCursorDraw copies whichever of the two
 * prepared quads is current into the primitive buffer - alternating them is
 * what makes it blink. Hiding it is bit 0 of g_btl_cursor_flags, not a move
 * off screen, so BtlCursorFlags is enough to save and restore its state.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>

/* The pad. */
extern u_short g_btl_pad1;      /* held this frame                          */
extern u_short g_btl_pad1_edge; /* newly pressed, and cleared by whoever acts */
extern u_long  g_btl_input;     /* what BtlPadRepeat leaves for the menus    */

/* BtlPadRepeat's counters: frames a direction has been held, capped at
   BTL_HOLD_MAX, and the countdown between repeats once it is capped. */
extern int g_btl_hold_frames;
extern int g_btl_repeat_delay;

#define BTL_HOLD_MAX     0x1E
#define BTL_REPEAT_DELAY 3

/* The control scheme's masks. */
extern u_short g_btl_key_up;
extern u_short g_btl_key_down;
extern u_short g_btl_key_left;
extern u_short g_btl_key_right;
extern u_short g_btl_key_confirm;
extern u_short g_btl_key_cancel;
extern u_short g_btl_key_abort;

/* The two sideways bits BtlMenuKey hands back, which are the pad's own rather
   than the control scheme's. */
#define PAD_LEFT  0x8000
#define PAD_RIGHT 0x2000

extern int BtlMenuKey(void);

/* Read twice rather than kept in a local: hoisting them costs the match. */
#define BTL_DIRECTIONS                                                       \
    (g_btl_key_up | g_btl_key_down | g_btl_key_left | g_btl_key_right)

/* The cursor. */
extern u_char   g_btl_cursor_flags;
extern u_char   g_btl_cursor_buf;   /* which of the pair is current         */
extern u_char   g_btl_cursor_anim;
extern u_char   g_btl_cursor_cel;
extern u_char   g_btl_cursor_timer;
extern POLY_FT4 g_btl_cursor_prims[];

/* One row per cursor animation. The cels run left to right from the corner
   below, so a cel's texture x is the corner plus its number times the cell. */
typedef struct {
    /* 0x0 */ u_char u;       /* texture corner of cel nought */
    /* 0x1 */ u_char v;
    /* 0x2 */ u_char cels;    /* how many cels it runs through */
    /* 0x3 */ u_char frames;  /* how long each cel is held for */
} BtlCursorAnim;              /* 4 bytes */

extern BtlCursorAnim g_btl_cursor_anims[];

#define BTL_CURSOR_VISIBLE 1

extern void   BtlPadRepeat(void);
extern u_long BtlInputKeys(void);
extern void   BtlInputClear(void);
extern u_char BtlCursorFlags(void);
extern void   BtlCursorShow(int on);
extern void   BtlCursorInitPrims(void);
extern void   BtlCursorDraw(void);
extern int    BtlCursorNext(int slot);
extern int    BtlCursorPrev(int slot);

extern void BtlCursorPlace(short x, short y);

#endif
