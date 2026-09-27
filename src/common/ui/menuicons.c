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

MENUICONS_ASM_2;

MENUICONS_ASM_3;

/* Opens the icon wheel on entry `cur`: the frames, the row and its three
   shade sprites, each of the three set on its circle by angle. `kind` is
   not read. */
/* 95.71%: the image divides by 24 (or 22) and then by 32 on the same
   register, the second in place; written as one expression fold makes one
   division of them, and through `d` the second works on a copy. */
#ifdef NON_MATCHING
void MenuWheelOpen(int kind, short cur)
{
    int d;

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
    d = rsin(0x100) / 24;
    g_slots[SHADE_SLOT + 0].x = 0x68 - d / 32;
    d = rcos(0x100) / 24;
    g_slots[SHADE_SLOT + 0].y = 0x38 - d / 32;
    g_slots[SHADE_SLOT + 0].u_add = 0x10;
    g_slots[SHADE_SLOT + 0].v_add = 0x10;
    d = rsin(0x200) / 24;
    g_slots[SHADE_SLOT + 1].x = 0x58 - d / 32;
    d = rcos(0x200) / 24;
    g_slots[SHADE_SLOT + 1].y = 0x4C - d / 32;
    g_slots[SHADE_SLOT + 1].u_add = 0x10;
    g_slots[SHADE_SLOT + 1].v_add = 0x10;
    d = rsin(0x800) / 22;
    g_slots[SHADE_SLOT + 2].x = 0xD2 - d / 32;
    d = rcos(0x800) / 22;
    g_slots[SHADE_SLOT + 2].y = 0x64 - d / 32;
}
#else
MENUICONS_ASM_WHEELOPEN;
#endif

MENUICONS_ASM_5;

MENUICONS_ASM_6;

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
