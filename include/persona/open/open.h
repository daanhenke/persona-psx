#ifndef PERSONA_OPEN_OPEN_H
#define PERSONA_OPEN_OPEN_H

/* Persona 1 (JP) - OPEN.EXE declarations shared by its units. */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

extern int  func_80081018(int movie);
extern void CardOpenEvents(void);
extern void CardCloseEvents(void);
extern void CardEnableEvents(void);
extern int  OpenPlayMovie(void);

/* The title's sprites: every image and every font cell it draws. */
extern GsSPRITE g_sprites[];

extern short    g_open_seq[];
/* Frames of the opening movie shown so far; it ends at 0x825. */
extern int      g_movie_frame;
extern u_char   D_800B410C;
/* The pad this frame and last, and what went down this frame (active low
   in the buffer, so inverted). */
extern int      g_pad_now;
extern int      g_pad_old;
extern u_char   g_pad_buf0[4];
extern u_char   g_pad_buf1[4];
extern u_char   D_800B40F8;
extern int      g_active_buff;
extern u_char   g_packet[2][0x4000];
extern GsOT     g_ot[2];
extern int      g_pad_trig;
/* Per sprite: 0x80 draws it, 0x81 and 0x82 as well (FIXME: the difference). */
extern int      g_sprite_flags[];
extern int      g_text_len;
/* The frame behind a message. */
extern GsBOXF   g_msg_box;
extern int      g_msg_len;
/* The glyph being drawn: 16 rows of 16 4-bit pixels. */
extern u_long   g_font_glyph[16][2];
extern u_short  D_800A0C40[];
extern u_short  D_800A0C5C[];
extern u_short  D_800A0C74[];
extern u_short  D_800A0C10[];

extern void OpenPlaySeq(int i);
extern void OpenLoadTim(u_long *addr, int no_clut);
extern void OpenSpriteInit(u_short no, u_short w, u_short h, u_short tpage,
                          u_short u, u_short v, u_short cx, u_short cy);
extern void func_80084288(void);
extern void func_800842D0(int arg0);
extern void func_80084B40(int arg0);
extern void func_80085798(void);
extern void OpenFontPutText(int row, u_char *text);
extern void OpenFontPutNumber(int row, int value, int col);
extern void OpenSpriteSetUV();
extern void func_800843A0(void);
extern void OpenMessageOpen(short w, short h, short x, short y, u_short *text);
extern int  CardPollPorts(int want0, int want1);
extern void OpenFontLoadText(short w, short h, short x, short y, u_short *text);
extern void OpenFontRenderGlyph(u_short code);

#endif
