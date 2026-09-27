/* Persona 1 (JP) - bringing S2D's scene up.
 *   0x8008B5D4 S2dApplyPadLayout  0x8008B6AC S2dSceneInit
 *
 * S2dSceneInit runs once on entry: libsnd is restarted (or, coming back from
 * ADV with the music still going, only the mark callbacks are re-armed), the
 * graphics system is brought up with two 512x240 buffers side by side, the
 * five ordering-table pairs are pointed at their tag buffers, and the screen
 * is cleared behind a black fade.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libsnd.h>
#include <persona/main/state.h>
#include <persona/s2d/s2d.h>

#define g_pad_layout (*(u_char *)0x801F2AC7)
#define g_seq_handle ((short *)0x801F537C)

typedef struct {
    u_short mask;
    u_short pad;
} S2dKey;

/* The actions S2D reads the pad for, in the order its code tests them. */
extern S2dKey g_s2d_keys[10];

/* Double-buffered ordering tables, one pair per layer, and the tag buffers
   they point into. */

extern DR_MODE  g_scene_drmode[2];
extern DR_MODE  g_menu_drmode[2];
extern u_char   D_8009FCC0[];
extern u_char   D_800A4CFC[];

extern short  g_bgm_ready;

extern void DngSeqMarkCallback(short access, short seq, short data);
extern void SoundInit(void);
extern void S2dLoadMapSound(void);
extern void SoundOpenMapSeqs(void);
extern void ModelMap(u_long *tmd, int slot);
extern void VramQueueLoad(u_long *data, short x, short y, short w, short h);
extern void FadeTilesInit(void);
extern void S2dResetState(void);

void S2dApplyPadLayout(void)
{
    if (g_pad_layout != 0) {
        g_s2d_keys[0].mask = 1;
        g_s2d_keys[1].mask = 0x800;
        g_s2d_keys[2].mask = 0x100;
        g_s2d_keys[3].mask = 0x20;
        g_s2d_keys[4].mask = 0x10;
        g_s2d_keys[5].mask = 0;
        g_s2d_keys[6].mask = 0;
    } else {
        g_s2d_keys[0].mask = 0x40;
        g_s2d_keys[1].mask = 0x80;
        g_s2d_keys[2].mask = 1;
        g_s2d_keys[3].mask = 2;
        g_s2d_keys[4].mask = 0x10;
        g_s2d_keys[5].mask = 4;
        g_s2d_keys[6].mask = 8;
    }
    g_s2d_keys[7].mask = 0;
    g_s2d_keys[8].mask = 0;
    g_s2d_keys[9].mask = 0;
}

void S2dSceneInit(void)
{
    RECT rect;
    int  i;
    int  j;
    GsOT_TAG *tag;

    if (g_state_prev != GAME_STATE_ADV || g_bgm_ready != 0) {
        SsEnd();
        SsInit();
    } else if (g_btl_map_id == 1) {
        SsSetMarkCallback(g_seq_handle[1], 0, (SsMarkCallbackProc)DngSeqMarkCallback);
        SsSetMarkCallback(g_seq_handle[2], 0, (SsMarkCallbackProc)DngSeqMarkCallback);
    }
    if (g_state_prev != GAME_STATE_ADV || g_bgm_ready == 1) {
        SoundInit();
        S2dLoadMapSound();
        SoundOpenMapSeqs();
    }
    VSync(0);
    SetDispMask(0);
    GsInitGraph(0x200, 0xF0, 4, 0, 0);
    GsDefDispBuff(0, 0, 0x200, 0);
    GsInit3D();
    S2dApplyPadLayout();

    tag = g_ot_tag_back;
    for (i = 0; i < 2; i++) {
        g_ot_back[i].length = 4;
        g_ot_back[i].org = tag + i * 16;
    }
    tag = g_ot_tag_map;
    for (i = 0; i < 2; i++) {
        g_ot_map[i].length = 11;
        g_ot_map[i].org = tag + i * 2048;
    }
    tag = g_ot_tag_obj;
    for (i = 0; i < 2; i++) {
        g_ot_obj[i].length = 6;
        g_ot_obj[i].org = tag + i * 64;
    }
    for (j = 0; j < 3; j++) {
        for (i = 0; i < 2; i++) {
            g_ot_layer[j][i].length = 7;
            g_ot_layer[j][i].org = g_ot_tag_layer[j] + i * 128;
        }
    }
    tag = g_ot_tag_front;
    for (i = 0; i < 2; i++) {
        g_ot_front[i].length = 3;
        g_ot_front[i].org = tag + i * 8;
    }

    SetDrawMode(&g_menu_drmode[0], 0, 1, 0, 0);
    SetDrawMode(&g_scene_drmode[0], 0, 1, 0x40, 0);
    g_menu_drmode[1] = g_menu_drmode[0];
    g_scene_drmode[1] = g_scene_drmode[0];

    rect.x = 0;
    rect.y = 0;
    rect.w = 0x3FF;
    rect.h = 0xF0;
    ClearImage(&rect, 0, 0, 0);
    DrawSync(0);
    ModelMap((u_long *)D_8009FCC0, 0);

    SetPolyF4(&g_fade_poly);
    SetSemiTrans(&g_fade_poly, 1);
    SetShadeTex(&g_fade_poly, 0);
    g_fade_poly.r0 = 0;
    g_fade_poly.g0 = 0;
    g_fade_poly.b0 = 0;
    VramQueueLoad((u_long *)(D_800A4CFC + 0x48), 0x200, 0xF8, 0x10, 4);
    DrawSync(0);
    g_fade.rgb[0] = 0x60;
    g_fade.rgb[1] = 0x60;
    g_fade.rgb[2] = 0x60;
    FadeTilesInit();
    S2dResetState();
}
