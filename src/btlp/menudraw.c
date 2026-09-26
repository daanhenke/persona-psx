/* Persona 1 (JP) - the choice box on screen.  BTLP only.
 *   0x8007DC48 BtlMenuDraw
 *
 * Three primitives go into this frame's UI ordering table while the box is up:
 * a draw area covering the whole working area, the panel sprite, and the draw
 * mode naming its texture page. A fourth follows and cuts the draw area down
 * to the band the panel occupies, so everything drawn after this - the choices
 * themselves - is clipped to the box.
 *
 * The panel rests on line 0xAC at x 0x20, and the two menu displacements move
 * it from there: BtlMenuUpdate walks the slide four pixels a frame to take the
 * box off the working area and bring it back.
 *
 * Each primitive is copied into the shared bump buffer rather than linked from
 * the stack, because the stack copy is gone by the time the GPU walks the list.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <decomp/libc.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/draw.h>

/* The whole working area, and the band the panel takes of it. */
#define MENU_AREA_W 0x140
#define MENU_AREA_H 0xF0

/* Where the panel sits and how big it is. */
#define MENU_PANEL_X 0x20
#define MENU_PANEL_Y 0xAC
#define MENU_PANEL_W 0x100
#define MENU_PANEL_H 0x30

/* Its texture: the page at 0x380,0x150 read four bits at a time, with the
   palette the loader left at 0x6038. */
#define MENU_TPX   0x380
#define MENU_TPY   0x150
#define MENU_CLUT  0x6038
#define MENU_U     0
#define MENU_V     0x50

extern int      g_btl_menu_state;
extern u_short  g_btl_menu_shift;
extern u_short  g_btl_menu_slide;
extern RECT     g_btl_menu_clip[][2];
extern char    *g_btl_prim_next;
extern u_long   g_btl_ot[][3];
extern int      g_btl_ot_index;

/* 99.68%. The clip rects are filled x, y, w, h, and the draw origin is the
   drawing environment's offset (draw.h), so its y read stays behind the store
   of x. Each rect has a pointer of its own: local-alloc ties the index sum to
   the pointer only when the pointer dies once in its block, and that tie is
   what puts the index in a1. Both index the clip table directly.
   Left: s0 and s1 swapped between the clip table's address and g_btl_ot's.
   The table register is stepped by 8 in place for the second rect, which
   ties the two into one quantity (5 refs over the body, same priority as
   g_btl_ot's, and born first, so it takes s0). The image gives g_btl_ot s0,
   so its table quantity must not have been tied (local-alloc.c qty_compare_1,
   combine_regs). */
#ifdef NON_MATCHING
void BtlMenuDraw(void)
{
    RECT    *clip;
    RECT    *clip2;
    SPRT     panel;
    DR_MODE  mode;
    DR_AREA  area;

    if (g_btl_menu_state != 0) {
        clip = &g_btl_menu_clip[g_btl_ot_index][0];
        clip->x = g_btl_draw_x;
        clip->y = g_btl_draw_y;
        clip->w = MENU_AREA_W;
        clip->h = MENU_AREA_H;
        SetDrawArea(&area, clip);
        memcpy(g_btl_prim_next, &area, sizeof(DR_AREA));
        AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
        g_btl_prim_next += sizeof(DR_AREA);

        SetSprt(&panel);
        SetSemiTrans(&panel, 0);
        SetShadeTex(&panel, 1);
        panel.u0 = MENU_U;
        panel.v0 = MENU_V;
        panel.w = MENU_PANEL_W;
        panel.clut = MENU_CLUT;
        panel.h = MENU_PANEL_H;
        panel.x0 = g_btl_menu_shift + MENU_PANEL_X;
        panel.y0 = g_btl_menu_slide + MENU_PANEL_Y;
        SetDrawMode(&mode, 0, 0, GetTPage(0, 0, MENU_TPX, MENU_TPY), 0);
        memcpy(g_btl_prim_next, &panel, sizeof(SPRT));
        AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
        g_btl_prim_next += sizeof(SPRT);
        memcpy(g_btl_prim_next, &mode, sizeof(DR_MODE));
        AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
        g_btl_prim_next += sizeof(DR_MODE);

        clip2 = &g_btl_menu_clip[g_btl_ot_index][1];
        clip2->x = g_btl_draw_x;
        clip2->y = g_btl_draw_y + MENU_PANEL_Y;
        clip2->w = MENU_AREA_W;
        clip2->h = MENU_PANEL_H;
        SetDrawArea(&area, clip2);
        memcpy(g_btl_prim_next, &area, sizeof(DR_AREA));
        AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
        g_btl_prim_next += sizeof(DR_AREA);
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/menudraw", BtlMenuDraw);
#endif

