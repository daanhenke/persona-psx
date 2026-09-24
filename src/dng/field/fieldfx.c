/* Persona 1 (JP) - getting the field ready for a battle transition.
 * DNG only.
 *   0x800712B8 FieldFxBegin
 *   0x80071368 FieldFxRun
 *   0x800713B0 FieldFxStep
 *   0x8007192C FieldFxSetup
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <rand.h>
#include <persona/dng/field.h>

/* Redraws until the first display buffer is the one being drawn, hands the
   scene's objects to the transition, and lets three frames pass on the
   clocks while it sets up. The display then covers 256 lines. */
void FieldFxBegin(void)
{
    func_80065978();
    do {
        func_80065978();
    } while (g_draw_buf != 0);
    g_fx_tiles = (POLY_FT4 *)g_scene->objs;
    g_fx_pos = (SVECTOR *)g_scene->coords;
    g_fx_rots = g_scene->rots;
    FieldFxSetup();
    VSync(3);
    FieldClockTick(3);
    GsDefDispBuff(320, 256, 0, 240);
    g_fx_shift.vx = g_fx_shift.vy = g_fx_shift.vz = 0;
}

/* Sixty-four frames of transition `kind`. */
void FieldFxRun(int kind)
{
    int i;

    for (i = 0; i < 64; i++) {
        FieldFxStep(kind);
    }
}

/* `a` moved `d` further from 0, `d` held in n. */
#define AWAY(a, d) (n = (d), (a) < 0 ? (a) - n : (a) + n)

/* One frame of the shattering: every tile darkens; once its delay has run
   out it tumbles and drifts off at random, falling faster and faster
   (kind 0) or drifting up and down as well; before that it only backs
   away. Kind 1 also spins the whole screen. The bottom row's tiles are
   half height. */
#ifdef NON_MATCHING
void FieldFxStep(int kind)
{
    MATRIX    m;
    MATRIX    tm;
    SVECTOR   bottom_left = { -16, 16, 0 };
    SVECTOR   bottom_right = { 16, 16, 0 };
    long      flag;
    long      p;
    long      otz;
    POLY_FT4 *tile;
    int       i;
    int       n;
    int       y;
    int       shift;

    tile = g_fx_tiles;
    g_draw_buf = GsGetActiveBuff();
    GsSetWorkBase(g_scene->packets[g_draw_buf]);
    GsClearOt(0, 0, &g_scene->world_ot[g_draw_buf]);
    if (kind == 1) {
        g_fx_shift.vz += 55;
    }
    for (i = 0; i < FX_TILES; i++) {
        tile->r0 -= 2;
        tile->g0 -= 2;
        tile->b0 -= 2;
        tile++;
        if (g_wave_delay[i] == 0) {
            g_fx_rots[i].vx += 32 + (rand() & 0x3F);
            g_fx_rots[i].vy += 32 + (rand() & 0x3F);
            g_fx_rots[i].vz += 32 + (rand() & 0x3F);
            g_fx_pos[i].vx = AWAY(g_fx_pos[i].vx, rand() % 6 + 1);
            g_fx_pos[i].vz -= rand() % 4 + 1;
            if (kind == 0) {
                g_wave_speed[i] += 0x40;
                n = (g_wave_speed[i] * g_wave_speed[i]) >> 16;
                y = g_fx_pos[i].vy;
                g_fx_pos[i].vy = g_wave_speed[i] < 0 ? y - n : y + n;
            } else {
                g_fx_pos[i].vy = AWAY(g_fx_pos[i].vy, rand() % 6 + 1);
            }
        } else {
            g_wave_delay[i]--;
            g_fx_pos[i].vz += 6;
        }
        RotMatrix(&g_fx_shift, &m);
        TransMatrix(&m, &g_fx_depth);
        SetRotMatrix(&m);
        SetTransMatrix(&m);
        RotTrans(&g_fx_pos[i], (VECTOR *)tm.t, &flag);
        RotMatrix(&g_fx_rots[i], &tm);
        MulMatrix(&tm, &m);
        SetRotMatrix(&tm);
        SetTransMatrix(&tm);
        if (i == 70) {
            bottom_left.vy = bottom_right.vy = 0;
        }
        if (RotNclip4(&g_fx_top_left, &g_fx_top_right, &bottom_left, &bottom_right,
                      (long *)&g_fx_tiles[i].x0, (long *)&g_fx_tiles[i].x1,
                      (long *)&g_fx_tiles[i].x2, (long *)&g_fx_tiles[i].x3,
                      &p, &otz, &flag) >= 0) {
            shift = 27 - Lzc(otz);
            if (shift < 0) {
                shift = 0;
            }
            GsSortPoly(&g_fx_tiles[i], &g_scene->world_ot[g_draw_buf], (u_short)(otz >> shift));
        }
    }
    VSync(2);
    FieldClockTick(2);
    GsSwapDispBuff();
    GsSortClear(0, 0, 0, &g_scene->world_ot[g_draw_buf]);
    GsDrawOt(&g_scene->world_ot[g_draw_buf]);
}
#else
INCLUDE_ASM("dng/nonmatchings/field/fieldfx", FieldFxStep);
#endif

/* Cuts the screen into tiles 32 pixels square (the bottom row 16 high),
   each textured from its own part of the display, and gives each a random
   delay and fall speed. */
#ifdef NON_MATCHING
void FieldFxSetup(void)
{
    POLY_FT4 *tile;
    SVECTOR  *pos;
    SVECTOR  *rot;
    int      *delay;
    int      *speed;
    int       row;
    int       v;
    int       y;
    int       col;
    int       u;
    u_char    ub;
    u_char    vb;

    speed = g_wave_speed;
    tile = g_fx_tiles;
    delay = g_wave_delay;
    y = -0x68;
    pos = g_fx_pos;
    rot = g_fx_rots;
    v = 0;
    for (row = 0; row < FX_ROWS; row++, v += 32, y += 32) {
        if (row == FX_ROWS - 1) {
            vb = v + 16;
        } else {
            vb = v + 32;
            if (vb == 0) {
                vb = v + 31;
            }
        }
        for (col = 0, u = 0; col < FX_COLS; col++, u += 32) {
            SetPolyFT4(tile);
            tile->tpage = GetTPage(2, 0, u & ~0xFF, v & ~0xFF);
            ub = (u_char)(u + 32) ? (u_char)(u + 32) : (u_char)(u + 31);
            tile->u1 = ub;
            tile->u3 = ub;
            tile->u0 = u;
            tile->v0 = v;
            tile->v1 = v;
            tile->u2 = u;
            tile->v2 = vb;
            tile->v3 = vb;
            tile->r0 = 0x80;
            tile->g0 = 0x80;
            tile->b0 = 0x80;
            *delay = rand() & 0x1F;
            pos->vx = u - 0x90;
            pos->vz = 0;
            pos->vy = y;
            rot->vx = rot->vy = rot->vz = 0;
            *speed = -((rand() % 3 + 2) << 8);
            tile++;
            pos++;
            rot++;
            delay++;
            speed++;
        }
    }
}
#else
INCLUDE_ASM("dng/nonmatchings/field/fieldfx", FieldFxSetup);
#endif
