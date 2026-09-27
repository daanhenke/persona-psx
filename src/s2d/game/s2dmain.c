/* Persona 1 (JP) - S2D's entry point and the state it keeps in the save.
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
#define SCRIPT_TAB   ((int *)(D_800A4CFC + 0xC8))

extern u_char D_801F2668;
extern u_char D_801F266A;
extern u_char D_801F266B;
extern int    D_801F2670;

extern short  g_bgm_ready;

extern int       D_800A4CE4;
extern int       D_800A4CEC;
/* [0] the exit a map asks for (-1 while none), [1] the party's facing. */
extern short     D_800A4CF4[2];
extern short     D_800A4CF8[2];
extern u_char    D_800A4CFC[];
/* The overlay's two work buffers: ordering-table tags, then the map's. */
extern GsOT_TAG *g_work_buf;
extern int       g_work_buf2;
extern short     D_800B0C68;
extern short     D_800B0CF4[];
extern short     D_800B0D2C;
extern short     D_800B0D3C;
extern u_long    D_800B1D30[];
extern int       D_800B85BC;
extern int       D_800B863C;
extern short     D_800B8FD4;
extern short     D_800B8FD8;
extern int       D_800B9150;
extern short     D_800B91EC[];
extern u_char    D_800AA66C[];

extern void S2dSceneInit(void);
extern void S2dLoadScene(void);
extern void func_80093680(void);
extern void S2dInitCoords(void);
extern void S2dInitLight(void);
extern void S2dInitView(void);
extern void func_8008E6A8(void);
extern void func_8009BF9C(void);
extern void func_80094B08(void);
extern void func_8008B048(void);

void S2dLoad266B(void)
{
    D_800A4CFC[2] = D_801F266B;
}

void S2dStore266B(void)
{
    D_801F266B = D_800A4CFC[2];
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
        D_800A4CF4[1] = *p - 1;
    } else {
        D_800A4CF4[1] = 1;
    }
}

void S2dStoreHeading(void)
{
    D_801F2668 = D_800A4CF4[1] + 1;
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
        D_800B0CF4[0] = D_800B0C68 = *p;
    }
}

void S2dStore2670(void)
{
    D_801F2670 = D_800B0C68;
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
    D_800B9150 = g_work_buf2;
    D_800B1D30[1] = D_800B9150 + 0x5780;
    bzero((u_char *)D_800B9150, 0x5780);
    if (*(u_short *)&g_map_id != 0) {
        ((u_char *)D_800B1D30)[0] = 1;
        ((u_char *)D_800B1D30)[1] = 1;
    }
    D_800B91EC[1] = 0;
    D_800A4CF4[0] = -1;
    S2dSceneInit();
    do {
        S2dLoad266B();
        S2dLoad266A();
        S2dLoadHeading();
        state = g_state_prev;
        g_s2d_facing = facing[g_adv_room & 3];
        D_800A4CF8[1] = D_800A4CF4[1] * 1024;
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
        func_8008E6A8();
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
        func_8008B048();
    } while (D_800A4CF4[0] == -1);
}
