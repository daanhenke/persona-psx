/* Persona 1 (JP) - putting a menu frame on screen.
 *
 * Compiled into three overlays rather than called across the boundary:
 *                  DNG         ADV         S2D
 *   FrameHook      0x8008F4C4  0x8008B2A4  0x8007F9D4
 *   VramClearRect  0x8008F4CC  0x8008B2AC  0x8007F9DC
 *   RenderFrame    0x8008F530  0x8008B310  0x8007FA40
 *   RenderFrames   0x8008F89C  0x8008B674  0x8007FDAC
 *
 * RenderFrame is the menus' whole frame: scroll the three map layers, sort
 * whichever of the six are shown, draw the slots and animations, then wait
 * for the GPU, flip, clear and start the next OT - and poll the pad and the
 * play clock, since this is the one place a menu frame passes through.
 *
 * ADV's copy does not call FrameHook; ADV's own frame, in charframe.c, does.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <decomp/include_asm.h>

#define BG_LAYERS 6
#define WORK_AT   (0x800C0000 + WORK_BIAS)
#define WORK_SIZE 0xB000

extern GsBG    g_bg_layers[];
extern u_short g_bg_layer_otz[BG_LAYERS];
extern u_short g_bg_shown;
extern GsOT    g_ot[];
/* Volatile: RenderFrame reads it back straight after setting it. */
extern volatile int g_ot_index;
extern int     g_effect_tick;
extern short   g_cam_x, g_cam_y;
extern short   g_view_dx, g_view_dy, g_view2_dx, g_view2_dy;
extern short   g_map_scroll_x, g_map_scroll_y;
extern short   g_header_scroll_x, g_header_scroll_y;

extern void SlotsRenderAll(void);
extern void ImageAnimStep(void);
extern void FlushImageUploads(void);
extern void PadPoll(void);
extern void PlayTimeTick(void);
extern int  rand(void);

/* Called once a frame, and empty. */
void FrameHook(void)
{
}

/* Blanks the display and clears a rectangle of VRAM to black.
   In the image sched2 leaves the prologue's sw $ra right after the frame
   adjust; here it lands two stores later. Nothing in the source shape has
   moved it (parameter types, K&R, a pointer or an array for the RECT,
   implicit int, a variable for the zeros); -fno-schedule-insns2 does, but
   RenderFrame needs sched2. The overlay's wrapper names the asm. */
#ifdef NON_MATCHING
void VramClearRect(int x, int y, int w, int h)
{
    RECT r;

    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;
    VSync(0);
    SetDispMask(0);
    ClearImage(&r, 0, 0, 0);
    DrawSync(0);
    VSync(0);
    VSync(0);
}
#else
RENDER_ASM_VRAMCLEARRECT;
#endif

void RenderFrame(void)
{
    g_bg_layers[0].scrollx = g_cam_x + g_view_dx;
    g_bg_layers[0].scrolly = g_cam_y + g_view_dy;
    g_bg_layers[1].scrollx = g_map_scroll_x + g_view2_dx;
    g_bg_layers[1].scrolly = g_map_scroll_y + g_view2_dy;
    g_bg_layers[2].scrollx = g_header_scroll_x;
    g_bg_layers[2].scrolly = g_header_scroll_y;
    if (g_bg_shown & 0x01) {
        GsSortFastBg(&g_bg_layers[0], &g_ot[g_ot_index], g_bg_layer_otz[0]);
    }
    if (g_bg_shown & 0x02) {
        GsSortFastBg(&g_bg_layers[1], &g_ot[g_ot_index], g_bg_layer_otz[1]);
    }
    if (g_bg_shown & 0x04) {
        GsSortFastBg(&g_bg_layers[2], &g_ot[g_ot_index], g_bg_layer_otz[2]);
    }
    if (g_bg_shown & 0x08) {
        GsSortFastBg(&g_bg_layers[3], &g_ot[g_ot_index], g_bg_layer_otz[3]);
    }
    if (g_bg_shown & 0x10) {
        GsSortFastBg(&g_bg_layers[4], &g_ot[g_ot_index], g_bg_layer_otz[4]);
    }
    if (g_bg_shown & 0x20) {
        GsSortFastBg(&g_bg_layers[5], &g_ot[g_ot_index], g_bg_layer_otz[5]);
    }
    SlotsRenderAll();
    ImageAnimStep();
    rand();
    DrawSync(0);
    FlushImageUploads();
    DrawSync(0);
    VSync(0);
    GsSwapDispBuff();
    GsSortClear(0, 0, 0, &g_ot[g_ot_index]);
    GsDrawOt(&g_ot[g_ot_index]);
    g_ot_index = GsGetActiveBuff();
    GsSetWorkBase((PACKET *)(WORK_AT + g_ot_index * WORK_SIZE));
    GsClearOt(0, 0, &g_ot[g_ot_index]);
    SetDispMask(1);
    PadPoll();
#ifndef RENDER_NO_HOOK
    FrameHook();
#endif
    PlayTimeTick();
    g_effect_tick = VSync(-1);
}

/* Runs `n` menu frames with nothing else happening. */
void RenderFrames(short n)
{
    while (n != 0) {
        RenderFrame();
        n--;
    }
}
