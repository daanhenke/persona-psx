/* Persona 1 (JP) - the message box a frame at a time, and on screen.
 * BTLP only.
 *   0x8007AAA4 BtlBoxState  0x8007AAB4 BtlBoxTick  0x8007AD50 BtlBoxDraw
 *
 * A unit of its own between the box setup in box.c and the rest of the
 * overlay: what the box is doing, the step that moves it, and the draw that
 * puts it up.
 *
 * The box is a row of textured quads and one shaded quad behind them, all
 * transformed by the GTE off the same matrix, so opening and closing it is
 * entirely a matter of moving the scale that matrix is built from. Every step
 * of the tick ends with the scale at one of the two extremes and drops the
 * step back to zero at the shared tail; the ones that are still going return
 * from inside.
 *
 * The frame is drawn a column at a time out of three parallel tables - where
 * the column sits, which tile it takes and how wide that tile is - and the
 * last column takes the closing entry rather than its own, so a box of any
 * width is drawn from the same left edge, middle and right edge.
 *
 * The quad behind them is coloured by the box's style: black for the plain
 * one, blue along its lower edge for the second, red down one side for the
 * third, and the fourth is left with whatever colours the record already
 * held.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <decomp/libc.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/box.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/draw.h>
#include <persona/btlp/text.h>
#include <persona/btlp/gfx.h>

/* Where the collapse leaves each axis before it gives up on it. */
#define BTL_BOX_GONE 4

/* The frame's tiles: how tall one is, where the row starts in the page, and
   which entry of the three column tables the closing column takes. */
#define BOX_TILE_H   0x18
#define BOX_TILE_V   0x88
#define BOX_COL_LAST 0x10

/* How far the row is carried left so the box is centred on its own origin:
   the left edge's own width, and half a column for each one past the two
   edges. */
#define BOX_ORIGIN   0x18
#define BOX_HALF_COL 8

/* A column of the quad behind the frame, and how far its two lit corners are
   taken up. */
#define BOX_BACK_W    0x10
#define BOX_BACK_TINT 0x80

/* The column's tile. The last column takes the closing entry of each table
   rather than the one its own index names. */
/* The drawer reads the box's state as members of the record it sits in
   (g_btl_box_flags heads it), not as scalars of their own. That is what makes
   gcc reload the column count after every store to a corner rather than
   lifting it out of the loop, and read the flags as a whole halfword. */
typedef struct { short v; } BoxField;
#define BOX_FIELD(g) (((BoxField *)&(g))->v)
#define BOX_COLS     BOX_FIELD(g_btl_box_cols)
#define BOX_COL(tbl, i) \
    ((tbl)[(i) == BOX_COLS - 1 ? BOX_COL_LAST : (i)])

/* The matrix values as the drawer's record sees them, off the offset pair. */
#define BOX_XF ((BtlBoxXform *)&g_btl_box_ox)

char BtlBoxState(void)
{
    return g_btl_box_step;
}

/* One frame of the box's open or close. */
void BtlBoxTick(void)
{
    u_char *hold;

    switch (g_btl_box_step) {
    case BTL_BOX_HOLD:
        hold = &g_btl_box_hold;
        (*hold)--;
        if (*hold != 0) {
            return;
        }
        break;
    case BTL_BOX_OPEN_NOW:
        g_btl_box_scale.vx = BTL_BOX_FULL;
        g_btl_box_scale.vy = BTL_BOX_FULL;
        g_btl_box_scale.vz = BTL_BOX_FULL;
        break;
    case BTL_BOX_CLOSE_NOW:
        g_btl_box_scale.vx = 0;
        g_btl_box_scale.vy = 0;
        g_btl_box_scale.vz = 0;
        g_btl_box_flags = 0;
        g_btl_box_step = 0;
        g_btl_text_page = 1;
        BtlTextReset();
        return;
    case BTL_BOX_OPEN:
        g_btl_box_scale.vx = BTL_BOX_FULL;
        g_btl_box_scale.vy += BTL_BOX_RAMP;
        if (g_btl_box_scale.vy < BTL_BOX_FULL) {
            return;
        }
        g_btl_box_scale.vx = BTL_BOX_FULL;
        g_btl_box_scale.vy = BTL_BOX_FULL;
        g_btl_box_scale.vz = BTL_BOX_FULL;
        break;
    case BTL_BOX_CLOSE:
        g_btl_box_scale.vy -= BTL_BOX_RAMP;
        if (g_btl_box_scale.vy >= 0) {
            return;
        }
        g_btl_text_page = 1;
        g_btl_box_scale.vx = 0;
        g_btl_box_scale.vy = 0;
        g_btl_box_scale.vz = 0;
        g_btl_text_page = 1;
        g_btl_box_flags = 0;
        g_btl_box_step = 0;
        BtlTextReset();
        return;
    case BTL_BOX_ZOOM:
        g_btl_box_scale.vx = g_btl_box_scale.vx / 2 + g_btl_box_scale.vx;
        if (g_btl_box_scale.vx < BTL_BOX_FULL + 1) {
            return;
        }
        g_btl_box_scale.vx = BTL_BOX_FULL;
        g_btl_box_scale.vy = g_btl_box_scale.vy / 2 + g_btl_box_scale.vy;
        if (g_btl_box_scale.vy < BTL_BOX_FULL + 1) {
            return;
        }
        g_btl_box_scale.vy = BTL_BOX_FULL;
        break;
    case BTL_BOX_COLLAPSE_STEP:
        /* The width and depth go through the drawer's view of the record and
           the height by its own name. gcc then cannot relate the two, so it
           keeps the height's address in a register across the reset and
           reaches the others absolutely. */
        g_btl_box_scale.vy = g_btl_box_scale.vy - g_btl_box_scale.vy / 2;
        if (g_btl_box_scale.vy >= BTL_BOX_THIN) {
            return;
        }
        g_btl_box_scale.vy = BTL_BOX_THIN;
        BOX_XF->scale.vx = BOX_XF->scale.vx - BOX_XF->scale.vx / 2;
        if (BOX_XF->scale.vx >= BTL_BOX_GONE) {
            return;
        }
        BtlTextReset();
        g_btl_text_page = 1;
        BOX_XF->scale.vx = 0;
        g_btl_box_scale.vy = 0;
        BOX_XF->scale.vz = 0;
        g_btl_box_flags = 0;
        break;
    case 0:
    default:
        return;
    }
    g_btl_box_step = 0;
}

/* The box on screen: one textured quad a column, and the shaded quad the
   whole row stands on. */
void BtlBoxDraw(void)
{
    MATRIX       m;
    SVECTOR      corner[4];
    POLY_FT4     tile;
    POLY_G4      back;
    long         ofx;
    long         ofy;
    /* The corner counter doubles as the two out-parameters the transform is
       handed, the way the overlay's other GTE calls write them. */
    long         otz;
    BtlBoxXform *x;
    int          i;

    ReadGeomOffset(&ofx, &ofy);
    x = (BtlBoxXform *)&g_btl_box_ox;
    SetGeomOffset(x->ox, g_btl_box_oy);
    RotMatrix(&x->rot, &m);
    TransMatrix(&m, &x->pos);
    ScaleMatrix(&m, &x->scale);
    SetRotMatrix(&m);
    SetTransMatrix(&m);
    if (((short)g_btl_box_flags & BTL_BOX_FRAME) != 0) {
        SetPolyFT4(&tile);
        SetSemiTrans(&tile, 0);
        SetShadeTex(&tile, 1);
        tile.tpage = GetTPage(1, 0,
                              (g_btl_text_page * BTL_BOX_PAGE_W + BTL_BOX_PAGE0)
                                  * BTL_BOX_COL,
                              BTL_BOX_TILES_Y);
        tile.clut = GetClut(0, BTL_BOX_CLUT_Y);
        for (i = 0; i < BOX_COLS; i++) {
            for (otz = 0; otz < 4; otz++) {
                corner[otz].vx = g_btl_box_col_x[i]
                                 + (otz % 2) * BOX_COL(g_btl_box_col_w, i)
                                 - (BOX_ORIGIN
                                    + (BOX_COLS - 2) * BOX_HALF_COL);
                corner[otz].vy = (otz / 2) * BOX_TILE_H - BOX_TILE_H / 2;
                corner[otz].vz = 0;
            }
            RotTransPers4(&corner[0], &corner[1], &corner[2], &corner[3],
                          (long *)&tile.x0, (long *)&tile.x1,
                          (long *)&tile.x2, (long *)&tile.x3, &otz, &otz);
            tile.u0 = BOX_COL(g_btl_box_col_u, i);
            tile.v0 = BOX_TILE_V;
            tile.u1 = BOX_COL(g_btl_box_col_u, i)
                      + BOX_COL(g_btl_box_col_w, i);
            tile.v1 = BOX_TILE_V;
            tile.u2 = BOX_COL(g_btl_box_col_u, i);
            tile.v2 = BOX_TILE_V + BOX_TILE_H;
            tile.u3 = BOX_COL(g_btl_box_col_u, i)
                      + BOX_COL(g_btl_box_col_w, i);
            tile.v3 = BOX_TILE_V + BOX_TILE_H;
            memcpy(g_btl_prim_next, &tile, sizeof(POLY_FT4));
            AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
            g_btl_prim_next += sizeof(POLY_FT4);
        }

        SetPolyG4(&back);
        SetSemiTrans(&back, 0);
        SetShadeTex(&back, 1);
        switch ((BOX_FIELD(g_btl_box_flags) >> BTL_BOX_STYLE_SHIFT) & 3) {
        case 0:
            back.r0 = 0;
            back.g0 = 0;
            back.b0 = 0;
            back.r1 = 0;
            back.g1 = 0;
            back.b1 = 0;
            back.r2 = 0;
            back.g2 = 0;
            back.b2 = 0;
            back.r3 = 0;
            back.g3 = 0;
            back.b3 = 0;
            break;
        case 1:
            back.r0 = 0;
            back.g0 = 0;
            back.b0 = 0;
            back.r1 = 0;
            back.g1 = 0;
            back.b1 = BOX_BACK_TINT;
            back.r2 = 0;
            back.g2 = 0;
            back.b2 = BOX_BACK_TINT;
            back.r3 = 0;
            back.g3 = 0;
            back.b3 = 0;
            break;
        case 2:
            back.r0 = 0;
            back.g0 = 0;
            back.b0 = 0;
            back.r1 = 0;
            back.g1 = 0;
            back.b1 = 0;
            back.r2 = BOX_BACK_TINT;
            back.g2 = 0;
            back.b2 = 0;
            back.r3 = BOX_BACK_TINT;
            back.g3 = 0;
            back.b3 = 0;
            break;
        }
        for (otz = 0; otz < 4; otz++) {
            corner[otz].vx = BOX_BACK_W * BOX_COLS * (otz % 2)
                             - (BOX_ORIGIN + (BOX_COLS - 2) * BOX_HALF_COL);
            corner[otz].vy = (otz / 2) * BOX_TILE_H - BOX_TILE_H / 2;
            corner[otz].vz = 0;
        }
        RotTransPers4(&corner[0], &corner[1], &corner[2], &corner[3],
                      (long *)&back.x0, (long *)&back.x1, (long *)&back.x2,
                      (long *)&back.x3, &otz, &otz);
        memcpy(g_btl_prim_next, &back, sizeof(POLY_G4));
        AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
        g_btl_prim_next += sizeof(POLY_G4);
    }
    SetGeomOffset(ofx, ofy);
}
