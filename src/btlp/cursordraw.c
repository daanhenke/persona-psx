/* Persona 1 (JP) - drawing the command cursor.  BTLP only.
 *   0x8007A328 BtlCursorInitPrims, 0x8007A728 BtlCursorDraw
 *
 * Two textured quads are prepared for the cursor and used on alternate frames,
 * which is what makes it blink. Drawing one is a copy into the primitive
 * buffer rather than a link of the prepared quad itself, so the pair stays
 * clean and the copy is what the GPU walks.
 *
 * The buffer is a bump allocator shared by everything the battle draws: write
 * at g_btl_prim_next, link it, then move the pointer on by what was written.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/input.h>

extern char    *g_btl_prim_next;
extern u_long   g_btl_ot[][3];
extern int      g_btl_ot_index;

/* Where the cursor's texture and palette live: one page in at 8 bits, and the
   CLUT the loader put in slot 29. Full grey leaves the texture untinted. */
#define CURSOR_TP   1
#define CURSOR_X    0x340
#define CURSOR_Y    0x80
#define CURSOR_CLUT 29
#define CURSOR_GREY 0x80

/* Both quads are built by preparing one and copying it over the other, so the
   pair start identical and only their texture corner is moved afterwards. The
   cursor starts hidden, on cel zero of animation zero. */
void BtlCursorInitPrims(void)
{
    POLY_FT4 *p;

    p = g_btl_cursor_prims;
    SetPolyFT4(p);
    SetSemiTrans(p, 0);
    SetShadeTex(p, 0);
    p->r0 = CURSOR_GREY;
    p->g0 = CURSOR_GREY;
    p->b0 = CURSOR_GREY;
    p->tpage = GetTPage(CURSOR_TP, 0, CURSOR_X, CURSOR_Y);
    p->clut = g_btl_clut[CURSOR_CLUT];
    p[1] = p[0];
    g_btl_cursor_timer = 0;
    g_btl_cursor_cel = 0;
    BtlCursorShow(0);
    g_btl_cursor_anim = 0;
}
