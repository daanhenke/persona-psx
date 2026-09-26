#ifndef PERSONA_END_END_H
#define PERSONA_END_END_H

/* Persona 1 (JP) - END.EXE, the ending.
 *
 * The resident runs it with the ending to show in g_movie_id (0x30 up); it
 * plays that ending's credits script: credit images streamed in from
 * END.BIN, the ending's music, and its movies from \STR<bank>\MV<nn>.STR.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/common/str.h>

/* ---- data --------------------------------------------------------------- */

/* Where each credit image, song and movie starts in END.BIN, in sectors from
   the start of the file; each runs up to the next entry. */
extern u_short g_end_sectors[];
/* How many frames each movie runs. */
extern u_short g_end_movie_frames[];
/* The credits script of each ending. */
extern u_short *g_end_scripts[];
extern u_short  g_end_script1[];
extern u_short  g_end_script3[];
/* The layout each credit image uses, and where each layout's logo goes. */
extern u_char  g_end_image_kinds[];
extern u_short g_end_kind_x[];
extern u_short g_end_kind_y[];
/* The part of the image each logo shows. */
extern u_char  g_end_logo_u[];
extern u_char  g_end_logo_v[];
extern u_char  g_end_logo_w[];
extern u_char  g_end_logo_h[];

/* ---- gp small data; each unit keeps its own tentative definition --------- */

extern int      g_end_pad;         /* the pad this frame, -1 until read */
extern int      g_end_pad_old;
extern int      g_end_pad_trig;    /* buttons pressed this frame */
extern int      g_end_show_sprite;
extern int      g_end_movie_frame;
extern int      g_end_buf;         /* the ordering table being built */
extern short    g_end_vab;
extern short    g_end_seq;
extern u_short *g_end_script;
extern u_char   g_end_image_kind;
extern int      g_end_base;        /* END.BIN's first sector */
extern int      g_end_page_a;
extern int      g_end_movie;       /* the movie playing */
extern int      g_end_page_b;
extern int      g_end_page_c;
extern int      g_end_movie_done;  /* raised within three frames of its end */

/* ---- bss ---------------------------------------------------------------- */

extern u_long   g_end_seq_table[];
extern PACKET   g_end_packets[];
extern GsOT_TAG g_end_ot_tags[2][16];
extern GsSPRITE g_end_sprite;
extern GsSPRITE g_end_spr_a[2][2];
extern GsSPRITE g_end_spr_b[2];
extern GsSPRITE g_end_spr_c[2][2];
extern CdlFILE  g_end_file;
extern GsOT     g_end_ot[2];
extern GsIMAGE  g_end_image;

/* The resident's work area. */
extern u_short g_movie_id;
extern int     g_movie_next_state;
extern u_char  g_options[];

/* ---- code --------------------------------------------------------------- */

extern void EndInit(void);
extern void EndLoadImage(void);
extern void EndLoadTim(u_long *tim, int noclut, int x, int y);
/* Defined old-style: callers pass the width unnarrowed and it is masked
   where it is used. */
extern void EndSetSprite();
extern void EndDraw(void);
extern int  EndPlayMovie(void);

extern void StrPutHex(short value, char *p, short n);

extern void LoadFileToAddrAsync(char *name, u_long *dest);
extern void LoadFileToAddr(char *name, u_long *dest);
extern void CdReadFileToAddrAsync(CdlLOC *pos, int sectors, u_long *dest);
extern void CdReadFileToAddr(CdlLOC *pos, int sectors, u_long *dest);
extern volatile int g_cd_busy;

#endif
