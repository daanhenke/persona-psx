/* Persona 1 (JP) - S2D's text windows (ui/textwin.c). */
#ifndef PERSONA_S2D_TEXTWIN_H
#define PERSONA_S2D_TEXTWIN_H

#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>

/* A window: its script, the text laid out in it, the transform it is drawn
   through and the two draw areas, one per buffer, it clips to. */
typedef struct {
    /* 0x000 */ short   mode;
    /* 0x002 */ u_short flags;
    /* 0x004 */ u_short unk04;
    /* 0x006 */ short   unk06;
    /* 0x008 */ int     unk08;
    /* 0x00C */ int     script;
    /* 0x010 */ short   unk10;
    /* 0x012 */ short   count;     /* characters placed */
    /* 0x014 */ short   unk14;
    /* 0x016 */ short   unk16;
    /* 0x018 */ short   scroll;    /* rows scrolled */
    /* 0x01A */ short   scroll_wait;
    /* 0x01C */ u_char  unk1C[0x19C - 0x1C];
    /* 0x19C */ VECTOR  trans;
    /* 0x1AC */ SVECTOR rot;
    /* 0x1B4 */ VECTOR  scale;
    /* 0x1C4 */ short   unk1C4;
    /* 0x1C6 */ short   unk1C6;
    /* 0x1C8 */ short   unk1C8;
    /* 0x1CA */ short   unk1CA;
    /* 0x1CC */ short   x;
    /* 0x1CE */ short   y;
    /* 0x1D0 */ short   x0;
    /* 0x1D2 */ short   y0;
    /* 0x1D4 */ short   ofs_x;
    /* 0x1D6 */ short   ofs_y;
    /* 0x1D8 */ short   cols;
    /* 0x1DA */ short   rows;
    /* 0x1DC */ u_char  unk1DC[0x20C - 0x1DC];
    /* 0x20C */ DR_AREA area[2];
    /* 0x224 */ DR_AREA text_area[2];
} S2dWin;                               /* 0x23C */

/* Mode bit: the window is not offset sideways by its width. */
#define WIN_NO_XOFS 4

/* The end of a string the windows lay out. */
#define TEXT_END 0xFF01

#endif
