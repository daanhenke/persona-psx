/* Persona 1 (JP) - what the battle overlay sets up before it runs.
 *   BTLP @ 0x8008189C BtlInitGraphics, 0x8008DB78 BtlPadRead
 *
 * The packet area holds two complete frames, 0xE660 apart, and each one opens
 * with its own environments: a DRAWENV at the start and a DISPENV at +0x5C,
 * which is what leaves the primitive arrays starting at +0x70. The two frames
 * are the two halves of a 320x480 frame buffer, so each draws into the half the
 * other is displaying.
 *
 * The debug font is only opened when g_btl_debug is set; without it the display
 * stays blanked here and is unblanked later by whoever is ready to draw.
 */
#include <types.h>
#include <libetc.h>
#include <libgpu.h>
#include <libgte.h>

#define BTL_FRAME_STRIDE 0xE660
#define BTL_DISPENV_OFF  0x5C

#define SCREEN_W 320
#define SCREEN_H 240

/* Where the debug font's texture and window go when g_btl_debug is on. */
#define DEBUG_FNT_TX 0x200
#define DEBUG_FNT_TY 0x100
#define DEBUG_FNT_X  0x10
#define DEBUG_FNT_Y  0x10
#define DEBUG_FNT_W  0x100
#define DEBUG_FNT_H  0x80
#define DEBUG_FNT_N  0x200

extern u_char *g_btl_prim_pool;
extern long    g_btl_screen_dist;
extern u_char  g_btl_debug;
extern int     g_btl_debug_fnt;
extern RECT    g_btl_clear_rect;

extern u_short g_btl_pad1;
extern u_short g_btl_pad2;
extern u_short g_btl_pad1_edge;
extern u_short g_btl_pad2_edge;

void BtlInitGraphics(void)
{
    VSync(0);
    SetDispMask(0);
    InitGeom();
    SetGraphDebug(0);
    SetGeomScreen(g_btl_screen_dist);

    SetDefDrawEnv((DRAWENV *)g_btl_prim_pool, 0, 0, SCREEN_W, SCREEN_H);
    SetDefDrawEnv((DRAWENV *)(g_btl_prim_pool + BTL_FRAME_STRIDE), 0, SCREEN_H,
                  SCREEN_W, SCREEN_H);
    SetDefDispEnv((DISPENV *)(g_btl_prim_pool + BTL_DISPENV_OFF), 0, SCREEN_H,
                  SCREEN_W, SCREEN_H);
    SetDefDispEnv((DISPENV *)(g_btl_prim_pool + BTL_FRAME_STRIDE
                              + BTL_DISPENV_OFF), 0, 0, SCREEN_W, SCREEN_H);

    ((DRAWENV *)g_btl_prim_pool)->isbg = 1;
    ((DRAWENV *)(g_btl_prim_pool + BTL_FRAME_STRIDE))->isbg = 1;
    ((DRAWENV *)g_btl_prim_pool)->dtd = 0;
    ((DRAWENV *)(g_btl_prim_pool + BTL_FRAME_STRIDE))->dtd = 0;
    ((DRAWENV *)g_btl_prim_pool)->r0 = 0;
    ((DRAWENV *)g_btl_prim_pool)->g0 = 0;
    ((DRAWENV *)g_btl_prim_pool)->b0 = 0;
    ((DRAWENV *)(g_btl_prim_pool + BTL_FRAME_STRIDE))->r0 = 0;
    ((DRAWENV *)(g_btl_prim_pool + BTL_FRAME_STRIDE))->g0 = 0;
    ((DRAWENV *)(g_btl_prim_pool + BTL_FRAME_STRIDE))->b0 = 0;

    g_btl_pad1 = 0;
    g_btl_pad2 = 0;

    if (g_btl_debug != 0) {
        FntLoad(DEBUG_FNT_TX, DEBUG_FNT_TY);
        g_btl_debug_fnt = FntOpen(DEBUG_FNT_X, DEBUG_FNT_Y, DEBUG_FNT_W,
                                  DEBUG_FNT_H, 0, DEBUG_FNT_N);
        SetDispMask(1);
    }
    ClearImage(&g_btl_clear_rect, 0, 0, 0);
}

/* Both pads, and what each has pressed since the last call.

   Both complements go through one temporary, and pad 1's is taken before the
   globals are overwritten. Writing `pad & ~was1` directly costs the match. */
void BtlPadRead(void)
{
    u_long  pad;
    u_short was1;
    u_short was2;
    int     inv;

    pad = PadRead(0);
    was1 = g_btl_pad1;
    was2 = g_btl_pad2;
    inv = ~was1;
    g_btl_pad1 = pad;
    g_btl_pad2 = pad >> 16;
    g_btl_pad1_edge = pad & inv;
    inv = ~was2;
    g_btl_pad2_edge = g_btl_pad2 & inv;
}
