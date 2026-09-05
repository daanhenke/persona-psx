#ifndef LIBGS_H
#define LIBGS_H

#include <types.h>

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

void GsClearVcount(void);
long GsGetVcount();
void GsInitVcount();
void GsSortSprite(GsSPRITE *sp, GsOT *ot, u_short pri);

#endif
