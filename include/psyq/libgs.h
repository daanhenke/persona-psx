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

typedef struct {
    /* 0x00 */ u_long attribute;
    /* 0x04 */ short  x0, y0;
    /* 0x08 */ short  x1, y1;
    /* 0x0C */ u_char r, g, b, pad;
} GsLINE;                       /* 0x10 bytes */

/* The gouraud line carries a colour at each end. The three per-channel screen
   fades in ADV are what fix the layout: they write the same value to 0x0C and
   0x0F, 0x0D and 0x10, or 0x0E and 0x11, one pair each, so the two ends' three
   channels are 0x0C..0x0E and 0x0F..0x11 with nothing in between. */
typedef struct {
    /* 0x00 */ u_long attribute;
    /* 0x04 */ short  x0, y0;
    /* 0x08 */ short  x1, y1;
    /* 0x0C */ u_char r0, g0, b0;
    /* 0x0F */ u_char r1, g1, b1;
    /* 0x12 */ u_char code, pad;
} GsGLINE;                      /* 0x14 bytes */

/* A filled box, sorted straight into the ordering table. */
typedef struct {
    /* 0x00 */ u_long attribute;
    /* 0x04 */ short  x, y;
    /* 0x08 */ short  w, h;
    /* 0x0C */ u_char r, g, b, pad;
} GsBOXF;                       /* 0x10 bytes */

/* The 3D side of libgs. The game barely uses it - GsSortObject4 is called
   from nine places and GsLinkObject4 from one - so these layouts are the
   documented Psy-Q ones rather than anything read back out of this binary.
   Check the field offsets against the asm before relying on one. */
typedef struct {
    /* 0x00 */ VECTOR  scale;
    /* 0x10 */ SVECTOR rotate;
    /* 0x18 */ VECTOR  trans;
} GsCOORD2PARAM;

typedef struct {
    /* 0x00 */ u_long         attribute;
    /* 0x04 */ GsCOORDINATE2 *coord2;
    /* 0x08 */ u_long        *tmd;
    /* 0x0C */ u_long         id;
} GsDOBJ2;                      /* 0x10 bytes */

/* A parallel light source: a direction and a colour. */
typedef struct {
    /* 0x00 */ long   vx, vy, vz;
    /* 0x0C */ u_char r, g, b, pad;
} GsF_LIGHT;                    /* 0x10 bytes */

typedef struct {
    /* 0x00 */ long   dqa;
    /* 0x04 */ long   dqb;
    /* 0x08 */ u_char rfc, gfc, bfc, pad;
} GsFOGPARAM;                   /* 0x0C bytes */

/* Viewpoint, reference point and twist, in the coordinate system `super`. */
typedef struct {
    /* 0x00 */ long           vpx, vpy, vpz;
    /* 0x0C */ long           vrx, vry, vrz;
    /* 0x18 */ long           rz;
    /* 0x1C */ GsCOORDINATE2 *super;
} GsRVIEW2;                     /* 0x20 bytes */

void GsClearVcount(void);
long GsGetVcount();
void GsInitVcount();

void GsInitGraph(u_short x, u_short y, u_short intmode, u_short dith,
                 u_short varmmode);
void GsInitGraph2(u_short x, u_short y, u_short intmode, u_short dith,
                  u_short varmmode);
void GsInit3D(void);
void GsSwapDispBuff(void);
int  GsGetActiveBuff(void);

void GsClearOt(u_short offset, u_short point, GsOT *otp);
void GsSortClear(u_char r, u_char g, u_char b, GsOT *otp);
void GsDrawOt(GsOT *otp);
void GsSortOt(GsOT *ot_src, GsOT *ot_dest);

void GsSortSprite(GsSPRITE *sp, GsOT *ot, u_short pri);
void GsSortFastSprite(GsSPRITE *sp, GsOT *ot, u_short pri);
void GsSortLine(GsLINE *line, GsOT *ot, u_short pri);
void GsSortGLine(GsGLINE *line, GsOT *ot, u_short pri);
void GsSortBg(GsBG *bg, GsOT *otp);
void GsSortFastBg(GsBG *bg, GsOT *otp);
void GsSortBoxFill(GsBOXF *boxf, GsOT *otp, u_short pri);
void GsSortObject4(GsDOBJ2 *objp, GsOT *otp);
void GsLinkObject4(u_long objnum, u_long *base, GsDOBJ2 *objp);
u_long *GsMapModelingData(u_long *base);

void GsSetAmbient(long r, long g, long b);
void GsSetLightMode(int mode);
void GsSetFlatLight(int id, GsF_LIGHT *light);
void GsSetFogParam(GsFOGPARAM *fogp);
void GsSetLightMatrix(MATRIX *mp);
void GsSetLightMatrix2(MATRIX *mp);
void GsSetLsMatrix(MATRIX *mp);
void GsSetRefView2(GsRVIEW2 *pv);

void GsInitCoordinate2(GsCOORDINATE2 *super, GsCOORDINATE2 *coord);
void GsGetLs(GsCOORDINATE2 *coord, MATRIX *m);
void GsGetLw(GsCOORDINATE2 *coord, MATRIX *m);
void GsGetLws(GsCOORDINATE2 *coord, MATRIX *lw, MATRIX *ls);
void GsGetTimInfo(u_long *tim, GsIMAGE *img);

#endif
