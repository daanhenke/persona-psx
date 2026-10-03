/* Persona 1 (JP) - the map image's pages.  S2D.
 *   0x80093458 S2dMapPageSwap  0x80093510 S2dMapPageCheck
 *   0x800935C8 S2dMapPagesFlip
 *
 * The map's picture is taller than the VRAM it is given: four 64-row pages
 * from row 0x100 hold one half while the other waits at 0x80190000, a page
 * every 0x10000. Crossing row 0x90 or wrapping round 0xC7 asks for the
 * halves to be exchanged, which the frame's end does page by page.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/s2d/s2d.h>

#define PAGE_BUF   ((u_long *)0x800D0000)
#define PAGE_AT(n) ((u_long *)(0x80190000 + ((n) << 16)))

/* A map row counted from the bottom. */
#define FLIP(y) (0xC7 - (y))

/* The page being exchanged (3 asks for all four, counting down), and each
   page's height. */
extern int    g_map_page;
extern u_char g_map_page_h[];

/* The party's row now and before. */
extern short D_800B8FD8;
extern short D_800B91E4;

extern void VramStore();
extern void VramLoad();
extern void CopyLongs(u_long *dst, u_long *src, int n);

#ifdef NON_MATCHING
/* One page out of VRAM into the work buffer, the waiting one in, and the
   buffer kept for next time. */
void S2dMapPageSwap(void)
{
    int     page;
    short   y;
    int     h;
    u_long *p;
    int     n;

    page = g_map_page;
    y = page * 64 + 0x100;
    h = g_map_page_h[page];
    p = PAGE_AT(page);
    n = h << 10;
    VramStore(0x200, y, 0x200, h, PAGE_BUF);
    VramLoad(0x200, y, 0x200, h, p);
    DrawSync(0);
    CopyLongs(p, PAGE_BUF, n);
}
#else
/* 89%: the image forms n before the first call, in a saved register of its
   own; here n is set once, so sched1's birthing boost sinks it to its use
   and it shares h's register. */
INCLUDE_ASM("s2d/nonmatchings/game/mappage", S2dMapPageSwap);
#endif

void S2dMapPageCheck(void)
{
    if ((FLIP(D_800B91E4) == 0x90 && FLIP(D_800B8FD8) == 0x91) ||
        (FLIP(D_800B91E4) == 0x91 && FLIP(D_800B8FD8) == 0x90)) {
        g_map_page = 3;
    }
    if ((FLIP(D_800B91E4) == 0 && FLIP(D_800B8FD8) == 0xC7) ||
        (FLIP(D_800B91E4) == 0xC7 && FLIP(D_800B8FD8) == 0)) {
        g_map_page = 3;
    }
}

void S2dMapPagesFlip(void)
{
    if (g_map_page == 3) {
        S2dMapPageSwap();
        g_map_page--;
        S2dMapPageSwap();
        g_map_page--;
        S2dMapPageSwap();
        g_map_page--;
        S2dMapPageSwap();
        g_map_page--;
        g_map_side ^= 1;
    }
}
