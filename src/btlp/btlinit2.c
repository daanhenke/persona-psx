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
#include <decomp/types.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/input.h>

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

extern long    g_btl_screen_dist;
extern u_char  g_btl_debug;
extern int     g_btl_debug_fnt;
extern RECT    g_btl_clear_rect;

extern u_short g_btl_pad2;
extern u_short g_btl_pad2_edge;
/* Defined in the unit before this one; the prototype is what
   decides how the arguments are converted. */
extern void BtlInitGraphics(void);

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
