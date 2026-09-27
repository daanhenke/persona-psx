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
extern u_char   g_menu_state;
/* The pad this frame and last, and what went down this frame (active low
   in the buffer, so inverted). */
extern int      g_pad_now;
extern int      g_pad_old;
extern u_char   g_pad_buf0[4];
extern u_char   g_pad_buf1[4];
extern u_char   g_msg_box_shown;
extern int      g_active_buff;
extern u_char   g_packet[2][0x4000];
extern GsOT     g_ot[2];
extern GsLINE   g_lines[35];

/* One save as the list shows it: the head of the block CardScanSaves reads. */
typedef struct {
    /* 0x00 */ u_char name[8];
    /* 0x08 */ u_char level;
    /* 0x09 */ u_char hours;
    /* 0x0A */ u_char minutes;
    /* 0x0B */ u_char pad0B[0xF];
} OpenSaveEntry;                      /* 0x1A bytes */

extern OpenSaveEntry g_save_list[7];
extern int      g_card_scan;
extern int      g_card_status;
extern int      g_load_status;
extern u_char   g_save_slot;
extern u_char   g_save_chan;
extern u_short  g_txt_loading[];
extern u_short  g_txt_save_broken[];
extern u_short  g_txt_no_saves[];
extern u_short  g_txt_suspend_erase[];
extern void     OpenSaveListInit(int kind, int mask, int cursor);
extern int      CardLoad(u_char chan);
extern int      CardScanSaves(u_char chan, u_char kind, OpenSaveEntry *out);
extern int      CardCheckSave(u_char chan, u_char kind, u_char slot);
extern int      CardLoadSave(u_char chan, u_char slot, u_char kind);
extern int      CardDeleteFile(u_char chan, u_char slot, u_char kind);
extern POLY_G4  g_grad_poly;
extern int      g_pad_trig;
/* Per sprite: 0x80 draws it, 0x81 and 0x82 as well (FIXME: the difference). */
extern int      g_sprite_flags[];
extern int      g_text_len;
/* The frame behind a message. */
extern GsBOXF   g_msg_box;
extern int      g_msg_len;
/* The glyph being drawn: 16 rows of 16 4-bit pixels. */
extern u_long   g_font_glyph[16][2];
extern u_short  g_txt_no_card[];
extern u_short  g_txt_card_error[];
extern u_short  g_txt_unformatted[];
extern u_short  g_txt_load_header[];

extern void OpenPlaySeq(int i);
extern void OpenLoadTim(u_long *addr, int no_clut);
extern void OpenSpriteInit(u_short no, u_short w, u_short h, u_short tpage,
                          u_short u, u_short v, u_short cx, u_short cy);
extern void OpenConfirmOpen(void);
extern void OpenLoadMenuReset(int arg0);
extern void OpenLoadMenuSprites(int arg0);
extern void OpenConfirmSprites(void);
extern void OpenFontPutText(int row, u_char *text);
extern void OpenFontPutNumber(int row, int value, int col);
extern void OpenSpriteSetUV();
extern void OpenDrawMenu(void);
extern void OpenMessageOpen(short w, short h, short x, short y, u_short *text);
extern int  CardPollPorts(int want0, int want1);
extern void OpenFontLoadText(short w, short h, short x, short y, u_short *text);
extern void OpenFontRenderGlyph();

#endif
