/* Persona 1 (JP) - drawing the status panel.  BTLP only.
 *   0x8007E374 BtlHudDraw
 *
 * The panel is two faces: the ninety squares of its own face, taken cell by
 * cell out of g_btl_hud_cells, and the nine pieces of the bar along its top.
 *
 * At full size both are flat against the screen, so each face goes up as
 * sprites - six for the panel and nine for the bar - and the record's own draw
 * modes carry the pages. While either scale is still ramping the panel is
 * turned through the GTE instead: every square and every piece of the bar is a
 * four-cornered quad, rotated, translated and scaled through the matrix the
 * record holds.
 *
 * Every primitive is built in the scratchpad and copied into the frame's
 * buffer, so one work area serves the whole draw.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <decomp/libc.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/draw.h>
#include <persona/btlp/hud.h>
#include <persona/btlp/battle.h>

/* The work area the whole draw is built in: the corner the square being drawn
   sits on, the texture corners of its four corners, the cell of the map the
   walk is on, the four corners themselves, and the two primitives they are
   put up as. */
typedef struct {
    /* 0x00 */ short    ox;
    /* 0x02 */ short    oy;
    /* 0x04 */ u_char   u[4];
    /* 0x08 */ u_char   v[4];
    /* 0x0C */ u_char  *cell;
    /* 0x10 */ SVECTOR  pos[4];
    /* 0x30 */ POLY_FT4 quad;
    /* 0x58 */ SPRT     sprite;
} BtlHudPad;                       /* 0x6C bytes */

#define HUD_PAD ((BtlHudPad *)0x1F800000)

/* The panel's face: eighteen squares to a row, each a sixteenth of the sheet,
   and where the first of them sits. */
#define HUD_FACE_COLS 18
#define HUD_FACE_CELL 0x10
#define HUD_FACE_X    (-0x90)
#define HUD_FACE_Y    (-0x28)

/* A cell from row five on is half a cell further down the sheet. */
#define HUD_CELL_SPLIT 0x50
#define HUD_CELL_SHIFT 8

/* The pages the two faces are taken from. */
#define HUD_FACE_TP_X   0x300
#define HUD_FACE_TP_Y   0x100
#define HUD_FACE_TP     1
#define HUD_FACE_CLUT_X 0
#define HUD_FACE_CLUT_Y 0x1FA
#define HUD_BAR_TP_X    0x340
#define HUD_BAR_TP_Y    0x1B0
#define HUD_BAR_TP      0
#define HUD_BAR_CLUT_X  0x300
#define HUD_BAR_CLUT_Y  0x1F8

/* The bar along the top: nine pieces, the last of them half as wide. */
#define HUD_BAR_PIECES 9
#define HUD_BAR_LAST   8
#define HUD_BAR_W      0x20
#define HUD_BAR_LAST_W 0x10
#define HUD_BAR_H      0x38
#define HUD_BAR_X      (-0x88)
#define HUD_BAR_Y      (-0x20)
#define HUD_BAR_V      (-0x50)

/* And where the same bar goes at full size, where it is a row of sprites
   rather than quads. */
#define HUD_BAR_FLAT_X 0x18
#define HUD_BAR_FLAT_Y 0xA8
#define HUD_BAR_FLAT_U 0
#define HUD_BAR_FLAT_V 0xB0

/* The six pieces of the panel's own face are put up against this corner. */
#define HUD_PIECE_DX 0x10
#define HUD_PIECE_DY 0xA0
#define HUD_PIECES   6

/* Full brightness for a piece that is drawn shaded. */
#define HUD_GREY 0x80

/* The four corners of a square, as offsets of half of each step. */
#define HUD_CORNER_X(j) ((j) % 2)
#define HUD_CORNER_Y(j) ((j) / 2)

/* 93.63%. The bar pass picks each corner's offset with one ternary and one
   store, as the image does. The two passes, the cell walk and every primitive
   are the image's;
   what is left is which saved register each of the two bases takes - the image
   keeps the record in s1 and the scratchpad in s2, this the other way round -
   and a handful of instructions the scheduler puts on the far side of a store.
   Declaring the two in either order, assigning them at their declaration or in
   the body, reaching the record through a pointer or by name, and spelling the
   scratchpad through the macro all leave the pair as they are. */
#ifdef NON_MATCHING
void BtlHudDraw(void)
{
    BtlHudPad *pad = HUD_PAD;
    MATRIX     m;
    int        ox;
    int        oy;
    int        n;
    int        j;
    int        x;
    int        w;
    /* The corner walk counts in the same word the transform is handed for the
       depth and the flag it writes back: the panel is flat, so neither is
       read. */
    long       i;

    if ((g_btl_hud_flags & BTL_HUD_DRAWN) == 0) {
        return;
    }
    BtlDrawIndicator();
    pad->cell = g_btl_hud_cells;
    for (n = 3; n >= 0; n--) {
        pad->pos[n].vz = 0;
    }
    ReadGeomOffset(&ox, &oy);
    SetGeomOffset(g_btl_hud.x, g_btl_hud.y);
    RotMatrix(&g_btl_hud.rot, &m);
    TransMatrix(&m, &g_btl_hud.trans);
    ScaleMatrix(&m, &g_btl_hud.scale);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (g_btl_hud.scale.vx >= BTL_HUD_FULL && g_btl_hud_scale_y >= BTL_HUD_FULL) {
        SetSprt(&pad->sprite);
        SetSemiTrans(&pad->sprite, 0);
        SetShadeTex(&pad->sprite, 1);
        SetDrawMode(&g_btl_hud.mode[0][g_btl_ot_index], 0, 0,
                    GetTPage(HUD_FACE_TP, 0, HUD_FACE_TP_X, HUD_FACE_TP_Y), 0);
        pad->sprite.clut = GetClut(HUD_FACE_CLUT_X, HUD_FACE_CLUT_Y);
        j = 0;
        do {
            pad->sprite.x0 = g_btl_hud.piece_xy[j][0] + HUD_PIECE_DX;
            pad->sprite.y0 = g_btl_hud.piece_xy[j][1] + HUD_PIECE_DY;
            pad->sprite.u0 = g_btl_hud.piece_uv[j][0];
            pad->sprite.v0 = g_btl_hud.piece_uv[j][2];
            pad->sprite.w  = g_btl_hud.piece_wh[j][0];
            pad->sprite.h  = g_btl_hud.piece_wh[j][1];
            memcpy(g_btl_prim_next, &pad->sprite, sizeof(SPRT));
            AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
            g_btl_prim_next += sizeof(SPRT);
            pad->cell++;
            j++;
        } while (j < HUD_PIECES);

        memcpy(g_btl_prim_next, &g_btl_hud.mode[0][g_btl_ot_index],
               sizeof(DR_MODE));
        AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
        g_btl_prim_next += sizeof(DR_MODE);

        SetSprt(&pad->sprite);
        SetSemiTrans(&pad->sprite, 1);
        SetShadeTex(&pad->sprite, 0);
        pad->sprite.r0 = HUD_GREY;
        pad->sprite.g0 = HUD_GREY;
        pad->sprite.b0 = HUD_GREY;
        pad->sprite.u0 = HUD_BAR_FLAT_U;
        pad->sprite.v0 = HUD_BAR_FLAT_V;
        SetDrawMode(&g_btl_hud.mode[1][g_btl_ot_index], 0, 0,
                    GetTPage(HUD_BAR_TP, 0, HUD_BAR_TP_X, HUD_BAR_TP_Y), 0);
        pad->sprite.clut = GetClut(HUD_BAR_CLUT_X, HUD_BAR_CLUT_Y);
        n = 0;
        x = HUD_BAR_FLAT_X;
        do {
            w = HUD_BAR_W;
            if (n == HUD_BAR_LAST) {
                w = HUD_BAR_LAST_W;
            }
            pad->sprite.w  = w;
            pad->sprite.x0 = x;
            x += HUD_BAR_W;
            pad->sprite.h  = HUD_BAR_H;
            pad->sprite.y0 = HUD_BAR_FLAT_Y;
            memcpy(g_btl_prim_next, &pad->sprite, sizeof(SPRT));
            AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
            g_btl_prim_next += sizeof(SPRT);
            n++;
        } while (n < HUD_BAR_PIECES);

        memcpy(g_btl_prim_next, &g_btl_hud.mode[1][g_btl_ot_index],
               sizeof(DR_MODE));
        AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
        g_btl_prim_next += sizeof(DR_MODE);
    } else {
        SetPolyFT4(&pad->quad);
        SetSemiTrans(&pad->quad, 0);
        SetShadeTex(&pad->quad, 0);
        pad->quad.r0 = HUD_GREY;
        pad->quad.g0 = HUD_GREY;
        pad->quad.b0 = HUD_GREY;
        pad->quad.tpage = GetTPage(HUD_FACE_TP, 0, HUD_FACE_TP_X, HUD_FACE_TP_Y);
        pad->quad.clut = GetClut(HUD_FACE_CLUT_X, HUD_FACE_CLUT_Y);
        n = 0;
        do {
            if (*pad->cell != BTL_HUD_EMPTY) {
                i = 0;
                do {
                    pad->u[i] = ((*pad->cell & 0xF) + HUD_CORNER_X(i))
                                * HUD_FACE_CELL;
                    w = ((*pad->cell >> 4) + HUD_CORNER_Y(i)) * HUD_FACE_CELL;
                    if (*pad->cell >= HUD_CELL_SPLIT) {
                        w += HUD_CELL_SHIFT;
                    }
                    pad->v[i] = w;
                    i++;
                } while (i < 4);
                i = 0;
                pad->quad.u0 = pad->u[0];
                pad->quad.v0 = pad->v[0];
                pad->quad.u1 = pad->u[1];
                pad->quad.v1 = pad->v[1];
                pad->quad.u2 = pad->u[2];
                pad->quad.v2 = pad->v[2];
                pad->quad.u3 = pad->u[3];
                pad->quad.v3 = pad->v[3];
                pad->ox = (n % HUD_FACE_COLS) * HUD_FACE_CELL + HUD_FACE_X;
                pad->oy = (n / HUD_FACE_COLS) * HUD_FACE_CELL + HUD_FACE_Y;
                do {
                    pad->pos[i].vx = pad->ox + HUD_CORNER_X(i) * HUD_FACE_CELL;
                    pad->pos[i].vy = pad->oy + HUD_CORNER_Y(i) * HUD_FACE_CELL;
                    i++;
                } while (i < 4);
                RotTransPers4(&pad->pos[0], &pad->pos[1], &pad->pos[2],
                              &pad->pos[3], (long *)&pad->quad.x0,
                              (long *)&pad->quad.x1, (long *)&pad->quad.x2,
                              (long *)&pad->quad.x3, &i, &i);
                memcpy(g_btl_prim_next, &pad->quad, sizeof(POLY_FT4));
                AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
                g_btl_prim_next += sizeof(POLY_FT4);
            }
            pad->cell++;
            n++;
        } while (n < BTL_HUD_CELLS);

        SetSemiTrans(&pad->quad, 1);
        pad->quad.tpage = GetTPage(HUD_BAR_TP, 0, HUD_BAR_TP_X, HUD_BAR_TP_Y);
        pad->quad.clut = GetClut(HUD_BAR_CLUT_X, HUD_BAR_CLUT_Y);
        n = 0;
        do {
            i = 0;
            do {
                pad->u[i] = n == HUD_BAR_LAST
                                ? HUD_CORNER_X(i) * HUD_BAR_LAST_W
                                : HUD_CORNER_X(i) * HUD_BAR_W;
                pad->v[i] = HUD_CORNER_Y(i) * HUD_BAR_H + HUD_BAR_V;
                i++;
            } while (i < 4);
            pad->ox = n * HUD_BAR_W + HUD_BAR_X;
            pad->oy = HUD_BAR_Y;
            i = 0;
            pad->quad.u0 = pad->u[0];
            pad->quad.v0 = pad->v[0];
            pad->quad.u1 = pad->u[1];
            pad->quad.v1 = pad->v[1];
            pad->quad.u2 = pad->u[2];
            pad->quad.v2 = pad->v[2];
            pad->quad.u3 = pad->u[3];
            pad->quad.v3 = pad->v[3];
            do {
                pad->pos[i].vx = n == HUD_BAR_LAST
                                     ? pad->ox + HUD_CORNER_X(i) * HUD_BAR_LAST_W
                                     : pad->ox + HUD_CORNER_X(i) * HUD_BAR_W;
                pad->pos[i].vy = pad->oy + HUD_CORNER_Y(i) * HUD_BAR_H;
                i++;
            } while (i < 4);
            RotTransPers4(&pad->pos[0], &pad->pos[1], &pad->pos[2],
                          &pad->pos[3], (long *)&pad->quad.x0,
                          (long *)&pad->quad.x1, (long *)&pad->quad.x2,
                          (long *)&pad->quad.x3, &i, &i);
            memcpy(g_btl_prim_next, &pad->quad, sizeof(POLY_FT4));
            AddPrim(g_btl_ot[g_btl_ot_index], g_btl_prim_next);
            g_btl_prim_next += sizeof(POLY_FT4);
            n++;
        } while (n < HUD_BAR_PIECES);
    }
    SetGeomOffset(ox, oy);
}
#else
INCLUDE_ASM("btlp/nonmatchings/huddraw", BtlHudDraw);
#endif
