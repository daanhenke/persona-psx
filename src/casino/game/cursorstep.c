/* Persona 1 (JP) - initialize and step the active casino cursor display. */
#include <decomp/types.h>
extern s8 D_800AAE94[];
extern char D_800A96A4[], g_casino_cursor[];
extern int g_casino_timer;
extern void CasinoCursorRepeat();
extern void func_8008FF1C(void);
extern void func_8009019C(void *);
extern void func_80090DD4(void);
extern void func_8009065C(void);

void func_8008D620(void)
{
    if (g_casino_timer == 0) {
        D_800AAE94[1] = 1;
        D_800AAE94[0] = 1;
    }
    if (g_casino_timer >= 0) {
        CasinoCursorRepeat(g_casino_cursor, 2, 4);
        func_8008FF1C();
        func_8009019C(D_800A96A4);
        func_80090DD4();
        func_8009065C();
    }
}
