#ifndef LIBGS_H
#define LIBGS_H

#include <types.h>
#include <libgte.h>

/* A coordinate system. GsInitCoordinate2 starts one at GsIDMATRIX, and the
   routines that place an object write its rotation and translation into
   `coord` and clear `flg` so libgs recomputes the composed matrix. */
typedef struct GsCOORDINATE2 {
    /* 0x00 */ u_long                flg;
    /* 0x04 */ MATRIX                coord;
    /* 0x24 */ MATRIX                workm;
    /* 0x44 */ SVECTOR              *rotate;
    /* 0x48 */ struct GsCOORDINATE2 *super;
    /* 0x4C */ struct GsCOORDINATE2 *sub;
} GsCOORDINATE2;                    /* 0x50 bytes */

extern MATRIX GsIDMATRIX;


/* Ordering table. GsSortSprite takes the entry for the frame being built; the
   overlays keep a two-element array and index it with the buffer number, which
   is visible in the asm as a stride of 5 words. */
typedef struct {
    unsigned p : 24;
    unsigned num : 8;
} GsOT_TAG;

typedef struct {
    /* 0x00 */ u_long    length;
    /* 0x04 */ GsOT_TAG *org;
    /* 0x08 */ u_long    offset;
    /* 0x0C */ u_long    point;
    /* 0x10 */ GsOT_TAG *tag;
} GsOT;                         /* 0x14 bytes */

/* attribute bits used by the game; the rest are libgs's own. */
#define GsSPRITE_SEMITRANS 0x40000000
#define GsSPRITE_NODISPLAY 0x80000000

typedef struct {
    /* 0x00 */ u_long attribute;
    /* 0x04 */ short  x, y;
    /* 0x08 */ u_short w, h;
    /* 0x0C */ u_short tpage;
    /* 0x0E */ u_char  u, v;
    /* 0x10 */ u_short cx, cy;
    /* 0x14 */ u_char  r, g, b, pad;
    /* 0x18 */ short  mx, my;
    /* 0x1C */ short  scalex, scaley;
    /* 0x20 */ long   rotate;
} GsSPRITE;                     /* 0x24 bytes */

/* Tiled background. A GsMAP is cellw x cellh pixels per cell, ncellw x ncellh
   cells, `base` the cell definitions and `index` the map of which cell goes
   where. The game keeps one 15x4 map of 16x16 cells per overlay. */
typedef struct {
    /* 0x00 */ u_char  u, v;
    /* 0x02 */ u_short cba;
    /* 0x04 */ u_short flag;
    /* 0x06 */ u_short tpage;
} GsCELL;                       /* 8 bytes */

typedef struct {
    /* 0x00 */ u_char   cellw, cellh;
    /* 0x02 */ u_short  ncellw;
    /* 0x04 */ u_short  ncellh;
    /* 0x08 */ GsCELL  *base;
    /* 0x0C */ u_short *index;
} GsMAP;                        /* 0x10 bytes */

/* One TIM as GsGetTimInfo unpacks it. */
typedef struct {
    /* 0x00 */ u_long  pmode;   /* bit 3 says the TIM carries a CLUT */
    /* 0x04 */ short   px, py;
    /* 0x08 */ short   pw, ph;
    /* 0x0C */ u_long *pixel;
    /* 0x10 */ short   cx, cy;
    /* 0x14 */ short   cw, ch;
    /* 0x18 */ u_long *clut;
} GsIMAGE;                      /* 0x1C bytes */

typedef struct {
    /* 0x00 */ u_long  attribute;
    /* 0x04 */ short   x, y;
    /* 0x08 */ short   w, h;
    /* 0x0C */ short   scrollx, scrolly;
    /* 0x10 */ u_char  r, g, b, pad;
    /* 0x14 */ GsMAP  *map;
    /* 0x18 */ short   mx, my;
    /* 0x1C */ short   scalex, scaley;
    /* 0x20 */ long    rotate;
} GsBG;                         /* 0x24 bytes */

void GsClearVcount(void);
long GsGetVcount();
void GsInitVcount();
void GsSortSprite(GsSPRITE *sp, GsOT *ot, u_short pri);

#endif
