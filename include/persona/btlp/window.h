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
    /* 0x004 */ u_char        pad004[8];
    /* 0x00C */ const u_char *script;
    /* 0x010 */ u_long        attr;    /* every cell takes a copy of this   */
    /* 0x014 */ int           timer;
    /* 0x018 */ u_char        pad018[4];
    /* 0x01C */ int           placed;  /* characters on screen              */
    /* 0x020 */ int           staged;  /* which staging cell is in use      */
    /* 0x024 */ u_char        pad024[2];
    /* 0x026 */ short         scroll;  /* moved up four pixels a frame      */
    /* 0x028 */ BtlWindowCell cells[BTL_WINDOW_CELLS];
    /* 0x268 */ u_char        pad268[4];
    /* 0x26C */ short         vram_x;  /* where its glyphs are uploaded     */
    /* 0x26E */ short         vram_y;
    /* 0x270 */ u_char        pad270[4];
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
