/* Persona 1 (JP) - the CASINO overlay's shared state.
 *
 * The casino runs one of five games, picked from the spot on the map the
 * player walked up to. Its own variables live in the overlay's bss; the
 * save data it reads and writes back is reached by address, as the other
 * overlays reach it.
 */
#ifndef PERSONA_CASINO_CASINO_H
#define PERSONA_CASINO_CASINO_H

#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>

/* g_casino_game: the game being played, or the way out. */
#define CASINO_GAME_LEAVE 31     /* fading out; leaves once the timer runs */
#define CASINO_GAME_DONE  0xEC   /* the main loop's signal to return       */

/* The save block's own words, by address. */
#define g_money2         (*(int *)0x801F2678)
#define g_items          ((u_short *)0x801F267C)
#define g_exp_carry      (*(int *)0x801F29AC)
#define g_29B0           (*(u_int *)0x801F29B0)
#define g_29B4           (*(u_int *)0x801F29B4)
#define g_playtime_hours ((u_char *)0x801F29BC)
#define D_801F29ED       (*(u_char *)0x801F29ED)
#define g_script_534C    (*(u_char *)0x801F534C)
#define g_bgm_ready      (*(short *)0x801F5358)
#define g_vab_id         ((short *)0x801F535C)
#define g_seq_handle     ((short *)0x801F537C)

extern int    D_801F1BDC;
extern u_char g_playtime_min;
extern u_char g_playtime_sec;
extern u_char g_playtime_frame;

/* Six one-bit flags the visit starts with all clear. */
typedef struct {
    u_int b0 : 1;
    u_int b1 : 1;
    u_int b2 : 1;
    u_int b3 : 1;
    u_int b4 : 1;
    u_int b5 : 1;
} CasinoFlags;

/* A cursor on a grid of w by h cells. The previous cell is kept for the
   redraw, and each axis either wraps or stops at its edges. */
typedef struct {
    s8     x;
    s8     y;
    s8     prev_x;
    s8     prev_y;
    s8     w;
    s8     h;
    u_char wrap_x;
    u_char wrap_y;
    u_char unk8;
    u_char unk9;
    u_char unkA;
    u_char unkB;
    u_char unkC;
    u_char unkD;
} CasinoCursor;

/* The pad's direction bits. */
#define PAD_UP    0x1000
#define PAD_RIGHT 0x2000
#define PAD_DOWN  0x4000
#define PAD_LEFT  0x8000

/* The other keys. */
#define PAD_L2       0x01
#define PAD_R2       0x02
#define PAD_L1       0x04
#define PAD_R1       0x08
#define PAD_TRIANGLE 0x10
#define PAD_CIRCLE   0x20
#define PAD_CROSS    0x40
#define PAD_SQUARE   0x80

/* The buffer the other one of a pair belongs to: the one not being drawn
   into this frame. */
#define CASINO_OTHER(a) (a)[!g_casino_buf]

/* The two draw buffers. */
typedef struct {
    DRAWENV *draw;
    DISPENV *disp;
    u_long  *ot;
} CasinoDB;

/* The sprites: a POLY_FT4 per sprite in each buffer, its depth in the
   ordering table, and whether it is drawn. Each game sets how many it
   uses in g_casino_sprite_count. */
#define CASINO_SPRITES 1000

typedef struct {
    POLY_FT4 *prim[2];
    short    *z;
    u_char   *on;
    short     n;
} CasinoSprites;

/* Images waiting for the next DrawSync to go up to VRAM. */
typedef struct {
    RECT    *rect;
    u_long **data;
    short    n;
} CasinoLoadQueue;

/* The other draw lists. Each keeps its arrays and its count once per
   buffer, and an entry is added to both, so it is still there when the
   other buffer is drawn. What some of them draw is not worked out yet. */
/* A sprite's four corners. */
typedef struct {
    short x0, y0, x1, y1, x2, y2, x3, y3;
} CasinoQuad;

/* Where a sprite's image sits in its texture page. */
typedef struct {
    short u, v, w, h;
} CasinoUV;

/* New corners for sprites, applied to each buffer's polygons as that
   buffer is next drawn. */
typedef struct {
    CasinoQuad *quad[2];
    short      *spr[2];
    short       n[2];
} CasinoQuadQueue;

/* New textures for sprites, applied the same way. */
typedef struct {
    CasinoUV *uv[2];
    u_short  *tpage[2];
    u_short  *clut[2];
    short    *spr[2];
    short     n[2];
} CasinoUVQueue;

/* Whole sprites: where, which part of which texture page, with which
   palette, applied the same way. */
typedef struct {
    RECT    *pos[2];
    RECT    *uv[2];
    u_short *tpage[2];
    u_short *clut[2];
    short   *spr[2];
    short    n[2];
} CasinoRectQueue;

/* Semi-transparency for runs of sprites, eight runs a frame, applied the
   same way. */
typedef struct {
    short  first[2][8];
    short  count[2][8];
    u_char mode[2][8];
    short  n[2];
} CasinoSemiQueue;

/* New palettes for sprites, applied the same way. */
typedef struct {
    u_short *clut[2];
    short   *spr[2];
    short    n[2];
} CasinoClutQueue;


/* Where a 3D object is: rotation, position and scale. */
typedef struct {
    SVECTOR rot;
    VECTOR  trans;
    VECTOR  scale;
} CasinoXform;

/* A flat face of a 3D object: four corners, the depth it takes when the
   object does not sort by distance, and whether it is shown. Face i is
   drawn as sprite first + i. */
typedef struct {
    SVECTOR v[4];
    u_short z;
    u_char  on;
    u_char  pad;
} CasinoFace;

typedef struct {
    u_char       busy;   /* an animation is moving it */
    u_char       flat;   /* the corners are already screen positions */
    u_char       zsort;  /* 1: depth from the GTE, else each face's own */
    u_char       unk3;
    short        first;
    short        n;
    CasinoFace  *face;
    CasinoXform *xform;
} CasinoObj;

/* How a face is built: a corner, a size, and which way round it faces. */
typedef struct {
    SVECTOR origin;
    short   w;
    short   h;
    u_short z;
    u_char  on;
    u_char  pad;
    short   axis;
} CasinoFaceDef;

/* A face's texture. */
typedef struct {
    short   u, v, w, h;
    u_short tpage;
    u_short clut;
} CasinoTex;

/* An object and the tables its faces are built and textured from. */
typedef struct {
    CasinoFaceDef *def;
    CasinoTex     *tex;
    short         *def_idx;
    short         *tex_idx;
    CasinoObj     *obj;
} CasinoModel;

/* A sprite's place on screen, its depth and whether it is shown. */
typedef struct {
    short   x, y, w, h;
    u_short z;
    u_char  on;
    u_char  pad;
} CasinoCell;

/* A picture made of a run of sprites: where each piece goes and which
   texture it shows, both as indices into the layout's own tables. While a
   tween is moving it, busy is set. */
typedef struct {
    short       busy;
    short       first;
    short       n;
    short       unk6;
    CasinoCell *cells;
    CasinoTex  *tex;
    short      *cell_idx;
    short      *tex_idx;
} CasinoLayout;

/* A tween: a layout's pieces (or their textures) moved by delta every
   frame for left + 1 frames. */
typedef struct {
    RECT          *delta;
    CasinoLayout **layout;
    short         *left;
    short          n;
} CasinoTweenQueue;

/* What a tween is asked for, handed over by value. */
typedef struct {
    RECT          d;
    CasinoLayout *l;
    short         frames;
} CasinoTweenArg;

/* Animations: an object's transform moved by delta every frame for
   left + 1 frames, the object drawn each time. */
typedef struct {
    CasinoXform *delta;
    CasinoObj  **obj;
    short       *left;
    short        n;
} CasinoAnimQueue;

/* What an animation is asked for, handed over by value. */
typedef struct {
    CasinoXform d;
    CasinoObj  *o;
    short       frames;
} CasinoAnimArg;

/* A palette animation: every wait + 1 ticks the next 16-colour palette
   in seq goes up to r, frames palettes in all. */
typedef struct {
    u_long (*pal)[8];
    short   *seq;
    RECT     r;
    u_char   on;
    u_char   pad;
    short    wait;
    short    frames;
} CasinoPalAnim;

/* A palette rotation: the count colours of pal turned by one place every
   wait + 1 ticks, a full turn in all. */
typedef struct {
    u_short *pal;
    u_char   count;
    u_char   pad;
    RECT     r;
    u_char   on;
    u_char   pad2;
    short    wait;
} CasinoPalCycle;

/* What is playing: the object, how many ticks it runs and how far it is. */
typedef struct {
    CasinoPalAnim *a[8];
    short          len[8];
    short          t[8];
    short          n;
} CasinoPalAnims;

typedef struct {
    CasinoPalCycle *a[8];
    short           len[8];
    short           t[8];
    short           n;
} CasinoPalCycles;

/* A grid of equal cells, rows by cols, gap apart; built into a layout. */
typedef struct {
    short   cols;
    short   rows;
    short   x;
    short   y;
    short   w;
    short   h;
    short   dx;
    short   dy;
    u_short z;
    u_char  on;
} CasinoGridDef;

/* A layout and how it is built, as the games list them. */
typedef struct {
    void         *def;
    CasinoLayout *l;
} CasinoLayoutDef;

/* A frame of four pieces: two of w0 by h0 and two of w1 by h1, taken from
   one place in a texture page. */
typedef struct {
    short   x;
    short   y;
    short   u;
    short   v;
    short   w0;
    short   h0;
    short   w1;
    short   h1;
    u_short z;
    u_char  on;
    u_char  pad;
    u_short tpage;
    u_short clut;
} CasinoFrameDef;

typedef struct {
    CasinoFrameDef *def;
    CasinoLayout   *l;
} CasinoFrame;

/* Colour fades: a run of sprites' tint stepped towards a colour, per
   buffer, up to 32 at a time. */
typedef struct {
    u_char r, g, b;
} CasinoRGB;

/* A fade's step per frame, per channel. */
typedef struct {
    s8 r, g, b;
} CasinoDelta;

typedef struct {
    short       first[2][32];
    short       count[2][32];
    CasinoDelta d[2][32];
    short       left[2][32];
    short       n[2];
} CasinoFadeQueue;

/* What a fade is asked for, handed over by value. */
typedef struct {
    short     first;
    short     count;
    CasinoRGB c;
    u_char    pad;
    short     frames;
} CasinoFadeArg;

/* The overlay's own. */
extern CasinoFadeQueue  g_casino_fades;
extern CasinoPalAnims   g_casino_palanims;
extern CasinoPalCycles  g_casino_palcycles;
extern CasinoAnimQueue  g_casino_anims;
extern CasinoCell       *g_casino_cells;
extern CasinoTex        *g_casino_texs;
extern CasinoTweenQueue  g_casino_tweens;
extern CasinoTweenQueue  g_casino_scrolls;
extern CasinoSprites   g_casino_sprites;
extern short           g_casino_sprite_count;
extern CasinoLoadQueue g_casino_load_queue;
extern CasinoQuadQueue g_casino_quads;
extern CasinoUVQueue   g_casino_uvs;
extern CasinoRectQueue g_casino_rects;
extern CasinoClutQueue g_casino_cluts;
extern CasinoSemiQueue g_casino_semis;
extern int             g_casino_buf;
extern CasinoDB       *g_casino_cur_db;
extern CasinoDB g_casino_db[2];
/* The lamps' flip states, a word the code reads either way: bit by bit
   through CASINO_LAMPS, or (the poker HUD) masked as a whole. */
extern u_int g_casino_lamps;
#define CASINO_LAMPS (*(CasinoFlags *)&g_casino_lamps)
extern u_short *g_casino_coin_item;
extern u_char   g_casino_game;
extern u_char   g_casino_step;
extern int      g_casino_frame;
extern u_int    g_casino_money;
extern int      g_casino_timer;
extern short    g_casino_vab;      /* the casino's own sound bank      */
extern short    g_casino_main_vab; /* the bank the field left open     */
extern short    g_casino_seqs[16]; /* open sequence handles, 0xFF free */
extern u_long   g_casino_pad;
extern u_long   g_casino_pad_trig;
extern u_char   g_casino_spot;
extern int      g_casino_seed;
extern u_char   g_casino_clock_on;

/* The round every game plays: coins in, a win paid out. */
extern u_char       g_casino_bet;      /* coins bet this round             */
extern u_char       g_casino_bet_shown; /* the bet as last drawn, 0xFF none  */
extern u_char       g_casino_max_bet;  /* coins going in until the limit    */
extern u_char       g_casino_bet_done; /* the bet is placed                 */
extern u_char       g_casino_quit;     /* leaving the table                 */
extern int          g_casino_win;      /* what the round paid               */
extern CasinoCursor g_casino_cursor;

/* A win is counted into the money a coin at a time, highest digit first:
   its eight decimal digits, and the one being counted. */
extern u_char g_casino_win_digits[8];
extern u_char g_casino_pay_digit;

/* The jackpot a card game feeds: what the HUD last drew, the ticks still to
   add to it, and whether this round hit it. */
extern int    g_casino_jackpot_shown;
extern u_char g_casino_jackpot_add;
extern u_char g_casino_jackpot_hit;

/* The shuffled shoe the card games deal from: poker uses ten cards of one
   deck, blackjack four decks. */
extern u_char g_casino_deck[208];

extern CasinoObj g_casino_objs[25]; /* the games' 3D objects: cards, chips */

#endif
