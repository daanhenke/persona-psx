/* Persona 1 (JP) - S2D's entry point and the state it keeps in the save.
 *   0x80089804 S2dFrameUnlocked  0x80089840 S2dBeginFrame
 *   0x800899A8 .. 0x80089BEC  the save-block load/store pairs (S2dLoad*,
 *                             S2dStore*), S2dMarkScript, S2dResumeScript
 *   0x80089C04 ovl_s2d_entry
 *
 * S2D keeps a few bytes of its own in the save block from 0x801F2668 on; each
 * is copied into a working variable on entry and back on the way out, a pair
 * of small routines per field.
 *
 * ovl_s2d_entry carves the overlay's work buffer into ordering-table tags,
 * clears the second one, brings the scene up and then runs map after map
 * until one of them asks to leave: each pass places the party (from the
 * battle's record when it has just come back from one), restarts the music
 * when ADV did not leave it playing, loads the map and runs it.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libsnd.h>
#include <memory.h>
#include <persona/main/state.h>
#include <persona/s2d/s2d.h>

#define g_seq_handle ((short *)0x801F537C)
#define g_adv_room   (*(u_char *)0x801F5355)
#define MAP_POS_Y    ((u_char *)0x801F5353)
#define SCRIPT_TAB   (D_800A4CFC.scripts)

extern u_char D_801F2668;
extern u_char D_801F266A;
extern u_char D_801F266B;
extern int    D_801F2670;

extern short  g_bgm_ready;

extern int       D_800A4CE4;
extern int       D_800A4CEC;
extern short     D_800A4CF8[2];
/* The overlay's two work buffers: ordering-table tags, then the map's. */
extern GsOT_TAG *g_work_buf;
extern int       g_work_buf2;
extern short     D_800B0D2C;
extern short     D_800B0D3C;
extern u_long    D_800B1D30[];
extern int       D_800B85BC;
extern int       D_800B863C;
extern short     D_800B8FD4;
extern short     D_800B8FD8;
extern short     D_800B91EC[];
extern u_char    D_800AA66C[];

extern void S2dSceneInit(void);
extern void S2dLoadScene(void);
extern void func_80093680(void);
extern void S2dInitCoords(void);
extern void S2dInitLight(void);
extern void S2dInitView(void);
extern void S2dPlaceCompass(void);
extern void func_8009BF9C(void);
extern void func_80094B08(void);
extern void S2dRunMap(void);

extern u_long g_s2d_workbase;

/* Set while the field is not taking the pad (the debug screen's KEY_LOCK). */
extern int g_key_lock;

extern void S2dUpdate(void);
extern void S2dDraw(void);

/* One frame with the pad taken, after which it is locked again. */
void S2dFrameUnlocked(void)
{
    g_key_lock = 0;
    S2dUpdate();
    S2dDraw();
    g_key_lock = 1;
}

#ifdef NON_MATCHING
/* Starts a frame's drawing: the packet area for the buffer now being built
   (the map's, or the larger one `alt` asks for) and every ordering table
   of that side cleared. */
void S2dBeginFrame(int alt)
{
    int    j;
    u_long base;

    g_draw_side = GsGetActiveBuff();
    if (alt == 0) {
        base = g_draw_side * 0x21000 + 0x800E0000;
    } else {
        base = g_draw_side * 0x10000 + 0x80100000;
    }
    g_s2d_workbase = base;
    GsSetWorkBase((PACKET *)base);
    GsClearOt(0, 0, &g_ot_back[g_draw_side]);
    GsClearOt(0, 0, &g_ot_map[g_draw_side]);
    GsClearOt(0, 0, &g_ot_obj[g_draw_side]);
    for (j = 0; j < 3; j++) {
        GsClearOt(0, 0, &g_ot_layer[j][g_draw_side]);
    }
    GsClearOt(0, 0, &g_ot_front[g_draw_side]);
}
#else
/* 97%: the image keeps the active buffer in v0 and fills the branch slot
   with the shift; this build copies it out first. */
INCLUDE_ASM("s2d/nonmatchings/game/s2dmain", S2dBeginFrame);
#endif

void S2dLoad266B(void)
{
    D_800A4CFC.unk02 = D_801F266B;
}

void S2dStore266B(void)
{
    D_801F266B = D_800A4CFC.unk02;
}

void S2dMarkScript(int kind)
{
    u_char *flags = (u_char *)0x801F2669;
    int    *script = (int *)0x801F266C;

    switch (kind) {
    case 3:
        if (!(*flags & 1)) {
            *flags |= 1;
            *script = (int)(D_800AA66C + 0xD0);
        }
        break;
    case 5:
        if (!(*flags & 2)) {
            *flags |= 2;
            *script = (int)(D_800AA66C + 0x25C);
        }
        break;
    }
}

void S2dLoad266A(void)
{
    u_char *p = (u_char *)0x801F266A;

    if (*p == 0) {
        *p = 0x80;
    }
    D_800A4CE4 = *p;
}

void S2dStore266A(void)
{
    D_801F266A = D_800A4CE4;
}

void S2dLoadHeading(void)
{
    u_char *p = (u_char *)0x801F2668;

    if (*p != 0) {
        g_s2d_heading = *p - 1;
    } else {
        g_s2d_heading = 1;
    }
}

void S2dStoreHeading(void)
{
    D_801F2668 = g_s2d_heading + 1;
}

void S2dResumeScript(void)
{
    int i = 0;
    int s = 0x801F266C;

    for (; SCRIPT_TAB[i] != -1; i++) {
        if (*(int *)s == SCRIPT_TAB[i]) {
            break;
        }
    }
    do {
        s = *(int *)s;
    } while (0);
    if (s != 0 && SCRIPT_TAB[i] != -1) {
        func_80093680();
    }
}

void S2dStoreScript(void)
{
    int *script = (int *)0x801F266C;

    if (D_800A4CEC != -1) {
        *script = D_800A4CEC;
    }
}

void S2dLoad2670(void)
{
    int *p = (int *)0x801F2670;

    if (*p != 0) {
        D_800B0C90.rot.vx = D_800B0C04.rot.vx = *p;
    }
}

void S2dStore2670(void)
{
    D_801F2670 = D_800B0C04.rot.vx;
}

void ovl_s2d_entry(void)
{
    u_short   facing[4] = { 1, 3, 2, 0 };
    int       i;
    int       state;

    g_ot_tag_map = g_work_buf + 0x20;
    g_ot_tag_back = g_work_buf;
    g_ot_tag_obj = g_work_buf + 0x1020;
    for (i = 0; i < 3; i++) {
        g_ot_tag_layer[i] = g_work_buf + 0x10A0 + i * 0x100;
    }
    g_ot_tag_front = g_work_buf + 0x13A0;
    D_800B85BC = (int)(g_work_buf + 0x13B0);
    g_map_objs = (S2dMapObj *)g_work_buf2;
    D_800B1D30[1] = (int)g_map_objs + 0x5780;
    bzero((u_char *)g_map_objs, 0x5780);
    if (*(u_short *)&g_map_id != 0) {
        ((u_char *)D_800B1D30)[0] = 1;
        ((u_char *)D_800B1D30)[1] = 1;
    }
    D_800B91EC[1] = 0;
    g_s2d_exit = -1;
    S2dSceneInit();
    do {
        S2dLoad266B();
        S2dLoad266A();
        S2dLoadHeading();
        state = g_state_prev;
        g_s2d_facing = facing[g_adv_room & 3];
        D_800A4CF8[1] = g_s2d_heading * 1024;
        if (state == GAME_STATE_BTL) {
            g_map_id = g_btl_map_id;
            g_map_pos_x = g_btl_pos_x;
            g_map_pos_y = g_btl_pos_y;
            g_s2d_facing = g_btl_facing;
        }
        g_btl_map_id = g_map_id;
        g_map_side = *MAP_POS_Y < 0x91;
        D_800B8FD4 = g_map_pos_x;
        D_800B8FD8 = 0xC7 - *MAP_POS_Y;
        if (state != GAME_STATE_ADV || g_bgm_ready == 1) {
            SsSeqSetVol(g_seq_handle[0], 0x7E, 0x7E);
            SsSeqPlay(g_seq_handle[0], 1, 1);
            SsSeqSetVol(g_seq_handle[0], 0, 0);
            SsSeqSetCrescendo(g_seq_handle[0], 0x7E, 0x78);
        }
        S2dLoadScene();
        S2dInitCoords();
        S2dInitLight();
        S2dInitView();
        D_800B863C = 0;
        D_800B0D2C = -1;
        D_800B0D3C = -1;
        S2dPlaceCompass();
        func_8009BF9C();
        func_80094B08();
        g_s2d_cam_rx = 0x22A;
        g_s2d_cam_y = 0x4C8;
        g_s2d_cam_z = 0x1744;
        g_fade.unk04 = 1;
        g_fade.unk08 = 1;
        g_s2d_cam_ry = 0;
        g_s2d_cam_rz = 0;
        g_s2d_cam_x = 0;
        g_map_xform.rot.vy = 0x1000 - g_s2d_facing * 1024;
        S2dRunMap();
    } while (g_s2d_exit == -1);
}
