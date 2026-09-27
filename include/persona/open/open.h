#ifndef PERSONA_OPEN_OPEN_H
#define PERSONA_OPEN_OPEN_H

/* Persona 1 (JP) - OPEN.EXE declarations shared by its units. */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

extern int  func_80081018(int movie);
extern void func_80086D14(void);
extern void func_80086E7C(void);
extern void func_80086F34(void);
extern int  func_80088D3C(void);

/* The title's sprites: every image and every font cell it draws. */
extern GsSPRITE g_sprites[];

extern short    g_open_seq[];
extern u_char   D_800B410C;
extern int      D_800B4094;
extern int      D_800B409C;
extern int      D_8011F390[];
extern u_char   D_800A0A90[];

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
extern void func_80085D80(int a0, int a1, int a2, int a3, u_char *text);

#endif
