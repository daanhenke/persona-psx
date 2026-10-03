/* Persona 1 (JP) - the field's indicator.  S2D.
 *   0x8009B490 S2dIndicatorBar  0x8009B4A4 S2dIndicatorClear
 *   0x8009B4B4 S2dDrawIndicator 0x8009B5D0 S2dIndicatorState
 *   0x8009B5E0 S2dIndicatorOpen 0x8009B614 S2dIndicatorClose
 *   0x8009B644 S2dIndicatorStep 0x8009B758 (asm)
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
extern short   D_800AA66E;
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

INCLUDE_ASM("s2d/nonmatchings/ui/indicator", func_8009B758);

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
