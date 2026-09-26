/* Persona 1 (JP) - the TYN cutscene's two turning pictures.  ADV only.
 *   0x8008A6FC TynDrawPlanes   0x8008A93C TynSyncPlanes
 *   0x8008A9B4 TynInitPolys    0x8008AC50 TynInitDivide
 *   0x8008AE2C TynSetView      0x8008AE80 TynInitCoords
 *
 * AdvTynCutscene (tyncut.c) shows two pictures in 3D, back to back: one quad
 * of four vertices, placed twice by two coordinate systems, the second turned
 * half way round from the first. Each frame the quad is transformed for both
 * and either added to the OT as it is or, when a subdivision is asked for,
 * cut up by DivideFT4 into a prim area of its own.
 *
 * The polygons are kept per frame buffer and per picture. The set-up gives
 * the second picture's UVs to [0][1] and its CLUT and page to [1][0], so the
 * two buffers do not agree about which picture is which; the subdivided path
 * takes its UVs from the tables here instead and is not affected.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

#define PLANES 2

/* One DivideFT4 corner: a u,v pair and the CLUT or page beside it. */
typedef struct {
    u_char  u;
    u_char  v;
    u_short cba;
} TynUV;

extern SVECTOR       g_char_rot[PLANES];
extern SVECTOR       g_quad_verts[4];
extern GsCOORDINATE2 g_tyn_coords[PLANES];
extern POLY_FT4      g_tyn_polys[2][PLANES];
extern POLY_FT4      g_tyn_heap[2][PLANES][16];
/* The corners DivideFT4 is handed, one of each per picture. The first
   table has a third entry nothing uses. */
extern TynUV         g_tyn_uv0[3];
extern TynUV         g_tyn_uv1[PLANES];
extern TynUV         g_tyn_uv2[PLANES];
extern TynUV         g_tyn_uv3[PLANES];
/* The colour DivideFT4 is handed, and its subdivision settings. */
extern CVECTOR       g_tyn_rgbc;
extern DIVPOLYGON4   g_tyn_div;
extern int           g_tyn_pad;

extern void CoordSetRot(SVECTOR *rot, GsCOORDINATE2 *coord);
extern void func_80033BE0(int h);
extern void func_80033A40(int z);

void TynDrawPlanes(GsCOORDINATE2 *unused, int buf, u_long *ot, int unused2)
{
    MATRIX    ls;
    long      p;
    long      otz;
    long      flag;
    POLY_FT4 *poly;
    long      nclip;
    int       i;

    for (i = 0; i < PLANES; i++) {
        CoordSetRot(&g_char_rot[i], &g_tyn_coords[i]);
        GsGetLs(&g_tyn_coords[i], &ls);
        GsSetLightMatrix(&ls);
        GsSetLsMatrix(&ls);
        poly = &g_tyn_polys[buf][i];
        nclip = RotAverageNclip4(&g_quad_verts[0], &g_quad_verts[1],
                                 &g_quad_verts[2], &g_quad_verts[3],
                                 (long *)&poly->x0, (long *)&poly->x1,
                                 (long *)&poly->x2, (long *)&poly->x3,
                                 &p, &otz, &flag);
        if (g_tyn_div.ndiv == 0) {
            if (flag >= 0 && nclip >= 0 && otz > 0) {
                AddPrim(ot + (otz >> 3), poly);
            }
        } else {
            DivideFT4(&g_quad_verts[0], &g_quad_verts[1], &g_quad_verts[2],
                      &g_quad_verts[3], (u_long *)&g_tyn_uv0[i],
                      (u_long *)&g_tyn_uv1[i], (u_long *)&g_tyn_uv2[i],
                      (u_long *)&g_tyn_uv3[i], &g_tyn_rgbc,
                      g_tyn_heap[buf][i], ot + 0x80, &g_tyn_div);
        }
    }
}

/* The second picture follows the first, turned half way round. */
void TynSyncPlanes(int unused, int pad)
{
    g_tyn_coords[1].coord.t[0] = g_tyn_coords[0].coord.t[0];
    g_tyn_coords[1].coord.t[1] = g_tyn_coords[0].coord.t[1];
    g_tyn_coords[1].coord.t[2] = g_tyn_coords[0].coord.t[2];
    g_tyn_pad = pad;
    g_char_rot[1].vy = g_char_rot[0].vy + 0x800;
    g_char_rot[1].vx = g_char_rot[0].vx;
    g_char_rot[1].vz = -g_char_rot[0].vz;
}

void TynInitPolys(void)
{
    POLY_FT4 *p;

    p = &g_tyn_polys[0][0];
    setPolyFT4(p);
    g_tyn_rgbc.b = 0x80;
    g_tyn_rgbc.g = 0x80;
    g_tyn_rgbc.r = 0x80;
    setRGB0(p, 0x80, 0x80, 0x80);
    g_tyn_rgbc.cd = 0x2C;
    setXY4(p, -0x30, -0x48, 0x30, -0x48, -0x30, 0x48, 0x30, 0x48);
    setUV4(p, 0, 0, 0x60, 0, 0, 0x90, 0x60, 0x90);
    p->clut = GetClut(0, 0x1E6);
    p->tpage = GetTPage(1, 0, 0x140, 0x100);
    g_tyn_polys[0][1] = g_tyn_polys[0][0];
    g_tyn_polys[1][0] = g_tyn_polys[0][0];
    setUV4(&g_tyn_polys[0][1], 0, 0, 0x60, 0, 0, 0x90, 0x60, 0x90);
    g_tyn_polys[1][0].clut = GetClut(0, 0x1E7);
    g_tyn_polys[1][0].tpage = GetTPage(1, 0, 0x300, 0x100);
    g_tyn_polys[1][1] = g_tyn_polys[0][1];
}

void TynInitDivide(void)
{
    setVector(&g_quad_verts[0], 0xC0, -0x120, 0);
    setVector(&g_quad_verts[1], -0xC0, -0x120, 0);
    setVector(&g_quad_verts[2], 0xC0, 0x120, 0);
    setVector(&g_quad_verts[3], -0xC0, 0x120, 0);
    g_tyn_uv0[0].u = 0;
    g_tyn_uv0[0].v = 0;
    g_tyn_uv0[0].cba = GetClut(0, 0x1E6);
    g_tyn_uv1[0].u = 0x60;
    g_tyn_uv1[0].v = 0;
    g_tyn_uv1[0].cba = GetTPage(1, 0, 0x140, 0x100);
    g_tyn_uv0[1] = g_tyn_uv0[0];
    g_tyn_uv1[1] = g_tyn_uv1[0];
    g_tyn_uv2[0].u = 0;
    g_tyn_uv2[0].v = 0x90;
    g_tyn_uv2[1] = g_tyn_uv2[0];
    g_tyn_uv3[0].u = 0x60;
    g_tyn_uv3[0].v = 0x90;
    g_tyn_uv3[1] = g_tyn_uv3[0];
    g_tyn_uv0[1].cba = GetClut(0, 0x1E7);
    g_tyn_uv1[1].cba = GetTPage(1, 0, 0x300, 0x100);
    g_tyn_div.ndiv = 1;
    g_tyn_div.pih = 0x140;
    g_tyn_div.piv = 0xF0;
}

void TynSetView(void)
{
    GsRVIEW2 view;

    func_80033BE0(300);
    view.vpx = 0;
    view.vpy = 0;
    view.vpz = 2000;
    view.vrx = 0;
    view.vry = 0;
    view.vrz = 0;
    view.rz = 0;
    view.super = WORLD;
    GsSetRefView2(&view);
    func_80033A40(100);
}

void TynInitCoords(void)
{
    GsInitCoordinate2(WORLD, &g_tyn_coords[0]);
    GsInitCoordinate2(WORLD, &g_tyn_coords[1]);
    g_tyn_coords[0].coord.t[1] = -150;
    g_tyn_coords[0].coord.t[2] = 0;
    g_tyn_coords[1].coord.t[2] = 0;
    g_char_rot[1].vx = g_char_rot[1].vy = g_char_rot[1].vz = 0;
    g_char_rot[0].vx = g_char_rot[0].vy = g_char_rot[0].vz = 0;
    g_tyn_coords[0].coord.t[2] = -240;
    g_tyn_coords[1].coord.t[0] = g_tyn_coords[0].coord.t[0] = -300;
    g_char_rot[0].vz = -240;
    setVector(&g_quad_verts[0], 0x90, -0xD8, 0);
    setVector(&g_quad_verts[1], -0x90, -0xD8, 0);
    setVector(&g_quad_verts[2], 0x90, 0xD8, 0);
    setVector(&g_quad_verts[3], -0x90, 0xD8, 0);
}
