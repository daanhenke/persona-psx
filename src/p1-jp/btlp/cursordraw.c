/* Persona 1 (JP) - drawing the command cursor.  BTLP only.
 *   0x8007A728 BtlCursorDraw
 *
 * Two textured quads are prepared for the cursor and used on alternate frames,
 * which is what makes it blink. Drawing one is a copy into the primitive
 * buffer rather than a link of the prepared quad itself, so the pair stays
 * clean and the copy is what the GPU walks.
 *
 * The buffer is a bump allocator shared by everything the battle draws: write
 * at g_btl_prim_next, link it, then move the pointer on by what was written.
 */
#include <types.h>
#include <libc.h>
#include <libgpu.h>

#define BTL_CURSOR_VISIBLE 1

extern u_char   g_btl_cursor_flags;
extern u_char   g_btl_cursor_buf;
extern POLY_FT4 g_btl_cursor_prims[];
extern char    *g_btl_prim_next;
extern u_long   g_btl_ot[][3];
extern int      g_btl_ot_index;

void BtlCursorDraw(void)
{
    if ((g_btl_cursor_flags & BTL_CURSOR_VISIBLE) != 0) {
        memcpy(g_btl_prim_next, &g_btl_cursor_prims[g_btl_cursor_buf],
               sizeof(POLY_FT4));
        AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
        g_btl_prim_next += sizeof(POLY_FT4);
        g_btl_cursor_buf ^= 1;
    }
}
