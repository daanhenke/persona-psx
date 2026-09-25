#ifndef PERSONA_BTLP_DRAW_H
#define PERSONA_BTLP_DRAW_H

/* Persona 1 (JP) - the battle's per-frame primitives.
 *
 * Two small ordering tables are kept, one per display page, and
 * g_btl_ot_index says which one this frame builds. A primitive is set up on
 * the stack, copied to g_btl_prim_next - a bump buffer emptied every frame -
 * linked into the table, and the pointer moved on past it; the stack copy is
 * gone by the time the GPU walks the list.
 *
 * g_btl_draw_x/y is where this page's drawing area sits in VRAM, so a draw
 * area that should cover the screen starts there. They are the drawing
 * environment's offset, and read as its members: that is what keeps a load
 * of them behind a store into a RECT, as the image has it.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>

extern char   *g_btl_prim_next;
extern u_long  g_btl_ot[][3];
extern int     g_btl_ot_index;
extern DRAWENV g_btl_drawenv;
#define g_btl_draw_x (g_btl_drawenv.ofs[0])
#define g_btl_draw_y (g_btl_drawenv.ofs[1])

/* The whole working area, as a draw area covers it. */
#define BTL_AREA_W 0x140
#define BTL_AREA_H 0xF0

#endif
