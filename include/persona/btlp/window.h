/* Persona 1 (JP) - the battle's message windows.
 *
 * Two of them, both records of this shape, and both stepped a frame at a time
 * through BtlWindowStep - the sequencer owns one and the second is the other.
 * The state at +2 is what a caller waits on, and BtlSeqReset clears the rest.
 *
 * Text arrives one character at a time. Each is expanded into a staging cell,
 * queued for VRAM, and given a place in the window's own cell array, fifteen
 * to a row; `placed` counts the characters on screen and `staged` the staging
 * cell in use, which is why they are two separate numbers.
 *
 * Only the fields the code named so far uses are spelt out.
 */
#ifndef PERSONA_BTLP_WINDOW_H
#define PERSONA_BTLP_WINDOW_H

#include <types.h>

typedef struct {
    /* 0x0 */ short  x;
    /* 0x2 */ short  y;
    /* 0x4 */ u_char pad4[4];
    /* 0x8 */ u_long attr;      /* the window's, copied in as each is placed */
} BtlWindowCell;                /* 0xC bytes */

#define BTL_WINDOW_CELLS 48

typedef struct {
    /* 0x000 */ u_char        pad000[2];
    /* 0x002 */ short         state;
    /* 0x004 */ int           answer;  /* left by the script; the sequencer's
                                          is read when it reaches state 10  */
    /* 0x008 */ const u_char *script;  /* how far the script walk has got   */
    /* 0x00C */ const u_char *text;    /* how far the typing has got        */
    /* 0x010 */ u_long        attr;    /* every cell takes a copy of this   */
    /* 0x014 */ int           timer;
    /* 0x018 */ int           unused;  /* cleared with the rest; nothing in
                                          the battle overlay reads it       */
    /* 0x01C */ int           placed;  /* characters on screen              */
    /* 0x020 */ int           staged;  /* which staging cell is in use      */
    /* 0x024 */ short         x;       /* where the window sits             */
    /* 0x026 */ short         y;       /* moved up four pixels a frame      */
    /* 0x028 */ BtlWindowCell cells[BTL_WINDOW_CELLS];
    /* 0x268 */ short         dx;      /* nudge, used to centre a message   */
    /* 0x26A */ short         dy;
    /* 0x26C */ short         vram_x;  /* where its glyphs are uploaded     */
    /* 0x26E */ short         vram_y;
    /* 0x270 */ int           slide;   /* the slower scroll, two a frame    */
} BtlWindow;                            /* 0x274 bytes */

/* Characters to a row, and the size of one in VRAM. */
#define BTL_GLYPH_ROW 15
#define BTL_GLYPH_W   4
#define BTL_GLYPH_H   0x10

/* A glyph expands from 0x20 bytes of 1bpp to 0x80 bytes of 4bpp. */
#define BTL_GLYPH_STRIDE 0x80

extern u_char g_btl_glyph_cells[];
extern int    g_btl_glyph_next;

#endif
