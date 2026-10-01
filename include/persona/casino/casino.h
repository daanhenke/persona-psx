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
} CasinoSprites;

/* Images waiting for the next DrawSync to go up to VRAM. */
typedef struct {
    RECT    *rect;
    u_long **data;
} CasinoLoadQueue;

/* The overlay's own. */
extern CasinoSprites   g_casino_sprites;
extern short           g_casino_sprite_count;
extern CasinoLoadQueue g_casino_load_queue;
extern short           g_casino_load_count;
extern int             g_casino_buf;
extern CasinoDB       *g_casino_cur_db;
extern CasinoDB g_casino_db[2];
extern CasinoFlags D_800AFC98;
extern u_short *g_casino_coin_item;
extern u_char   g_casino_game;
extern u_char   g_casino_step;
extern int      g_casino_frame;
extern int      g_casino_money;
extern int      g_casino_timer;
extern short    g_casino_vab;
extern u_long   g_casino_pad;
extern u_long   g_casino_pad_trig;
extern u_char   g_casino_spot;
extern int      g_casino_seed;
extern u_char   g_casino_clock_on;

#endif
