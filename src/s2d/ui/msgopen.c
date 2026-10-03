/* Persona 1 (JP) - the field's message window.  S2D.
 *   0x80093680 S2dMsgOpen  0x8009376C S2dMsgClose  0x80093784 S2dMsgZoom
 *
 * Opening runs the script up to its first wait, then lets the window grow
 * from a sliver: wide first, then tall.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/s2d/s2d.h>

/* The window and its script: the script, the window's state block and
   whether it is still growing. */
extern int   D_800A4CEC;
extern int   D_800A4CE8;

/* The window's scale, x and y, and a third the map run sets. */
extern int D_800B93A0;
extern int D_800B93A4;
extern int D_800B93A8;

/* Steps the script took before it stopped. */
extern int D_800B0EE4;

extern void S2dWinInit();
extern int  func_8009AC18(S2dWin *w);

void S2dMsgOpen(int script)
{
    D_800A4CEC = script;
    S2dWinInit(5, &D_800B91EC, script, 0x1C0, 0x100, 0x200, 0xF8, 0xF, 1,
               -0xD0, 0x54);
    D_800B93A0 = 0x10;
    D_800B93A4 = 0x80;
    D_800B93A8 = 0x1000;
    D_800B0EE4 = 0;
    while (func_8009AC18(&D_800B91EC)) {
        D_800B0EE4++;
    }
    D_800B91EC.flags = 0x8000;
    D_800A4CE8 = 1;
}

void S2dMsgClose(void)
{
    D_800B91EC.flags = 0;
    D_800A4CE8 = 0;
}

void S2dMsgZoom(void)
{
    if (D_800A4CE8 == 1) {
        D_800B93A0 += 0x800;
        if (D_800B93A0 >= 0x1999) {
            D_800B93A0 = 0x1999;
            D_800B93A4 += 0x300;
            if (D_800B93A4 >= 0x1000) {
                D_800B93A4 = 0x1000;
                D_800A4CE8 = 0;
            }
        }
    }
}
