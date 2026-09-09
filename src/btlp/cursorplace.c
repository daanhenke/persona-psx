/* Persona 1 (JP) - moving the command cursor and running its animation.
 *   0x8007A428 BtlCursorPlace    (BTLP only)
 *
 * Called once a frame with where the cursor is to sit. It writes the corners
 * of whichever of the two prepared quads is current, then steps the animation:
 * each cel is held for the row's own count of frames, and the cel number wraps
 * at the row's length.
 *
 * The eight texture bytes are written through the array every time rather than
 * through a held pointer, which is why the buffer, the animation and the cel
 * are read again for each one.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/input.h>

/* The cursor is one sixteen-pixel cell, and its texture sits on the row at
   0x80 with the bottom half a cell below that. */
#define CURSOR_CELL 0x10
#define CURSOR_V    0x80

/* The two rows: the resting cursor and the one shown over a target. */
BtlCursorAnim g_btl_cursor_anims[2] = {
    { 0x00, 0x60, 8, 4 },
    { 0x18, 0x50, 6, 4 },
};

void BtlCursorPlace(short x, short y)
{
    POLY_FT4 *p;
    POLY_FT4 *base;
    /* Declared and never used: the original reserves fifty-six bytes of
       frame it never touches, and without this there is no frame at all. */
    SVECTOR  unused[7];

    /* The array's address is taken before the slot is worked out;
       subscripting it directly computes the two the other way round. */
    base = g_btl_cursor_prims;
    p = &base[g_btl_cursor_buf];
    p->x0 = x;
    p->y0 = y;
    p->x1 = x + CURSOR_CELL;
    p->y1 = y;
    p->x2 = x;
    p->y2 = y + CURSOR_CELL;
    p->x3 = x + CURSOR_CELL;
    p->y3 = y + CURSOR_CELL;

    /* Tested this way round: the cel wrap is the arm that falls through. */
    if (g_btl_cursor_timer >= g_btl_cursor_anims[g_btl_cursor_anim].frames) {
        /* The cel is worked out first and the timer cleared after it, even
           though the two stores come out the other way round. */
        g_btl_cursor_cel = (g_btl_cursor_cel + 1)
                           % g_btl_cursor_anims[g_btl_cursor_anim].cels;
        g_btl_cursor_timer = 0;
    } else {
        g_btl_cursor_timer = g_btl_cursor_timer + 1;
    }

    g_btl_cursor_prims[g_btl_cursor_buf].u0 =
        g_btl_cursor_anims[g_btl_cursor_anim].u
        + g_btl_cursor_cel * CURSOR_CELL;
    g_btl_cursor_prims[g_btl_cursor_buf].v0 =
        g_btl_cursor_anims[g_btl_cursor_anim].v + CURSOR_V;
    g_btl_cursor_prims[g_btl_cursor_buf].u1 =
        g_btl_cursor_anims[g_btl_cursor_anim].u
        + g_btl_cursor_cel * CURSOR_CELL + CURSOR_CELL;
    g_btl_cursor_prims[g_btl_cursor_buf].v1 =
        g_btl_cursor_anims[g_btl_cursor_anim].v + CURSOR_V;
    g_btl_cursor_prims[g_btl_cursor_buf].u2 =
        g_btl_cursor_anims[g_btl_cursor_anim].u
        + g_btl_cursor_cel * CURSOR_CELL;
    g_btl_cursor_prims[g_btl_cursor_buf].v2 =
        g_btl_cursor_anims[g_btl_cursor_anim].v + CURSOR_V + CURSOR_CELL;
    g_btl_cursor_prims[g_btl_cursor_buf].u3 =
        g_btl_cursor_anims[g_btl_cursor_anim].u
        + g_btl_cursor_cel * CURSOR_CELL + CURSOR_CELL;
    g_btl_cursor_prims[g_btl_cursor_buf].v3 =
        g_btl_cursor_anims[g_btl_cursor_anim].v + CURSOR_V + CURSOR_CELL;
}
