/* Persona 1 (JP) - CASINO's frame bookkeeping and its empty debug hooks.
 *   0x80066EA4 CasinoDummy0 .. CasinoDummy3
 *   0x80066EC4 CasinoPlayTimeTick
 *   0x80066F60 CasinoFrameWrap
 *   0x80066F88 CasinoDebugInit
 *   0x80066F90 CasinoDummy4
 *   0x80066F98 CasinoPrintf
 *   0x80066FAC CasinoDummy5
 *
 * The empty routines are what a debug build's hooks compile to in a retail
 * one; only CasinoDebugInit is still called.
 */
#include <decomp/types.h>
#include <persona/casino/casino.h>

extern u_char g_casino_wrapped;

/* Frames in two hours. */
#define WRAP_FRAMES 432000

void CasinoDummy0(void)
{
}

void CasinoDummy1(void)
{
}

void CasinoDummy2(void)
{
}

void CasinoDummy3(void)
{
}

/* Hours, minutes, seconds, frames. Each place rolls over once it has
   counted past 60, and the clock stops for good at 99:59. */
void CasinoPlayTimeTick(u_char *t)
{
    u_char n;

    n = t[3]++;
    if (n == 60) {
        t[3] = 0;
        n = t[2]++;
        if (n == 60) {
            t[2] = 0;
            n = t[1]++;
            if (n == 60) {
                t[1] = 0;
                t[0]++;
            }
        }
    }
    if (t[1] == 59 && t[0] == 99) {
        g_casino_clock_on = 0;
        t[3] = 59;
        t[2] = 59;
        t[1] = 59;
        t[0] = 99;
    }
}

void CasinoFrameWrap(int *frame)
{
    if (*frame == WRAP_FRAMES) {
        g_casino_wrapped = 1;
        *frame = 0;
    }
}

void CasinoDebugInit(void)
{
}

void CasinoDummy4(void)
{
}

/* A debug print with its body compiled out. Each argument still has its
   address taken, so each still goes to its stack slot. */
void CasinoPrintf(char *fmt, int a, int b, int c)
{
    void *p;

    p = &fmt;
    p = &a;
    p = &b;
    p = &c;
}

void CasinoDummy5(void)
{
}
