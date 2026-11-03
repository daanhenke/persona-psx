/* Persona 1 (JP) - the field's indicator.  S2D.
 *   0x8009B490 S2dIndicatorBar  0x8009B4A4 S2dIndicatorClear
 *   0x8009B4B4 S2dDrawIndicator 0x8009B5D0 S2dIndicatorState
 *   0x8009B5E0 S2dIndicatorOpen 0x8009B614 S2dIndicatorClose
 *   0x8009B644 S2dIndicatorStep 0x8009B758 (draw)
 *   0x8009BB8C S2dLoadWinColors 0x8009BBF4 S2dEventFlagSet
 *
 * BTLP's indicator (btlp/indicator.c) without its icon: blank, or a bar of
 * eleven cells whose lit end travels, uploaded every frame. Its model opens
 * by growing from nothing, wide first, and closes by shrinking, tall first.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/s2d/s2d.h>

#define INDICATOR_OFF 0
#define INDICATOR_BAR 1

#define INDICATOR_CELLS  11
#define INDICATOR_BLANK  0xCE
#define INDICATOR_PERIOD 0x38

extern short   g_indicator_mode;
extern u_short g_indicator_phase;
extern u_short D_800AA66E;
extern u_short D_800AA670;
extern S2dXform D_800AA674;

extern short g_indicator_cells[];
extern short g_indicator_ramp[];

extern void VramQueueLoad(u_long *data, short x, short y, short w, short h);

/* Its model's state: 1 opening, 2 closing, 0 still; and whether it is
   open. */
#define D_800AA670_OPENING 1
#define D_800AA670_CLOSING 2

/* The window colour option, the colour tables, and the window image. */
extern u_char D_801F2AC6;
extern u_char D_800A8910[];
extern u_char D_800A8A10[];

#define g_event_flags ((u_char *)0x801F29C8)

void S2dIndicatorBar(void)
{
    g_indicator_mode = INDICATOR_BAR;
}

void S2dIndicatorClear(void)
{
    g_indicator_mode = INDICATOR_OFF;
}

void S2dDrawIndicator(void)
{
    SPRT    sprt;    /* BTLP's icon's, which S2D's copy does not draw */
    DR_MODE mode;
    short  *cell;
    short   blank;
    int     i;

    switch (g_indicator_mode) {
    case INDICATOR_OFF:
        blank = INDICATOR_BLANK;
        i = INDICATOR_CELLS - 1;
        cell = &g_indicator_cells[INDICATOR_CELLS - 1];
        do {
            *cell = blank;
            i--;
            cell--;
        } while (i >= 0);
        g_indicator_phase = 0;
        break;
    case INDICATOR_BAR:
        i = 0;
        do {
            g_indicator_cells[(INDICATOR_CELLS - 1) - i] =
                g_indicator_ramp[i + (g_indicator_phase >> 1)];
            i++;
        } while (i < INDICATOR_CELLS);
        g_indicator_phase = (g_indicator_phase + 1) % INDICATOR_PERIOD;
        break;
    }
    VramQueueLoad((u_long *)g_indicator_cells, 0x220, 0xF0, INDICATOR_CELLS, 1);
}

u_short S2dIndicatorState(void)
{
    return D_800AA670;
}

void S2dIndicatorOpen(void)
{
    D_800AA674.scale.vx = 0;
    D_800AA674.scale.vy = 0;
    D_800AA674.scale.vz = 0;
    D_800AA66E = 1;
    D_800AA670 = 1;
}

void S2dIndicatorClose(void)
{
    D_800AA674.scale.vx = ONE;
    D_800AA674.scale.vy = ONE;
    D_800AA674.scale.vz = ONE;
    D_800AA670 = 2;
}

void S2dIndicatorStep(void)
{
    switch (D_800AA670) {
    case 0:
        break;
    case D_800AA670_OPENING:
        if ((D_800AA674.scale.vx += 0x100) > ONE) {
            D_800AA674.scale.vx = ONE;
        }
        if ((D_800AA674.scale.vy += 0x80) > ONE) {
            D_800AA674.scale.vy = ONE;
        }
        if (D_800AA674.scale.vx >= ONE && D_800AA674.scale.vy >= ONE) {
            D_800AA670 = 0;
        }
        break;
    case D_800AA670_CLOSING:
        if ((D_800AA674.scale.vx -= 0x80) < 0) {
            D_800AA674.scale.vx = 0;
        }
        if ((D_800AA674.scale.vy -= 0x100) < 0) {
            D_800AA674.scale.vy = 0;
        }
        if (D_800AA674.scale.vx == 0 && D_800AA674.scale.vy == 0) {
            D_800AA66E = 0;
            D_800AA670 = 0;
        }
        break;
    }
}

/* The draw uses the transform fields as separate address constants. These
   aliases preserve those references without duplicating their storage. */
extern VECTOR g_indicator_translation __asm__("D_800AA674+8");
extern VECTOR g_indicator_scale __asm__("D_800AA674+24");
extern u_char D_800AA69C[];
#define WINDOW_COLOR (*(volatile u_char *)0x801F2AC6)

void func_8009B758(GsOT *ot, u_short z)
{
    /* 32 bytes of vertices, 32 of matrix, then a 40-byte polygon. */
    u_long work[26];
    SVECTOR *corner = (SVECTOR *)work;
#define INDICATOR_CORNER(n) (*(SVECTOR *)(((n) << 3) + (u_char *)corner))
#define INDICATOR_MATRIX (*(MATRIX *)((u_char *)corner + 32))
#define INDICATOR_POLY (*(POLY_FT4 *)((u_char *)corner + 64))
    long old_x, old_y;
    long j;
    int i;
    if (D_800AA66E) {
        i = 0;
        S2dDrawIndicator();
        SetPolyFT4(&INDICATOR_POLY);
        SetSemiTrans(&INDICATOR_POLY, 0);
        SetShadeTex(&INDICATOR_POLY, 1);
        INDICATOR_POLY.clut = GetClut(0x200, 0xF0);
        INDICATOR_POLY.tpage = GetTPage(1, 0, 0x200, 0x100);
        ReadGeomOffset(&old_x, &old_y);
        SetGeomOffset(-0x60, 0x4C);
        RotMatrix(&D_800AA674.rot, &INDICATOR_MATRIX);
        TransMatrix(&INDICATOR_MATRIX, &g_indicator_translation);
        ScaleMatrix(&INDICATOR_MATRIX, &g_indicator_scale);
        SetRotMatrix(&INDICATOR_MATRIX);
        SetTransMatrix(&INDICATOR_MATRIX);
        /* Eighteen columns of cells; blank cells leave holes in the grid. */
        for (; i < 90; i++) {
            if (D_800AA69C[i] != 0xFF) {
                for (j = 0; j < 4; j++) {
                    INDICATOR_CORNER(j).vx =
                        (i % 18) * 16 + (short)((j & 1) * 16 - 0x90);
                    INDICATOR_CORNER(j).vy =
                        (i / 18) * 16 + (short)((j / 2) * 16 - 0x28);
                    INDICATOR_CORNER(j).vz = 0;
                }
                RotTransPers4(&corner[0], &corner[1], &corner[2], &corner[3],
                              (long *)&INDICATOR_POLY.x0,
                              (long *)&INDICATOR_POLY.x1,
                              (long *)&INDICATOR_POLY.x2,
                              (long *)&INDICATOR_POLY.x3, &j, &j);
                INDICATOR_POLY.u0 = (D_800AA69C[i] & 7) << 4;
                INDICATOR_POLY.v0 = (D_800AA69C[i] >> 3) << 4;
                INDICATOR_POLY.u1 = ((D_800AA69C[i] & 7) << 4) + 16;
                INDICATOR_POLY.v1 = (D_800AA69C[i] >> 3) << 4;
                INDICATOR_POLY.u2 = (D_800AA69C[i] & 7) << 4;
                INDICATOR_POLY.v2 = ((D_800AA69C[i] >> 3) << 4) + 16;
                INDICATOR_POLY.u3 = ((D_800AA69C[i] & 7) << 4) + 16;
                INDICATOR_POLY.v3 = ((D_800AA69C[i] >> 3) << 4) + 16;
                GsSortPoly(&INDICATOR_POLY, ot, z);
            }
        }
        SetPolyFT4(&INDICATOR_POLY);
        SetSemiTrans(&INDICATOR_POLY, 1);
        SetShadeTex(&INDICATOR_POLY, 1);
        INDICATOR_POLY.clut = GetClut(0x200, 0xF2);
        INDICATOR_POLY.tpage = GetTPage(0, 0, 0x280, 0x140);
        /* Nine background strips, each 32 by 56 pixels. */
        for (i = 0; i < 9; i++) {
            for (j = 0; j < 4; j++) {
                INDICATOR_CORNER(j).vx = i * 32 + (short)((j & 1) * 32 - 0x90);
                INDICATOR_CORNER(j).vy = (j / 2) * 56 - 0x20;
                INDICATOR_CORNER(j).vz = 0;
            }
            RotTransPers4(
                &corner[0], &corner[1], &corner[2], &corner[3],
                (long *)&INDICATOR_POLY.x0, (long *)&INDICATOR_POLY.x1,
                (long *)&INDICATOR_POLY.x2, (long *)&INDICATOR_POLY.x3, &j, &j);
            INDICATOR_POLY.u0 = WINDOW_COLOR << 5;
            INDICATOR_POLY.v0 = 0x40;
            INDICATOR_POLY.u1 = (WINDOW_COLOR << 5) + 0x1F;
            INDICATOR_POLY.v1 = 0x40;
            INDICATOR_POLY.u2 = WINDOW_COLOR << 5;
            INDICATOR_POLY.v2 = 0x78;
            INDICATOR_POLY.u3 = (WINDOW_COLOR << 5) + 0x1F;
            INDICATOR_POLY.v3 = 0x78;
            GsSortPoly(&INDICATOR_POLY, ot, z);
        }
        SetGeomOffset(old_x, old_y);
    }
}

#undef INDICATOR_MATRIX
#undef INDICATOR_POLY
#undef INDICATOR_CORNER
#undef WINDOW_COLOR

/* The window's sixteen colours for the option chosen, and its image. */
void S2dLoadWinColors(void)
{
    VramQueueLoad((u_long *)(D_800A8910 + D_801F2AC6 * 32), 0x200, 0xF2, 0x10,
                  1);
    VramQueueLoad((u_long *)D_800A8A10, 0x280, 0x140, 0x40, 0x38);
}

void S2dEventFlagSet(u_short id)
{
    u_char *p;

    p = &g_event_flags[id >> 3];
    *p |= 1 << (id & 7);
}
