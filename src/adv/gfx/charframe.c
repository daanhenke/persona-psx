/* Persona 1 (JP) - a frame of the TYN cutscene.  ADV @ 0x8008B0A8.
 *
 * AdvTynCutscene's RenderFrame: only the fifth map layer, scrolled with the
 * camera, then the slots and animations, and - when asked - the two turning
 * pictures from tyn3d.c on top. It waits two vertical blanks where the menus
 * wait one, clears to the cutscene's own colour, and calls FrameHook, which
 * ADV's RenderFrame leaves out.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

#define WORK_AT   0x800C0000
#define WORK_SIZE 0xB000

extern GsBG          g_bg_layers[];
extern u_short       g_bg_layer_otz[];
extern int           g_bg_shown;
extern GsOT          g_ot[];
extern int           g_ot_index;
extern int           g_effect_tick;
extern short         g_cam_x, g_cam_y;
extern short         g_view_dx, g_view_dy;
extern u_char        g_BC5C4, g_BC5BC, g_BC5B8;
extern GsCOORDINATE2 g_tyn_coords[];

extern void SlotsRenderAll(void);
extern void ImageAnimStep(void);
extern void FlushImageUploads(void);
extern void PadPoll(void);
extern void PlayTimeTick(void);
extern void FrameHook(void);
extern int  rand(void);
extern void TynDrawPlanes(GsCOORDINATE2 *unused, int buf, u_long *ot,
                          int unused2);

void AdvRenderCharFrame(short planes)
{
    g_bg_layers[0].scrollx = g_cam_x + g_view_dx;
    g_bg_layers[0].scrolly = g_cam_y + g_view_dy;
    if (g_bg_shown & 0x10) {
        GsSortFastBg(&g_bg_layers[4], &g_ot[g_ot_index], g_bg_layer_otz[4]);
    }
    SlotsRenderAll();
    ImageAnimStep();
    if (planes) {
        TynDrawPlanes(g_tyn_coords, g_ot_index, g_ot[g_ot_index].org, 0);
    }
    rand();
    DrawSync(0);
    FlushImageUploads();
    DrawSync(0);
    VSync(2);
    GsSwapDispBuff();
    GsSortClear(g_BC5C4, g_BC5BC, g_BC5B8, &g_ot[g_ot_index]);
    GsDrawOt(&g_ot[g_ot_index]);
    g_ot_index = GsGetActiveBuff();
    GsSetWorkBase((PACKET *)(WORK_AT + g_ot_index * WORK_SIZE));
    GsClearOt(0, 0, &g_ot[g_ot_index]);
    SetDispMask(1);
    PadPoll();
    FrameHook();
    PlayTimeTick();
    g_effect_tick = VSync(-1);
}
