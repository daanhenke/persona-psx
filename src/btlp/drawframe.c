/* Persona 1 (JP) - one frame of the battle.  BTLP only.
 *   0x800803D4 BtlDrawFrame
 *
 * Everything that waits on something waits by calling this: it advances the
 * objects a frame, draws them, hands the ordering table to the GPU and reads
 * the pad. A stage that is holding for a message or an animation is doing it
 * by turning this over until whatever it wants has happened.
 *
 * The two frame buffers alternate, so every address in here is worked out
 * from g_btl_prim_pool and whichever of the two g_btl_frame says is current.
 *
 * The reverb settings are re-applied on the first frame after the clock says
 * one is due rather than once at the start: the sound driver loses them when
 * a sequence is swapped, and this is where they are put back.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/input.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>

extern u_long *g_btl_actor_clut;

/* What the reverb goes back to. */
#define BTL_REVERB_TYPE  4
#define BTL_REVERB_VOL   0x7F
#define BTL_REVERB_DEPTH 0x38

/* The bit BtlDrawFrame presses for the pad reader. */
#define BTL_PAD_CONFIRM 0x20

void BtlDrawFrame(void)
{
    BtlDrawDebugHud();
    BtlCheatWatch();
    if (g_btl_frame_due != 0) {
        SsUtSetReverbType(BTL_REVERB_TYPE);
        SsSetRVol(BTL_REVERB_VOL, BTL_REVERB_VOL);
        SsUtSetReverbDepth(BTL_REVERB_DEPTH, BTL_REVERB_DEPTH);
        SsUtReverbOn();
        if (g_btl_bgm_kinds[g_btl_bgm_index] == BTL_BGM_KIND_STOP) {
            BtlSePlay(0, 2);
        } else {
            BtlSePlay(0, 0);
        }
        g_btl_frame_due = 0;
    }
    g_btl_draw_dist = g_btl_screen_dist;
    ClearOTag((u_long *)(g_btl_prim_pool + g_btl_frame * BTL_FRAME_STRIDE
                         + BTL_OT),
              BTL_OT_LEN);
    BtlTickObjects();
    BtlStepCluts();
    BtlStepObjScripts();
    BtlWaveMesh();
    BtlDrawObjects();
    if (g_btl_closing == 0) {
        BtlDrawBehind((u_long *)(g_btl_prim_pool
                                 + g_btl_frame * BTL_FRAME_STRIDE
                                 + BTL_OT_END));
        BtlDrawFront((u_long *)(g_btl_prim_pool
                                + g_btl_frame * BTL_FRAME_STRIDE
                                + BTL_OT_END));
        BtlFlushVramQueues();
    }
    /* Both display environments, not just the current one. */
    g_btl_prim_pool[BTL_DISPENV + 0x10] = g_btl_interlace;
    g_btl_prim_pool[BTL_FRAME_STRIDE + BTL_DISPENV + 0x10] = g_btl_interlace;
    if (g_btl_blank_on_load != 0) {
        SetDispMask(0);
    }
    if (g_btl_closing == 0) {
        LoadImage(&g_btl_clut_block, g_btl_actor_clut);
    }
    DrawSync(0);
    if (g_btl_blank_on_load != 0) {
        SetDispMask(1);
    }
    if (g_btl_half_rate != 0 && g_btl_vsync_count == 0) {
        VSync(0);
    }
    VSync(0);
    PutDispEnv((DISPENV *)(g_btl_prim_pool + g_btl_frame * BTL_FRAME_STRIDE
                           + BTL_DISPENV));
    PutDrawEnv((DRAWENV *)(g_btl_prim_pool
                           + g_btl_frame * BTL_FRAME_STRIDE));
    DrawOTag((u_long *)(g_btl_prim_pool + g_btl_frame * BTL_FRAME_STRIDE
                        + BTL_OT));
    BtlPadRead();
    if ((g_btl_pad1_edge & g_btl_help_key) != 0) {
        g_btl_no_help ^= 1;
    }
    if (g_btl_auto_confirm != 0) {
        g_btl_pad1_edge |= BTL_PAD_CONFIRM;
    }
    if (g_btl_delay > 0) {
        g_btl_delay--;
    }
    g_btl_vsync_count = 0;
    g_btl_tick++;
    g_btl_frame ^= 1;
}
