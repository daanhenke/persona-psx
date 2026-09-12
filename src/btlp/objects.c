/* Persona 1 (JP) - the frame's pass over every display object.  BTLP only.
 *   0x80081B7C BtlDrawObjects
 *
 * BtlDrawFrame runs this once a frame, after BtlWaveMesh.
 *
 * It opens by pointing the seven primitive pointers into this frame's half of
 * the pool, then builds the camera from its rotation, the intro's scale and a
 * translation the screen shake writes into.
 *
 * All six object lists are then walked. A list head, or a record hidden, is
 * skipped for drawing but still has its colour walked. Every other record
 * first hands its position down to whatever hangs off it - the shadow takes
 * the script's last word and the x and y, the attached piece takes all three
 * when it asks for them, and the marker and its own attached piece take all
 * three as well.
 *
 * Kinds 4 and 5 are the two halves of one drawing, so the kind is rewritten
 * each frame: the flat one only while both intro scales are at unity and the
 * record is neither still nor skipped, the transformed one otherwise. Those
 * two draw at the camera's own screen distance; every other kind at 100. The
 * kind then picks the handler out of g_btl_obj_draw.
 *
 * Every record finishes with three BtlApproach steps walking its colour toward
 * the one it is heading for. The arena's five faces and the mesh are drawn
 * last, and the debug grid in between when both its flags are up.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <inline.h>
#include <libgpu.h>
#include <persona/btlp/object.h>
#include <persona/btlp/battle.h>

/* A list head is not drawn, and neither is a record carrying this. */

/* The two bits that keep a record on the transformed half of its pair. */
#define BTL_OBJ_FLAT_BARS 0x4008

/* An attached piece follows its owner only when it asks to. */
#define BTL_OBJ_FOLLOWS 0x40

/* The two kinds that are one drawing seen two ways. */
#define BTL_DRAW_FLAT 4
#define BTL_DRAW_ROT  5

/* Unity in the intro's scale, and what every other kind draws at. */
#define BTL_SCALE_ONE  0x1000
#define BTL_DIST_FIXED 100

/* Six object lists, and where the camera is centred. */
#define BTL_OBJ_GROUPS 6
#define BTL_SCREEN_CX  0xA0
#define BTL_SCREEN_CY  0x78

/* Each frame owns half the primitive pool. The seven runs it is cut into, and
   the two ordering tables that live in it. */
#define BTL_FRAME_BYTES 0xE660
#define BTL_SPRT_AT     0x70
#define BTL_TILE_AT     0x2398
#define BTL_POLYFT4_AT  0x2498
#define BTL_POLYF4_AT   0x6AE8
#define BTL_POLYG4_AT   0x6C68
#define BTL_LINEG2_AT   0x7568
#define BTL_DRMODE_AT   0xC1A8
#define BTL_ARENA_OT    0xD6C0
#define BTL_ARENA_AT    0x76A8
#define BTL_MESH_AT     0xA588

/* The debug grid: 0xF rows of five squares, each 0x1E across and 0x14 down. */
#define BTL_GRID_ROWS  0xF
#define BTL_GRID_COLS  5
#define BTL_GRID_W     0x1E
#define BTL_GRID_H     0x14
#define BTL_GRID_X0    -0x4B
#define BTL_GRID_X1    -0x2D
#define BTL_GRID_Y0    -0x82
#define BTL_GRID_Y1    -0x96
#define BTL_GRID_LIT   0xFF
#define BTL_GRID_DIM   0x40

/* What an ordering table entry keeps of a primitive's address. */
#define BTL_OT_ADDR 0xFFFFFF

/* The mesh: nine rows of twenty squares, walked with a bit a square. */
#define BTL_MESH_ROWS 9
#define BTL_MESH_COLS 0x14
#define BTL_MESH_TOP  0x80000000

extern u_short   g_btl_sprite_count;
extern u_short   g_btl_poly_count;
extern u_short   g_btl_sprite_peak;
extern u_short   g_btl_poly_peak;
extern int       g_btl_screen_dist;
extern MATRIX    g_btl_cam_matrix;
extern VECTOR    g_btl_cam_shift;
extern VECTOR    g_btl_intro_x;
extern long      g_btl_intro_y;
extern u_short   g_btl_tick;
extern signed char g_btl_shake_offsets[];
extern u_char    g_btl_debug_grid;
extern u_char    g_btl_debug_grid_cells[];
extern u_char    g_btl_arena_show;
extern u_char    g_btl_mesh_show;
extern SVECTOR   g_btl_arena_face[];
extern SVECTOR   g_btl_mesh[];
extern u_long    g_btl_mesh_hidden[];

extern void BtlApproach(short *cur, const short *target, int step);
extern void BtlDrawArenaBack(void);
extern void BtlDrawArenaRight(void);
extern void BtlDrawArenaLeft(void);
extern void BtlDrawArenaBottom(void);
extern void BtlDrawArenaTop(void);

#ifdef NON_MATCHING
void BtlDrawObjects(void)
{
    BtlObj *o;
    BtlObj *att;
    u_long *ot;
    u_char *cells;
    u_char *base;
    short  *hold;
    const short *m;
    const short *far;
    POLY_FT4 *poly;
    u_long *hidden;
    u_int   bit;
    int     group;
    int     dist;
    int     row;
    int     col;
    int     x0;
    int     x1;
    int     y0;
    int     y1;
    int     y_tl;
    int     y_tr;
    int     y_bl;
    u_int   mask;
    u_int   lit;
    u_int   dim;
    u_int   addr;
    short   held;

    g_btl_sprite_count = 0;
    g_btl_poly_count = 0;
    g_btl_sprt_next = (SPRT *)(g_btl_prim_pool + g_btl_frame * BTL_FRAME_BYTES
                               + BTL_SPRT_AT);
    g_btl_polyft4_next = (POLY_FT4 *)(g_btl_prim_pool
                                      + g_btl_frame * BTL_FRAME_BYTES
                                      + BTL_POLYFT4_AT);
    g_btl_polyf4_next = (POLY_F4 *)(g_btl_prim_pool
                                    + g_btl_frame * BTL_FRAME_BYTES
                                    + BTL_POLYF4_AT);
    g_btl_polyg4_next = (POLY_G4 *)(g_btl_prim_pool
                                    + g_btl_frame * BTL_FRAME_BYTES
                                    + BTL_POLYG4_AT);
    g_btl_tile_next = (TILE *)(g_btl_prim_pool + g_btl_frame * BTL_FRAME_BYTES
                               + BTL_TILE_AT);
    g_btl_lineg2_next = (LINE_G2 *)(g_btl_prim_pool
                                    + g_btl_frame * BTL_FRAME_BYTES
                                    + BTL_LINEG2_AT);
    g_btl_drmode_next = (DR_MODE *)(g_btl_prim_pool
                                    + g_btl_frame * BTL_FRAME_BYTES
                                    + BTL_DRMODE_AT);
    if (g_btl_shake_on != 0) {
        g_btl_cam_shift.vy = g_btl_shake_offsets[g_btl_tick & 3];
    } else {
        g_btl_cam_shift.vy = 0;
    }
    RotMatrix(&g_btl_cam_rot, &g_btl_cam_matrix);
    ScaleMatrix(&g_btl_cam_matrix, &g_btl_intro_x);
    TransMatrix(&g_btl_cam_matrix, &g_btl_cam_shift);
    group = 0;
    do {
        for (o = (BtlObj *)((char *)g_btl_obj_pool
                            + g_btl_obj_first[group] * sizeof(BtlObj));
             o != 0; o = o->next) {
            if ((o->kind & BTL_OBJ_HEAD) == 0
                && (o->attr & BTL_OBJ_HIDDEN) == 0) {
                if (o->shadow != 0) {
                    o->shadow->last = o->last;
                    o->shadow->x = o->x;
                    o->shadow->y = o->y;
                }
                att = o->attached;
                if (att != 0 && (att->attr & BTL_OBJ_FOLLOWS) != 0) {
                    att->x = o->x;
                    o->attached->y = o->y;
                    o->attached->z = o->z;
                }
                if (o->mark != 0) {
                    o->mark->x = o->x;
                    o->mark->y = o->y;
                    o->mark->z = o->z;
                    o->mark->attached->x = o->x;
                    o->mark->attached->y = o->y;
                    o->mark->attached->z = o->z;
                }
                g_btl_obj_x = o->x >> 16;
                g_btl_obj_y = o->y >> 16;
                if ((u_int)(o->draw - BTL_DRAW_FLAT) < 2) {
                    if (g_btl_intro_x.vx == BTL_SCALE_ONE
                        && g_btl_intro_y == BTL_SCALE_ONE
                        && (o->attr & BTL_OBJ_FLAT_BARS) == 0) {
                        o->draw = BTL_DRAW_FLAT;
                    } else {
                        o->draw = BTL_DRAW_ROT;
                    }
                    dist = g_btl_screen_dist;
                } else {
                    dist = BTL_DIST_FIXED;
                }
                SetGeomScreen(dist);
                g_btl_obj_draw[o->draw](o);
            }
            BtlApproach(&o->rgb[0], &o->rgb_to[0], o->fade);
            BtlApproach(&o->rgb[1], &o->rgb_to[1], o->fade);
            BtlApproach(&o->rgb[2], &o->rgb_to[2], o->fade);
        }
        group++;
    } while (group < BTL_OBJ_GROUPS);

    if (g_btl_sprite_peak < g_btl_sprite_count) {
        g_btl_sprite_peak = g_btl_sprite_count;
    }
    if (g_btl_poly_peak < g_btl_poly_count) {
        g_btl_poly_peak = g_btl_poly_count;
    }
    SetGeomScreen(g_btl_screen_dist);
    SetRotMatrix(&g_btl_cam_matrix);
    SetTransMatrix(&g_btl_cam_matrix);
    SetGeomOffset(BTL_SCREEN_CX, BTL_SCREEN_CY);

    hold = &held;
    if (g_btl_debug_hud != 0 && g_btl_debug_grid != 0) {
        row = 0;
        base = g_btl_debug_grid_cells;
        y0 = BTL_GRID_Y0;
        y1 = BTL_GRID_Y1;
        lit = BTL_GRID_LIT;
        dim = BTL_GRID_DIM;
        mask = BTL_OT_ADDR;
        do {
            col = 0;
            /* One y per corner of the quad rather than one per edge. The two
               far corners share a value and the two near ones share another,
               but the original keeps all four apart - and the near-right one
               on the stack, which is what `hold` reads back. */
            y_tl = y1;
            y_tr = y1;
            y_bl = y0;
            held = y0;
            x1 = BTL_GRID_X1;
            x0 = BTL_GRID_X0;
            cells = base;
            do {
                if (cells[row] != 0) {
                    g_btl_arena_face[0].vx = x0;
                    g_btl_arena_face[0].vy = y_tl;
                    g_btl_arena_face[0].vz = 0;
                    g_btl_arena_face[1].vx = x1;
                    g_btl_arena_face[1].vy = y_tr;
                    g_btl_arena_face[1].vz = 0;
                    g_btl_arena_face[2].vx = x0;
                    g_btl_arena_face[2].vy = y_bl;
                    g_btl_arena_face[2].vz = 0;
                    g_btl_arena_face[3].vx = x1;
                    /* The block boundary is load-bearing: it is what puts
                       the projection's registers where the original has
                       them. Do not unwrap it. */
                    do {
                        g_btl_arena_face[3].vz = 0;
                        g_btl_arena_face[3].vy = *hold;
                        gte_ldv3(&g_btl_arena_face[0], &g_btl_arena_face[1],
                                 &g_btl_arena_face[2]);
                        gte_rtpt();
                        gte_stsxy3((long *)&g_btl_polyg4_next->x0,
                                   (long *)&g_btl_polyg4_next->x1,
                                   (long *)&g_btl_polyg4_next->x2);
                        gte_ldv0(&g_btl_arena_face[3]);
                        gte_rtps();
                        gte_stsxy((long *)&g_btl_polyg4_next->x3);
                        g_btl_polyg4_next->r0 = lit;
                        g_btl_polyg4_next->g0 = lit;
                        g_btl_polyg4_next->b0 = 0;
                    } while (0);
                    g_btl_polyg4_next->r1 = dim;
                    g_btl_polyg4_next->g1 = dim;
                    g_btl_polyg4_next->b1 = 0;
                    g_btl_polyg4_next->r2 = dim;
                    g_btl_polyg4_next->g2 = dim;
                    g_btl_polyg4_next->b2 = 0;
                    g_btl_polyg4_next->r3 = lit;
                    g_btl_polyg4_next->g3 = lit;
                    g_btl_polyg4_next->b3 = 0;
                    ot = (u_long *)(g_btl_prim_pool
                                    + g_btl_frame * BTL_FRAME_BYTES
                                    + BTL_ARENA_OT);
                    setaddr(g_btl_polyg4_next, getaddr(ot) & mask);
                    addr = (u_int)g_btl_polyg4_next & mask;
                    g_btl_polyg4_next++;
                    setaddr(ot, addr);
                }
                cells += BTL_GRID_ROWS;
                x1 += BTL_GRID_W;
                col++;
                x0 += BTL_GRID_W;
            } while (col < BTL_GRID_COLS);
            y0 += BTL_GRID_H;
            row++;
            y1 += BTL_GRID_H;
        } while (row < BTL_GRID_ROWS);
    }

    if (g_btl_arena_show != 0) {
        g_btl_polyft4_next = (POLY_FT4 *)(g_btl_prim_pool
                                          + g_btl_frame * BTL_FRAME_BYTES
                                          + BTL_ARENA_AT);
        BtlDrawArenaBack();
        BtlDrawArenaRight();
        BtlDrawArenaLeft();
        BtlDrawArenaBottom();
        BtlDrawArenaTop();
    }
    SetGeomScreen(BTL_DIST_FIXED);
    m = (const short *)g_btl_mesh;
    g_btl_polyft4_next = (POLY_FT4 *)(g_btl_prim_pool
                                      + g_btl_frame * BTL_FRAME_BYTES
                                      + BTL_MESH_AT);
    row = 0;
    if (g_btl_mesh_show != 0) {
        hidden = g_btl_mesh_hidden;
        do {
            col = 0;
            bit = BTL_MESH_TOP;
            far = m + 0x10A;
            do {
                col++;
                if ((bit & *hidden) == 0) {
                    /* The four corners of the square are the mesh points at
                       this column, the next, and the same two a row on. */
                    poly = g_btl_polyft4_next;
                    dist = m[1];
                    poly->x0 = dist;
                    dist = far[-0x107];
                    poly->y0 = dist;
                    dist = far[-0xFD];
                    poly->x1 = dist;
                    dist = far[-0xFB];
                    poly->y1 = dist;
                    dist = far[-0xD];
                    poly->x2 = dist;
                    dist = far[-0xB];
                    poly->y2 = dist;
                    dist = far[-1];
                    poly->x3 = dist;
                    dist = far[1];
                    poly->y3 = dist;
                    poly->r0 = g_btl_arena_rgb[0];
                    g_btl_polyft4_next->g0 = g_btl_arena_rgb[1];
                    g_btl_polyft4_next->b0 = g_btl_arena_rgb[2];
                    poly = g_btl_polyft4_next;
                    ot = (u_long *)(g_btl_prim_pool
                                    + g_btl_frame * BTL_FRAME_BYTES
                                    + BTL_ARENA_OT);
                    addPrim(ot, poly);
                }
                far += 12;
                m += 12;
                g_btl_polyft4_next++;
                bit >>= 1;
            } while (col < BTL_MESH_COLS);
            m += 12;
            row++;
            hidden++;
        } while (row < BTL_MESH_ROWS);
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/objects", BtlDrawObjects);
#endif

