#include <persona/s2d/s2d.h>
extern int D_800A4D08, D_800A4D1C, D_800A4D20, D_800A4D34, D_800A4D3C, D_800A4D40;
extern int D_800B0EF0, D_800B0F18, D_800B8650;
extern short D_800B8FD4, D_800B8FD8;
extern int g_cd_busy, g_key_lock;
extern char *g_s2d_workbase;
extern void VramFlushQueues(void), S2dMapPagesFlip(void), func_8008E1C0(void), func_80097288(int),
    FadeTilesDraw(void);
extern char *func_80034860(void);
#include <persona/common/eventflag.h>
#include <libetc.h>
extern int D_800A4D0C[4];
extern short g_s2d_selected_event_flag;
extern u_short g_map_id;

extern char *D_800A4D24[], *D_800A4E8C[], *D_800A4EBC[], *D_800A4ED0[];
typedef struct {
    char label[4][4];
} Labels4;
typedef struct {
    char label[2][4];
} Labels2;
extern Labels4 D_80064E70;
extern Labels2 D_80064E80;

extern char D_80064C8C[];
extern char D_80064C98[];
extern char D_80064C9C[];
extern char D_80064CAC[];
extern char D_80064CBC[];
extern char D_80064CD8[];
extern char D_80064CEC[];
extern char D_80064CF8[];
extern char D_80064D04[];
extern char D_80064D10[];
extern char D_80064D14[];
extern char D_80064D18[];
extern char D_80064D24[];
extern char D_80064D34[];
extern char D_80064D54[];
extern char D_80064D7C[];
extern char D_80064D8C[];
extern char D_80064DA0[];
extern char D_80064DB4[];
extern char D_80064DBC[];
extern char D_80064DC4[];
extern char D_80064DD0[];
extern char D_80064DD8[];
extern char D_80064DE0[];
extern char D_80064DE8[];
extern char D_80064DEC[];
extern char D_80064DF8[];
extern char D_80064E04[];
extern char D_80064E18[];
extern char D_80064E1C[];
extern char D_80064E24[];
extern char D_80064E34[];
extern char D_80064E3C[];
extern char D_80064E4C[];
extern char D_80064E50[];
extern char D_80064E5C[];
extern char D_80064E68[];

/* Complete the frame and draw the selected diagnostic menu. */
void func_80089F5C(s32 syncMode) {
    /* Preserve the original local workspace before the debug label arrays. */
    char reserved[8];
    Labels4 labels4;
    Labels2 labels2;
    s32 mapY;
    s32 setting;
    s32 packetBytes;
    s32 maxObjects;
    s32 i;
    s32 layer;
    char *cdStatus;
    char *hitStatus;
    char *keyStatus;
    u32 bar;
    AddPrim(g_ot_back[g_draw_side].tag, &g_menu_drmode[g_draw_side]);
    AddPrim(g_ot_obj[g_draw_side].tag, &g_scene_drmode[g_draw_side]);
    GetDrawEnv(&g_s2d_drawenv);
    if (D_800A4D08 != 0) {
        SetDispMask(0);
    }
    DrawSync(0);
    VramFlushQueues();
    if (D_800A4D08 != 0) {
        SetDispMask(1);
    }
    packetBytes = func_80034860() - g_s2d_workbase;
    maxObjects = D_800A4D3C;
    if (maxObjects < g_map_obj_count) {
        maxObjects = g_map_obj_count;
    }
    D_800A4D3C = maxObjects;
    if (D_800A4D40 < packetBytes) {
        D_800A4D40 = packetBytes;
    }
    switch (D_800A4D20) {
    case 0:
        break;
    case 1:
        g_fade_poly.x0 = -256;
        g_fade_poly.y0 = -100;
        g_fade_poly.x1 = -0x40;
        g_fade_poly.y1 = -100;
        g_fade_poly.x2 = -256;
        g_fade_poly.y2 = 0xC;
        g_fade_poly.x3 = -0x40;
        g_fade_poly.y3 = 0xC;
        GsSortPoly(&g_fade_poly, &g_ot_back[g_draw_side], 0U);
        FntPrint(D_80064C8C);
        cdStatus = D_80064CAC;
        if (g_cd_busy != -1) {
            cdStatus = D_80064C9C;
        }
        FntPrint(D_80064C98, cdStatus);
        FntPrint(D_80064CBC, D_800B8FD4, D_800B8FD4);
        mapY = 0xC7 - D_800B8FD8;
        FntPrint(D_80064CD8, mapY, mapY);
        FntPrint(D_80064CEC, D_800A4D24[g_s2d_exit]);
        FntPrint(D_80064CF8, D_800B0EF0);
        hitStatus = D_80064D14;
        if (g_fade.unk04 != 0) {
            hitStatus = D_80064D10;
        }
        FntPrint(D_80064D04, hitStatus);
        FntPrint(D_80064D18, *(u_short *)(D_800B8FD4 * 2 +
                                          (D_800B8FD8 * 400 + *(u_int *)0x80122000) + 0x80122000));
        FntPrint(D_80064D24, 0x1080);
        FntPrint(D_80064D34, g_map_obj_count, D_800A4D3C);
        FntPrint(D_80064D54, packetBytes, D_800A4D40);
        FntPrint(D_80064D7C, *(u8 *)0x801F2B30);
        FntPrint(D_80064D8C, D_800B8650);
        keyStatus = D_80064DBC;
        if (g_key_lock == 1) {
            keyStatus = D_80064DB4;
        }
        FntPrint(D_80064DA0, keyStatus);
        FntFlush(D_800B0F18);
        break;
    case 2: /* Map and event controls. */

        g_fade_poly.x0 = -256;
        g_fade_poly.y0 = -100;
        g_fade_poly.x1 = -0x20;
        g_fade_poly.y1 = -100;
        g_fade_poly.x2 = -256;
        g_fade_poly.y2 = 4;
        g_fade_poly.x3 = -0x20;
        g_fade_poly.y3 = 4;
        GsSortPoly(&g_fade_poly, &g_ot_back[g_draw_side], 0U);
        FntPrint(D_80064DC4);
        for (i = 0; i < 12; i++) {
            if (i != D_800A4D1C)
                FntPrint(D_80064DD0);
            else {
                if (g_s2d_frame & 1)
                    FntPrint(D_80064DD8);
                else
                    FntPrint(D_80064DE0);
            }
            FntPrint(D_80064DE8, D_800A4E8C[i]);
            switch (i) {
            case 4:
                FntPrint(D_80064DEC, g_map_id, g_map_id);
                break;
            case 5:
                FntPrint(D_80064DF8, *(u_char *)0x801F5354, *(u_char *)0x801F5354);
                break;
            case 6:
                FntPrint(D_80064DF8, *(u_char *)0x801F5355, *(u_char *)0x801F5355);
                break;
            case 7:
                FntPrint(D_80064DF8, g_map_pos_x, g_map_pos_x);
                break;
            case 8:
                FntPrint(D_80064DF8, g_map_pos_y, g_map_pos_y);
                break;
            case 9:
                FntPrint(D_80064E04, g_s2d_selected_event_flag, g_s2d_selected_event_flag,
                         EventFlagTest((u_short *)&g_s2d_selected_event_flag) == 1 ? D_80064D10
                                                                                   : D_80064D14);
                break;
            case 10:
                FntPrint(D_80064DE8, g_fade.unk08 == 1 ? D_80064D10 : D_80064D14);
                break;
            }
            FntPrint(D_80064E18);
        }
        FntPrint(D_80064E1C);
        FntFlush(D_800B0F18);
        break;
    case 3: /* Sound controls. */

        g_fade_poly.x0 = -256;
        g_fade_poly.y0 = -100;
        g_fade_poly.x1 = -0x38;
        g_fade_poly.y1 = -100;
        g_fade_poly.x2 = -256;
        g_fade_poly.y2 = -0x34;
        g_fade_poly.x3 = -0x38;
        g_fade_poly.y3 = -0x34;
        GsSortPoly(&g_fade_poly, &g_ot_back[g_draw_side], 0U);
        FntPrint(D_80064E24);
        i = 0;
        do {
            if (i != D_800A4D1C)
                FntPrint(D_80064DD0);
            else {
                if (g_s2d_frame & 1)
                    FntPrint(D_80064DD8);
                else
                    FntPrint(D_80064DE0);
            }
            FntPrint(D_80064DE8, D_800A4EBC[i]);
            switch (i) {
            case 0:
                FntPrint(D_80064E34, D_800A4D0C[0] ? D_80064D10 : D_80064D14);
                break;
            case 1:
            case 2:
            case 3:
                setting = D_800A4D0C[i];
                FntPrint(D_80064E3C, setting, setting);
                if (((short *)0x801F538E)[i] != -1)
                    for (bar = 0; bar < g_s2d_frame % 5; bar++)
                        FntPrint(D_80064E4C);
                /* Fall through to the newline shared with the last row. */
            case 4:
                FntPrint(D_80064E18);
                break;
            }
            i++;
        } while (i < 5);
        FntPrint(D_80064E1C);
        FntFlush(D_800B0F18);
        break;
    case 4: /* Tile diagnostics. */

        g_fade_poly.x0 = -256;
        g_fade_poly.y0 = -100;
        g_fade_poly.x1 = -0x38;
        g_fade_poly.y1 = -100;
        g_fade_poly.x2 = -256;
        g_fade_poly.y2 = 0xC;
        g_fade_poly.x3 = -0x38;
        g_fade_poly.y3 = 0xC;
        GsSortPoly(&g_fade_poly, &g_ot_back[g_draw_side], 0U);
        FntPrint(D_80064E50);
        for (i = 0; i < 9; i++) {
            FntPrint(D_80064DE8, D_800A4ED0[i]);
            switch (i) {
            case 1:
                FntPrint(D_80064E5C, D_800B8FD4, D_800B8FD4);
                break;
            case 2:
                FntPrint(D_80064E5C, 199 - D_800B8FD8, 199 - D_800B8FD8);
                break;
            case 3:
                FntPrint(D_80064E68, *(u_short *)&D_800A4D34);
                break;
            case 4:
                FntPrint(D_80064E5C, *(u_char *)&D_800A4D34, *(u_char *)&D_800A4D34);
                break;
            case 5:
                labels4 = D_80064E70;
                FntPrint(D_80064DE8, labels4.label[(D_800A4D34 >> 8) & 3]);
                break;
            case 6:
                labels2 = D_80064E80;
                FntPrint(D_80064DE8, labels2.label[(D_800A4D34 >> 15) & 1]);
                break;
            case 8:
                FntPrint(D_80064E68,
                         *(u_short *)(D_800B8FD4 * 2 + (D_800B8FD8 * 400 + *(u_int *)0x80122000) +
                                      0x80122000));
                break;
            }
            FntPrint(D_80064E18);
        }
        FntPrint(D_80064E1C);
        FntFlush(D_800B0F18);
        FadeTilesDraw();
        break;
    }
    DrawSync(0);
    D_800B0EF0 = VSync(syncMode);
    ResetGraph(1);
    layer = 0;
    S2dMapPagesFlip();
    func_8008E1C0();
    GsSwapDispBuff();
    GsSortClear(g_fade.rgb[0], g_fade.rgb[1], g_fade.rgb[2], &g_ot_obj[g_draw_side]);
    GsDrawOt(&g_ot_obj[g_draw_side]);
    GsDrawOt(&g_ot_map[g_draw_side]);
    GsDrawOt(&g_ot_front[g_draw_side]);
    GsDrawOt(&g_ot_back[g_draw_side]);
    for (layer = 0; layer < 3; layer++)
        GsDrawOt(&g_ot_layer[layer][g_draw_side]);
    g_fade.rgb[0] = 0x60;
    g_fade.rgb[1] = 0x60;
    g_fade.rgb[2] = 0x60;
    g_s2d_frame += 1;
    func_80097288(syncMode);
}
