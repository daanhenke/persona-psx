/* Persona 1 (JP) - the menus' icon rows and the icon wheel.
 *
 * Compiled into DNG and ADV rather than called across the boundary. Each
 * overlay's wrapper (src/dng/ui/menuicons.c, src/adv/ui/menuicons.c) defines
 * MENUICONS_ASM_<routine> as the asm include of every routine that is not C
 * yet, under that overlay's name for it.
 *
 * A menu's commands stand as a row of sprite icons that turns with its
 * cursor: slot 40 onwards takes the cells of the entries from the cursor
 * round, the icon under the cursor first. Each kind of menu has its own table
 * of cells, (u, v) a pair; its entry 0 is the frame round the row, in slot
 * 45. The shade row (slots 56 on) is a strip of 16-pixel cells, four to a
 * row.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <persona/common/menuctx.h>
#include <persona/common/slot.h>

#define g_slots ((Slot *)0x800DC10C)

#define ICON_SLOT  40
#define FRAME_SLOT 45
#define SHADE_SLOT 56
#define WHEEL_SLOT 60

/* (u, v) per entry, flat; entry 0 is the frame. Six entries a kind for the
   five-icon row, four for three, three for two. */
extern u_short D_8009B0B4[];
extern u_short D_8009B0CC[];
extern u_short D_8009B10C[];

extern void SlotSetFlicker(u_char slot, u_char on);
extern void SlotInitTagged(void *def, u_char slot, int attr, short x, short y);

extern u_char D_8009AA2C[];
extern u_char D_8009AA4C[];
extern u_char D_8009AA8C[];
extern u_char D_8009AB58[];
extern u_char D_8009ABA8[];
extern u_char D_8009B074[];

extern u_char g_menu_blink;

void MenuIconsSet5(u_char kind);

/* One frame of the icon wheel opening: phase 0x1F puts the shade row and
   the frames up, phase 0 the icons, and every phase places the five shade
   sprites on their circle, further out as the count runs down. 0xFF is
   done. The angles are worked in int (`(g_menu_blink + 0x10) & 0xFF`): a
   u_char cast does it in QImode, and cse then shares the load the image
   makes again for every angle. */
int MenuWheelAnimIn(void)
{
    if (g_menu_blink != 0xFF) {
    if (g_menu_blink == 0x1F) {
        SlotInitTagged(D_8009AB58, 0x35, 0x300, 0x78, 0x40);
        SlotInitTagged(D_8009AA2C, 0x38, 0x300, 0, 0);
        SlotInitTagged(D_8009AA2C, 0x39, 0x300, 0, 0);
        SlotInitTagged(D_8009AA2C, 0x3A, 0x300, 0, 0);
        SlotInitTagged(D_8009AA2C, 0x3B, 0x300, 0, 0);
        SlotInitTagged(D_8009AA4C, 0x3C, 0x300, 0, 0);
        SlotInitTagged(D_8009AA4C, 0x3D, 0x300, 0xA0, 0x48);
        SlotInitTagged(D_8009B074, 0x2D, 0x2FF, 0x88, 0x41);
        g_slots[SHADE_SLOT + 0].u_add = 0x10;
        g_slots[SHADE_SLOT + 1].u_add = 0x20;
        g_slots[SHADE_SLOT + 2].u_add = 0x30;
        g_slots[SHADE_SLOT + 3].v_add = 0x10;
        g_slots[SHADE_SLOT + 4].u_add = 0x20;
    }
    if (g_menu_blink == 0) {
        SlotInitTagged(D_8009AA8C, 0x30, 0x300, 0x38, 0x1C);
        SlotInitTagged(D_8009AA8C, 0x31, 0x300, 0x28, 0x30);
        SlotInitTagged(D_8009AA8C, 0x32, 0x300, 0x18, 0x48);
        SlotInitTagged(D_8009AA8C, 0x33, 0x300, 8, 0x60);
        SlotInitTagged(D_8009ABA8, 0x2E, 0x2FF, 0xB2, 0x60);
        SlotInitTagged(D_8009B074, 0x28, 0x300, 0x38, 0x22);
        SlotInitTagged(D_8009B074, 0x29, 0x300, 0x28, 0x36);
        SlotInitTagged(D_8009B074, 0x2A, 0x300, 0x18, 0x4E);
        SlotInitTagged(D_8009B074, 0x2B, 0x300, 8, 0x66);
        SlotInitTagged(D_8009B074, 0x2C, 0x300, 0xBA, 0x67);
        g_slots[ICON_SLOT + 0].my = 6;
        g_slots[ICON_SLOT + 1].my = 6;
        g_slots[ICON_SLOT + 2].my = 6;
        g_slots[ICON_SLOT + 3].my = 6;
        g_slots[ICON_SLOT + 4].my = 6;
        MenuIconsSet5(0);
    }
    g_slots[SHADE_SLOT + 0].x = 0x78 - rsin(g_menu_blink * 16) / 24 * ((g_menu_blink & 0x1F) + 1) / 32;
    g_slots[SHADE_SLOT + 0].y = 0x24 - rcos(g_menu_blink * 16) / 24 * ((g_menu_blink & 0x1F) + 1) / 32;
    g_slots[SHADE_SLOT + 1].x = 0x68 - rsin(((g_menu_blink + 0x10) & 0xFF) * 16) / 24 * ((g_menu_blink & 0x1F) + 1) / 32;
    g_slots[SHADE_SLOT + 1].y = 0x38 - rcos(((g_menu_blink + 0x10) & 0xFF) * 16) / 24 * ((g_menu_blink & 0x1F) + 1) / 32;
    g_slots[SHADE_SLOT + 2].x = 0x58 - rsin(((g_menu_blink + 0x20) & 0xFF) * 16) / 24 * ((g_menu_blink & 0x1F) + 1) / 32;
    g_slots[SHADE_SLOT + 2].y = 0x4C - rcos(((g_menu_blink + 0x20) & 0xFF) * 16) / 24 * ((g_menu_blink & 0x1F) + 1) / 32;
    g_slots[SHADE_SLOT + 3].x = 0x48 - rsin(((g_menu_blink + 0x30) & 0xFF) * 16) / 24 * ((g_menu_blink & 0x1F) + 1) / 32;
    g_slots[SHADE_SLOT + 3].y = 0x64 - rcos(((g_menu_blink + 0x30) & 0xFF) * 16) / 24 * ((g_menu_blink & 0x1F) + 1) / 32;
    g_slots[SHADE_SLOT + 4].x = 0xD2 - rsin(((g_menu_blink + 0x80) & 0xFF) * 16) / 22 * ((g_menu_blink & 0x1F) + 1) / 32;
    g_slots[SHADE_SLOT + 4].y = 0x64 - rcos(((g_menu_blink + 0x80) & 0xFF) * 16) / 22 * ((g_menu_blink & 0x1F) + 1) / 32;
    g_menu_blink--;
    return 0;
    }
    return 1;
}

/* The wheel closing: the same circle run the other way, from the frame
   after the icons are cleared (0xFF) until the count reaches 0x20. */
int MenuWheelAnimOut(void)
{
    if (g_menu_blink != 0x20) {
        if (g_menu_blink == 0xFF) {
            g_menu_blink = 0;
            SlotClear(0x28);
            SlotClear(0x29);
            SlotClear(0x2A);
            SlotClear(0x2B);
            SlotClear(0x2C);
            SlotClear(0x2E);
            SlotClear(0x30);
            SlotClear(0x31);
            SlotClear(0x32);
            SlotClear(0x33);
            SlotClear(0x34);
            SlotClear(0x36);
        }
        g_slots[SHADE_SLOT + 0].x = 0x80 - rsin(g_menu_blink * 16) / 24 * ((g_menu_blink & 0x1F) + 1) / 32;
        g_slots[SHADE_SLOT + 0].y = 0x18 - rcos(g_menu_blink * 16) / 24 * ((g_menu_blink & 0x1F) + 1) / 32;
        g_slots[SHADE_SLOT + 1].x = 0x68 - rsin(((g_menu_blink + 0x10) & 0xFF) * 16) / 24 * ((g_menu_blink & 0x1F) + 1) / 32;
        g_slots[SHADE_SLOT + 1].y = 0x30 - rcos(((g_menu_blink + 0x10) & 0xFF) * 16) / 24 * ((g_menu_blink & 0x1F) + 1) / 32;
        g_slots[SHADE_SLOT + 2].x = 0x58 - rsin(((g_menu_blink + 0x20) & 0xFF) * 16) / 24 * ((g_menu_blink & 0x1F) + 1) / 32;
        g_slots[SHADE_SLOT + 2].y = 0x48 - rcos(((g_menu_blink + 0x20) & 0xFF) * 16) / 24 * ((g_menu_blink & 0x1F) + 1) / 32;
        g_slots[SHADE_SLOT + 3].x = 0x48 - rsin(((g_menu_blink + 0x30) & 0xFF) * 16) / 24 * ((g_menu_blink & 0x1F) + 1) / 32;
        g_slots[SHADE_SLOT + 3].y = 0x60 - rcos(((g_menu_blink + 0x30) & 0xFF) * 16) / 24 * ((g_menu_blink & 0x1F) + 1) / 32;
        g_slots[SHADE_SLOT + 4].x = 0xD0 - rsin(((g_menu_blink + 0x80) & 0xFF) * 16) / 22 * ((g_menu_blink & 0x1F) + 1) / 32;
        g_slots[SHADE_SLOT + 4].y = 0x68 - rcos(((g_menu_blink + 0x80) & 0xFF) * 16) / 22 * ((g_menu_blink & 0x1F) + 1) / 32;
        g_menu_blink++;
        return 0;
    }
    return 1;
}

/* The wheel at rest, as MenuBuild puts it back when the page is redrawn
   with the wheel open (g_menu_blink 0xFF). */
void MenuWheelShow(void)
{
    int a;

    SlotInitTagged(D_8009AA4C, 0x3D, 0x300, 0xA0, 0x48);
    SlotInitTagged(D_8009AB58, 0x35, 0x300, 0x78, 0x40);
    SlotInitTagged(D_8009B074, 0x2D, 0x2FF, 0x88, 0x41);
    SlotInitTagged(D_8009AA2C, 0x38, 0x300, 0, 0);
    SlotInitTagged(D_8009AA2C, 0x39, 0x300, 0, 0);
    SlotInitTagged(D_8009AA2C, 0x3A, 0x300, 0, 0);
    SlotInitTagged(D_8009AA2C, 0x3B, 0x300, 0, 0);
    SlotInitTagged(D_8009AA4C, 0x3C, 0x300, 0, 0);
    {
        int d = rsin(0) / 24;

        g_slots[SHADE_SLOT + 0].x = 0x78 - d / 32;
    }
    {
        int d = rcos(0) / 24;

        g_slots[SHADE_SLOT + 0].y = 0x24 - d / 32;
    }
    g_slots[SHADE_SLOT + 0].u_add = 0x10;
    a = 0x10;
    {
        int d = rsin(a * 16) / 24;

        g_slots[SHADE_SLOT + 1].x = 0x68 - d / 32;
    }
    {
        int d = rcos(a * 16) / 24;

        g_slots[SHADE_SLOT + 1].y = 0x38 - d / 32;
    }
    g_slots[SHADE_SLOT + 1].u_add = 0x20;
    a = 0x20;
    {
        int d = rsin(a * 16) / 24;

        g_slots[SHADE_SLOT + 2].x = 0x58 - d / 32;
    }
    {
        int d = rcos(a * 16) / 24;

        g_slots[SHADE_SLOT + 2].y = 0x4C - d / 32;
    }
    g_slots[SHADE_SLOT + 2].u_add = 0x30;
    a = 0x30;
    {
        int d = rsin(a * 16) / 24;

        g_slots[SHADE_SLOT + 3].x = 0x48 - d / 32;
    }
    {
        int d = rcos(a * 16) / 24;

        g_slots[SHADE_SLOT + 3].y = 0x64 - d / 32;
    }
    g_slots[SHADE_SLOT + 3].v_add = 0x10;
    a = 0x80;
    {
        int d = rsin(a * 16) / 22;

        g_slots[SHADE_SLOT + 4].x = 0xD2 - d / 32;
    }
    {
        int d = rcos(a * 16) / 22;

        g_slots[SHADE_SLOT + 4].y = 0x64 - d / 32;
    }
    g_slots[SHADE_SLOT + 4].u_add = 0x20;
    SlotInitTagged(D_8009AA8C, 0x30, 0x300, 0x38, 0x1C);
    SlotInitTagged(D_8009AA8C, 0x31, 0x300, 0x28, 0x30);
    SlotInitTagged(D_8009AA8C, 0x32, 0x300, 0x18, 0x48);
    SlotInitTagged(D_8009AA8C, 0x33, 0x300, 8, 0x60);
    SlotInitTagged(D_8009ABA8, 0x2E, 0x2FF, 0xB2, 0x60);
    SlotInitTagged(D_8009B074, 0x28, 0x300, 0x38, 0x22);
    SlotInitTagged(D_8009B074, 0x29, 0x300, 0x28, 0x36);
    SlotInitTagged(D_8009B074, 0x2A, 0x300, 0x18, 0x4E);
    SlotInitTagged(D_8009B074, 0x2B, 0x300, 8, 0x66);
    SlotInitTagged(D_8009B074, 0x2C, 0x300, 0xBA, 0x67);
    g_slots[ICON_SLOT + 0].my = 6;
    g_slots[ICON_SLOT + 1].my = 6;
    g_slots[ICON_SLOT + 2].my = 6;
    g_slots[ICON_SLOT + 3].my = 6;
    g_slots[ICON_SLOT + 4].my = 6;
    MenuIconsSet5(0);
}

/* Opens the icon wheel on entry `cur`: the frames, the row and its three
   shade sprites, each of the three set on its circle by angle. `kind` is
   not read. */
/* Each coordinate's quotient is its own block-scoped temporary, so the
   rounding of the second division ties to it (a shared `d` is global and
   works on a copy); the angle is a variable, and cse keeps its register for
   the later constants of the same value, as the image does. */
void MenuWheelOpen(int kind, short cur)
{
    int a;

    SlotInitTagged(D_8009AA8C, 0x30, 0x300, 0x28, 0x30);
    SlotInitTagged(D_8009AA8C, 0x31, 0x300, 0x18, 0x48);
    SlotInitTagged(D_8009ABA8, 0x2E, 0x2FF, 0xB2, 0x60);
    SlotInitTagged(D_8009B074, 0x28, 0x300, 0x28, 0x36);
    SlotInitTagged(D_8009B074, 0x29, 0x300, 0x18, 0x4E);
    SlotInitTagged(D_8009B074, 0x2A, 0x300, 0xD2, 0x67);
    g_slots[ICON_SLOT + 0].my = 6;
    g_slots[ICON_SLOT + 1].my = 6;
    g_slots[ICON_SLOT + 2].my = 6;
    g_slots[ICON_SLOT + 2].mx = 0x18;
    SlotInitTagged(D_8009AB58, 0x35, 0x300, 0x78, 0x40);
    SlotInitTagged(D_8009AA2C, 0x38, 0x300, 0, 0);
    SlotInitTagged(D_8009AA2C, 0x39, 0x300, 0, 0);
    SlotInitTagged(D_8009AA4C, 0x3A, 0x300, 0, 0);
    g_slots[WHEEL_SLOT].u_add = cur * 32;
    a = 0x10;
    {
        int d = rsin(a * 16) / 24;

        g_slots[SHADE_SLOT + 0].x = 0x68 - d / 32;
    }
    {
        int d = rcos(a * 16) / 24;

        g_slots[SHADE_SLOT + 0].y = 0x38 - d / 32;
    }
    g_slots[SHADE_SLOT + 0].u_add = 0x10;
    g_slots[SHADE_SLOT + 0].v_add = 0x10;
    a = 0x20;
    {
        int d = rsin(a * 16) / 24;

        g_slots[SHADE_SLOT + 1].x = 0x58 - d / 32;
    }
    {
        int d = rcos(a * 16) / 24;

        g_slots[SHADE_SLOT + 1].y = 0x4C - d / 32;
    }
    g_slots[SHADE_SLOT + 1].u_add = 0x10;
    g_slots[SHADE_SLOT + 1].v_add = 0x10;
    a = 0x80;
    {
        int d = rsin(a * 16) / 22;

        g_slots[SHADE_SLOT + 2].x = 0xD2 - d / 32;
    }
    {
        int d = rcos(a * 16) / 22;

        g_slots[SHADE_SLOT + 2].y = 0x64 - d / 32;
    }
}

/* Per frame of a turn, each shade sprite's offset from its place on the
   circle: [sprite][frame][x, y], sprite 1 and 2 the two row sprites and 0
   the one on the right. */
extern short g_wheel_turn[3][8][2];
extern int   g_pad_held;

extern void RunFrame(void);

void MenuIconsSet3(u_char kind, u_char list);

/* Turns the three-icon wheel a step over eight frames, backwards while the
   left shoulder buttons are held; the icons change half way.

   Not matched (67.65%). Found so far: MenuIconsSet3's prototype in scope
   (the image passes the low byte of each spilled short parameter), the
   angle set ahead of the i == 4 call so cse loses its constant, and a
   multiplier variable of 1 that the image loads afresh per section. Open:
   loop.c here folds every table read into one reduced pointer, where the
   image reduces only slot 56's x and reads the rest from a base register
   (table + 0x20, hoisted) plus i * 4, slot 58's y from the symbol; fold
   also turns `t + 0x68 - d` into `t - (d - 0x68)`; and the image clears a
   saved register at entry that nothing reads. */
#ifdef NON_MATCHING
void func_80086E20(short kind, short list)
{
    int i;
    int a;
    int m;
    short (*t)[8][2];

    m = 1;
    t = g_wheel_turn;
    g_slots[ICON_SLOT + 0].scale_y = 0x100;
    g_slots[ICON_SLOT + 1].scale_y = 0x100;
    g_slots[ICON_SLOT + 2].scale_y = 0x100;
    if (g_pad_held & 0x6000) {
        for (i = 7; i >= 0; i--) {
            a = 0x100;
            if (i == 4) {
                MenuIconsSet3(kind, list);
            }
            g_slots[SHADE_SLOT + 0].x = t[1][i][0] + 0x68 - rsin(a) / 24 * m / 32;
            g_slots[SHADE_SLOT + 0].y = t[1][i][1] + 0x38 - rcos(a) / 24 * m / 32;
            a = 0x200;
            g_slots[SHADE_SLOT + 1].x = t[2][i][0] + 0x58 - rsin(a) / 24 * m / 32;
            g_slots[SHADE_SLOT + 1].y = t[2][i][1] + 0x4C - rcos(a) / 24 * m / 32;
            a = 0x800;
            g_slots[SHADE_SLOT + 2].x = t[0][i][0] + 0xD2 - rsin(a) / 22 * m / 32;
            g_slots[SHADE_SLOT + 2].y = t[0][i][1] + 0x64 - rcos(a) / 22 * m / 32;
            RunFrame();
        }
    } else {
        for (i = 0; i < 8; i++) {
            a = 0x100;
            if (i == 4) {
                MenuIconsSet3(kind, list);
            }
            g_slots[SHADE_SLOT + 0].x = t[1][i][0] + 0x68 - rsin(a) / 24 * m / 32;
            g_slots[SHADE_SLOT + 0].y = t[1][i][1] + 0x38 - rcos(a) / 24 * m / 32;
            a = 0x200;
            g_slots[SHADE_SLOT + 1].x = t[2][i][0] + 0x58 - rsin(a) / 24 * m / 32;
            g_slots[SHADE_SLOT + 1].y = t[2][i][1] + 0x4C - rcos(a) / 24 * m / 32;
            a = 0x800;
            g_slots[SHADE_SLOT + 2].x = t[0][i][0] + 0xD2 - rsin(a) / 22 * m / 32;
            g_slots[SHADE_SLOT + 2].y = t[0][i][1] + 0x64 - rcos(a) / 22 * m / 32;
            RunFrame();
        }
    }
    a = 0x100;
    g_slots[SHADE_SLOT + 0].x = 0x68 - rsin(a) / 24 * m / 32;
    g_slots[SHADE_SLOT + 0].y = 0x38 - rcos(a) / 24 * m / 32;
    g_slots[SHADE_SLOT + 0].u_add = 0x10;
    g_slots[SHADE_SLOT + 0].v_add = 0x10;
    a = 0x200;
    g_slots[SHADE_SLOT + 1].x = 0x58 - rsin(a) / 24 * m / 32;
    g_slots[SHADE_SLOT + 1].y = 0x4C - rcos(a) / 24 * m / 32;
    g_slots[SHADE_SLOT + 1].u_add = 0x10;
    g_slots[SHADE_SLOT + 1].v_add = 0x10;
    a = 0x800;
    g_slots[SHADE_SLOT + 2].x = 0xD2 - rsin(a) / 22 * m / 32;
    g_slots[SHADE_SLOT + 2].y = 0x64 - rcos(a) / 22 * m / 32;
    g_slots[ICON_SLOT + 0].scale_y = 0x1000;
    g_slots[ICON_SLOT + 1].scale_y = 0x1000;
    g_slots[ICON_SLOT + 2].scale_y = 0x1000;
}
#else
MENUICONS_ASM_5;
#endif

/* The two-icon wheel opened on entry `cur`, as MenuWheelOpen does the
   three. `kind` is not read. */
void MenuWheelOpen2(int kind, short cur)
{
    int a;

    SlotInitTagged(D_8009AB58, 0x35, 0x300, 0x78, 0x40);
    SlotInitTagged(D_8009B074, 0x2D, 0x2FF, 0x88, 0x41);
    SlotInitTagged(D_8009AA2C, 0x38, 0x300, 0, 0);
    SlotInitTagged(D_8009AA4C, 0x39, 0x300, 0, 0);
    a = 0x20;
    {
        int d = rsin(a * 16) / 24;

        g_slots[SHADE_SLOT + 0].x = 0x58 - d / 32;
    }
    {
        int d = rcos(a * 16) / 24;

        g_slots[SHADE_SLOT + 0].y = 0x4C - d / 32;
    }
    g_slots[SHADE_SLOT + 0].u_add = 0x10;
    g_slots[SHADE_SLOT + 0].v_add = 0x10;
    a = 0x80;
    {
        int d = rsin(a * 16) / 22;

        g_slots[SHADE_SLOT + 1].x = 0xD2 - d / 32;
    }
    {
        int d = rcos(a * 16) / 22;

        g_slots[SHADE_SLOT + 1].y = 0x64 - d / 32;
    }
    SlotInitTagged(D_8009AA8C, 0x30, 0x300, 0x18, 0x48);
    SlotInitTagged(D_8009ABA8, 0x2E, 0x2FF, 0xB2, 0x60);
    SlotInitTagged(D_8009B074, 0x28, 0x300, 0x18, 0x4E);
    SlotInitTagged(D_8009B074, 0x29, 0x300, 0xBA, 0x67);
    g_slots[ICON_SLOT + 0].my = 6;
    g_slots[ICON_SLOT + 1].my = 6;
    g_slots[WHEEL_SLOT].u_add = cur * 32;
}

MENUICONS_ASM_6B;

MENUICONS_ASM_UPDATE;

MENUICONS_ASM_8;

void MenuIconsShade(void);

void MenuIconsSet5(u_char kind)
{
    u_char i;
    u_char k;

    for (i = 0; i < 5; i++) {
        k = (g_menu->top.cur + i) % 5 + 1;
        g_slots[ICON_SLOT + i].u_add = D_8009B0B4[kind * 12 + k * 2];
        g_slots[ICON_SLOT + i].v_add = D_8009B0B4[kind * 12 + k * 2 + 1];
        g_slots[WHEEL_SLOT].u_add = g_menu->top.cur * 32 + 0x20;
        MenuIconsShade();
    }
    SlotSetFlicker(0x2C, 1);
}

void MenuIconsSet3(u_char kind, u_char list)
{
    u_char i;
    u_char k;

    g_slots[FRAME_SLOT].u_add = D_8009B0CC[kind * 8];
    g_slots[FRAME_SLOT].v_add = D_8009B0CC[kind * 8 + 1];
    for (i = 0; i < 3; i++) {
        k = ((&g_menu->top)[list].cur + i) % 3 + 1;
        g_slots[ICON_SLOT + i].u_add = D_8009B0CC[kind * 8 + k * 2];
        g_slots[ICON_SLOT + i].v_add = D_8009B0CC[kind * 8 + k * 2 + 1];
    }
    SlotSetFlicker(0x2A, 1);
}

void MenuIconsSet2(u_char kind, u_char list)
{
    u_char i;
    u_char k;

    g_slots[FRAME_SLOT].u_add = D_8009B10C[kind * 6];
    g_slots[FRAME_SLOT].v_add = D_8009B10C[kind * 6 + 1];
    for (i = 0; i < 2; i++) {
        k = ((&g_menu->top)[list].cur + i) % 2 + 1;
        g_slots[ICON_SLOT + i].u_add = D_8009B10C[kind * 6 + k * 2];
        g_slots[ICON_SLOT + i].v_add = D_8009B10C[kind * 6 + k * 2 + 1];
    }
    SlotSetFlicker(0x29, 1);
}

void MenuIconsShade(void)
{
    u_char i;
    u_int  k;
    Slot  *s;

    for (i = 0; i < 4; i++) {
        s = &g_slots[SHADE_SLOT + i];
        k = (u_char)((g_menu->top.cur + i + 1) % 5);
        s->u_add = (k & 3) * 16;
        s->v_add = (k >> 2) * 16;
    }
}

/* 97.98%: k and the division's working copy trade a0/v1 against the
   image. */
#ifdef NON_MATCHING
void MenuIconsShadeFrom(u_char start, u_char n)
{
    u_char i;
    int    k;

    for (i = 0; i < n; i++) {
        k = (start + i + 1) % 5;
        g_slots[SHADE_SLOT + i].u_add = (k % 4) * 16;
        g_slots[SHADE_SLOT + i].v_add = (k / 4) * 16;
    }
}
#else
MENUICONS_ASM_SHADEFROM;
#endif
