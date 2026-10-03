/* Persona 1 (JP) - the field's indicator.  S2D.
 *   0x8009B490 S2dIndicatorBar  0x8009B4A4 S2dIndicatorClear
 *   0x8009B4B4 S2dDrawIndicator 0x8009B5D0 S2dIndicatorState
 *   0x8009B5E0 S2dIndicatorHide 0x8009B614 S2dIndicatorShow
 *
 * BTLP's indicator (btlp/indicator.c) without its icon: blank, or a bar of
 * eleven cells whose lit end travels, uploaded every frame. Its model's
 * scale is what showing and hiding it sets.
 */
#include <decomp/types.h>
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
extern short   D_800AA66E;
extern u_short D_800AA670;
extern S2dXform D_800AA674;

extern short g_indicator_cells[];
extern short g_indicator_ramp[];

extern void VramQueueLoad(u_long *data, short x, short y, short w, short h);

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

void S2dIndicatorHide(void)
{
    D_800AA674.scale.vx = 0;
    D_800AA674.scale.vy = 0;
    D_800AA674.scale.vz = 0;
    D_800AA66E = 1;
    D_800AA670 = 1;
}

void S2dIndicatorShow(void)
{
    D_800AA674.scale.vx = ONE;
    D_800AA674.scale.vy = ONE;
    D_800AA674.scale.vz = ONE;
    D_800AA670 = 2;
}
