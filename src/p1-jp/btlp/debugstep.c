/* Persona 1 (JP) - stepping through the battle overlay's start-up.
 *   BTLP @ 0x8008E4DC BtlDebugWait, 0x8008E638 BtlDebugWaitArgs
 *
 * ovl_btlp_entry calls one of these after every stage it completes - GPU_INIT,
 * PRIM_INIT, SND_INIT, SND_TRANSFER, CALL_BACK_SET, TEXTURE_SET, BG_INIT,
 * WORK_COPY, ACCESS_LAMP_SET, PARTY_SET - each guarded by g_btl_debug. With the
 * flag set the overlay boots one keypress at a time with the stage's name on
 * screen; with it clear none of this runs at all.
 *
 * Each pass puts up the frame that is ready, prints over it, and flips, so the
 * text keeps appearing while the loop waits.
 */
#include <types.h>
#include <libetc.h>
#include <libgpu.h>

#define BTL_FRAME_STRIDE 0xE660
#define BTL_DISPENV_OFF  0x5C
#define BTL_OT_OFF       0xD6C0

extern u_char *g_btl_prim_pool;
extern u_char  g_btl_frame;
extern int     g_btl_debug_fnt;
extern u_short g_btl_pad1_edge;

extern void BtlPadRead(void);

void BtlDebugWait(const char *msg)
{
    do {
        VSync(0);
        PutDispEnv((DISPENV *)(g_btl_prim_pool + g_btl_frame * BTL_FRAME_STRIDE
                               + BTL_DISPENV_OFF));
        PutDrawEnv((DRAWENV *)(g_btl_prim_pool
                               + g_btl_frame * BTL_FRAME_STRIDE));
        DrawOTag((u_long *)(g_btl_prim_pool + g_btl_frame * BTL_FRAME_STRIDE
                            + BTL_OT_OFF));
        BtlPadRead();
        FntPrint(g_btl_debug_fnt, msg);
        FntFlush(g_btl_debug_fnt);
        g_btl_frame = g_btl_frame ^ 1;
    } while (g_btl_pad1_edge == 0);
}

void BtlDebugWaitArgs(const char *fmt, int a, int b, int c)
{
    do {
        VSync(0);
        PutDispEnv((DISPENV *)(g_btl_prim_pool + g_btl_frame * BTL_FRAME_STRIDE
                               + BTL_DISPENV_OFF));
        PutDrawEnv((DRAWENV *)(g_btl_prim_pool
                               + g_btl_frame * BTL_FRAME_STRIDE));
        DrawOTag((u_long *)(g_btl_prim_pool + g_btl_frame * BTL_FRAME_STRIDE
                            + BTL_OT_OFF));
        BtlPadRead();
        FntPrint(g_btl_debug_fnt, fmt, a, b, c);
        FntFlush(g_btl_debug_fnt);
        g_btl_frame = g_btl_frame ^ 1;
    } while (g_btl_pad1_edge == 0);
}
