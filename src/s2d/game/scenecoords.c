/* Persona 1 (JP) - the map's coordinate systems, view and light.
 *   0x8008E288 S2dInitCoords  0x8008E588 S2dInitView  0x8008E634 S2dInitLight
 *
 * Everything S2D draws in 3D hangs off one root coordinate system: the
 * camera, the map, the party's marker and the other placed objects, and the
 * map's 160 pieces, which hang off the camera. S2dInitCoords links them all
 * up with an identity transform (a few objects start at their own place or
 * scale); S2dInitView sets the projection and the eye; S2dInitLight one white
 * flat light and a grey ambient.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <persona/s2d/s2d.h>
#include <decomp/include_asm.h>

#define MAP_OBJS 160

extern int           D_800B9150;
extern VECTOR        D_800A4FA4[5];
extern GsCOORDINATE2 D_800B0F30;
extern S2dXform      D_800B0F80;
extern short         D_800B0F1C;

extern void func_80033BE0(int h);
extern void func_80033A40(int z);

#ifdef NON_MATCHING
void S2dInitCoords(void)
{
    S2dMapObj     *m;
    S2dObj        *o;
    GsCOORDINATE2 *c;
    int        i;

    m = (S2dMapObj *)D_800B9150;
    GsInitCoordinate2(NULL, &g_root_coord);

    g_s2d_cam_x = 0;
    g_s2d_cam_y = 0;
    g_s2d_cam_z = 0;
    g_s2d_cam_rx = 0;
    g_s2d_cam_ry = 0;
    g_s2d_cam_rz = 0;
    g_s2d_cam_scale.vx = ONE;
    g_s2d_cam_scale.vy = ONE;
    g_s2d_cam_scale.vz = ONE;
    GsInitCoordinate2(&g_root_coord, &g_s2d_cam_coord);

    g_map_xform.trans.vx = 0;
    g_map_xform.trans.vy = 0;
    g_map_xform.trans.vz = 0;
    g_map_xform.rot.vx = 0;
    g_map_xform.rot.vy = 0;
    g_map_xform.rot.vz = 0;
    g_map_xform.scale.vx = ONE;
    g_map_xform.scale.vy = ONE;
    g_map_xform.scale.vz = ONE;
    GsInitCoordinate2(&g_root_coord, &g_map_coord);

    c = &g_s2d_obj.coord;
    g_s2d_obj.trans.vx = 0;
    g_s2d_obj.trans.vy = 0;
    g_s2d_obj.trans.vz = 0;
    g_s2d_obj.rot.vx = 0;
    g_s2d_obj.rot.vy = 0;
    g_s2d_obj.rot.vz = 0;
    g_s2d_obj.scale.vx = 0x1800;
    g_s2d_obj.scale.vy = 0x1800;
    g_s2d_obj.scale.vz = 0x1800;
    GsInitCoordinate2(&g_root_coord, c);

    c = &g_s2d_objs[0].coord;
    g_s2d_objs[0].trans.vx = 0;
    g_s2d_objs[0].trans.vy = 0;
    g_s2d_objs[0].trans.vz = 0;
    g_s2d_objs[0].rot.vx = 0;
    g_s2d_objs[0].rot.vy = 0;
    g_s2d_objs[0].rot.vz = 0;
    g_s2d_objs[0].scale.vx = 0x1400;
    g_s2d_objs[0].scale.vy = ONE;
    g_s2d_objs[0].scale.vz = ONE;
    GsInitCoordinate2(NULL, c);
    for (i = 1; i < 5; i++) {
        GsInitCoordinate2(c, &g_s2d_objs[i].coord);
    }
    o = &g_s2d_objs[1];
    for (i = 0; i < 5; i++) {
        o[i].trans = D_800A4FA4[i];
        o[i].rot.vx = 0;
        o[i].rot.vy = 0;
        o[i].rot.vz = 0;
        o[i].scale.vx = ONE;
        o[i].scale.vy = ONE;
        o[i].scale.vz = ONE;
    }

    for (i = 0; i < MAP_OBJS; i++, m++) {
        GsInitCoordinate2(&g_s2d_cam_coord, &m->coord);
        m->scale.vx = ONE;
        m->scale.vy = ONE;
        m->scale.vz = ONE;
    }

    GsInitCoordinate2(&g_root_coord, &D_800B0F30);
    D_800B0F80.trans.vx = 0;
    D_800B0F80.trans.vy = 0;
    D_800B0F80.trans.vz = 0;
    D_800B0F80.rot.vx = 0;
    D_800B0F80.rot.vy = 0;
    D_800B0F80.rot.vz = 0;
    D_800B0F80.scale.vx = ONE;
    D_800B0F80.scale.vy = ONE;
    D_800B0F80.scale.vz = ONE;
    D_800B0F1C = 0;
}
#else
/* 95%: the image steps the five transforms with three pointers taken off
   the coord pointer (rot, trans, scale); loop.c here folds them into one
   pointer and displacements. */
INCLUDE_ASM("s2d/nonmatchings/game/scenecoords", S2dInitCoords);
#endif

void S2dInitView(void)
{
    u_char unused[0x18];
    func_80033BE0(0x280);
    g_s2d_view.vpx = 0;
    g_s2d_view.vpy = 0;
    g_s2d_view.vpz = -0x500;
    g_s2d_view.vrx = 0;
    g_s2d_view.vry = 0;
    g_s2d_view.vrz = 0;
    g_s2d_view.rz = 0;
    g_s2d_view.super = NULL;
    g_s2d_view2.vpx = 0;
    g_s2d_view2.vpy = 0;
    g_s2d_view2.vpz = 0;
    g_s2d_view2.vrx = 0;
    g_s2d_view2.vry = 0;
    g_s2d_view2.vrz = 0;
    g_s2d_view2.rz = 0;
    g_s2d_view2.super = NULL;
    func_80033A40(0xA0);
}

void S2dInitLight(void)
{
    g_s2d_light.vx = 100;
    g_s2d_light.vy = 100;
    g_s2d_light.vz = 100;
    g_s2d_light.r = 0xFF;
    g_s2d_light.g = 0xFF;
    g_s2d_light.b = 0xFF;
    GsSetFlatLight(0, &g_s2d_light);
    GsSetAmbient(0x555, 0x555, 0x555);
    GsSetLightMode(0);
}
