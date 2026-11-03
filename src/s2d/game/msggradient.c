/* Submit the message window and restore the projection origin after its backdrop. */
#include <persona/s2d/s2d.h>
extern int D_800B0EE4,D_800B93A4;
extern u_short D_800B93B8,D_800B93BC;
extern void S2dWinSetArea(S2dWin*,int,int,int),func_8009A368(S2dWin*,GsOT*,u_short,int);
void func_800937FC(void){SVECTOR v[4];POLY_G4 poly;MATRIX matrix;long sp90,sp94,sp98;long var_a0;int temp_a1,temp_lo;SVECTOR*temp_v1;
    S2dWin *win=&D_800B91EC;
    if (g_s2d_heading == 0) {
        D_800B93B8 += 0xF0;
    }
    if (D_800B91EC.flags != 0) {
        S2dWinSetArea(win, g_draw_side, g_s2d_drawenv.ofs[0], g_s2d_drawenv.ofs[1]);
        func_8009A368(win, &g_ot_back[g_draw_side], 0, g_draw_side);
        D_800B93B8 = D_800B93BC;
    }
    if (D_800B91EC.flags & 0x8000) {
        SetPolyG4(&poly);
        SetSemiTrans(&poly, 1);
        SetShadeTex(&poly, 0);
        if (D_800B93A4 == 0x80) {
            poly.r0 = 0;
            poly.g0 = 0;
            poly.b0 = 0xFF;
            poly.r1 = 0;
            poly.g1 = 0;
            poly.b1 = 0xFF;
            poly.r2 = 0;
            poly.g2 = 0;
            poly.b2 = 0xFF;
            poly.r3 = 0;
            poly.g3 = 0;poly.b3=255;
        } else {
            poly.r0 = 0;
            poly.g0 = 0;
            poly.b0 = 0;
            poly.r1 = 0;
            poly.g1 = 0;
            poly.b1 = 0;
            poly.r2 = 0x60;
            poly.g2 = 0x40;
            poly.b2 = 0xFF;
            poly.r3 = 0x60;
            poly.g3 = 0x40;poly.b3=255;
        }

        ReadGeomOffset(&sp90, &sp94);
        if(g_s2d_heading)SetGeomOffset(win->x,win->y);else SetGeomOffset(win->x+240,win->y);
        RotMatrix(&win->rot, &matrix);
        TransMatrix(&matrix, &win->trans);
        ScaleMatrix(&matrix, &win->scale);
        SetRotMatrix(&matrix);
        SetTransMatrix(&matrix);
        sp98 = 0;
        do {
            temp_lo = (sp98 % 2) * (D_800B0EE4 + 1);
            temp_v1 = &v[sp98];
            temp_a1 = sp98 + 1;
            temp_v1->vy = (s16) (((sp98 / 2) * 0x18) - 0xC);
            temp_v1->vz = 0;
            sp98 = temp_a1;
            temp_v1->vx = (s16) ((temp_lo * 0x10) - 8);
        } while (temp_a1 < 4);
        RotTransPers4(&v[0], &v[1], &v[2], &v[3], (long*)&poly.x0, (long*)&poly.x1, (long*)&poly.x2, (long*)&poly.x3, &sp98, &sp98);
        GsSortPoly(&poly, &g_ot_back[g_draw_side], 0);
        SetGeomOffset(sp90, sp94);
    }
}