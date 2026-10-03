/* Persona 1 (JP) - the map's coordinate systems, view and light.
 *   0x8008E288 S2dInitCoords  0x8008E588 S2dInitView  0x8008E634 S2dInitLight
 *   0x8008E6A8 S2dPlaceCompass  0x8008E738 S2dBindModels
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
#include <persona/s2d/model.h>
#include <decomp/include_asm.h>

#define MAP_OBJS 160

extern VECTOR        D_800A4FA4[5];

extern void func_80033A40(int z);
extern short g_compass_x;
extern short g_compass_y;
extern void  S2dCompassMoveTo(int x, int y, int step);
extern void  func_8009994C(int x, int y, int w, int h, int a, int b, int c);

#ifdef NON_MATCHING
void S2dInitCoords(void)
{
    S2dMapObj     *m;
    S2dMapObj     *o;
    GsCOORDINATE2 *c;
    int        i;

    m = g_map_objs;
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
    GsInitCoordinate2(&g_root_coord, &g_party_obj.coord);

    c = &g_party_mark.coord;
    g_party_obj.trans.vx = 0;
    g_party_obj.trans.vy = 0;
    g_party_obj.trans.vz = 0;
    g_party_obj.rot.vx = 0;
    g_party_obj.rot.vy = 0;
    g_party_obj.rot.vz = 0;
    g_party_obj.scale.vx = 0x1800;
    g_party_obj.scale.vy = 0x1800;
    g_party_obj.scale.vz = 0x1800;
    GsInitCoordinate2(&g_root_coord, c);

    c = &g_compass_objs[0].coord;
    g_party_mark.trans.vx = 0;
    g_party_mark.trans.vy = 0;
    g_party_mark.trans.vz = 0;
    g_party_mark.rot.vx = 0;
    g_party_mark.rot.vy = 0;
    g_party_mark.rot.vz = 0;
    g_party_mark.scale.vx = 0x1400;
    g_party_mark.scale.vy = ONE;
    g_party_mark.scale.vz = ONE;
    GsInitCoordinate2(NULL, c);
    for (i = 1; i < 5; i++) {
        GsInitCoordinate2(c, &g_compass_objs[i].coord);
    }
    o = &g_compass_objs[0];
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

    GsInitCoordinate2(&g_root_coord, &D_800B0F1C.coord);
    D_800B0F1C.trans.vx = 0;
    D_800B0F1C.trans.vy = 0;
    D_800B0F1C.trans.vz = 0;
    D_800B0F1C.rot.vx = 0;
    D_800B0F1C.rot.vy = 0;
    D_800B0F1C.rot.vz = 0;
    D_800B0F1C.scale.vx = ONE;
    D_800B0F1C.scale.vy = ONE;
    D_800B0F1C.scale.vz = ONE;
    D_800B0F1C.kind = 0;
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
    GsSetProjection(0x280);
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

/* Where the compass sits depends on which way the map is turned. */
void S2dPlaceCompass(void)
{
    short *p;

    if (g_s2d_heading != 0) {
        g_compass_x = 0x4C;
        g_compass_y = -0x68;
    } else {
        g_compass_x = -0xE8;
        g_compass_y = -0x68;
    }
    p = &g_compass_x;
    S2dCompassMoveTo(*p + 0x58, -0x18, 0x1000);
    func_8009994C(*p + 0x10F, 0x18, 0x98, 0x68, 0, 0, 0x1000);
}

#ifdef NON_MATCHING
/* The map's model and the party's marker take their first objects from
   model slot 0, with the marker placed where the map is. */
void S2dBindModels(void)
{
    g_party_obj.obj.tmd = g_models[0].objs;
    *(u_long *)g_party_obj.obj.tmd[4] &= 0xFF000000;
    *(u_long *)g_party_obj.obj.tmd[4] |= g_party_obj.obj.tmd[5];
    g_party_obj.obj.attribute = 0;
    g_party_obj.obj.coord2 = &g_party_obj.coord;

    g_party_obj.rot.vx = g_map_xform.rot.vx;
    g_party_mark.obj.tmd = g_models[0].objs + 0x2A;
    g_party_obj.trans = g_map_xform.trans;
    *(u_long *)g_party_mark.obj.tmd[4] &= 0xFF000000;
    *(u_long *)g_party_mark.obj.tmd[4] |= g_party_mark.obj.tmd[5];
    g_party_mark.obj.attribute = 0;
    g_party_mark.obj.coord2 = &g_party_mark.coord;

    g_party_mark.rot.vx = g_map_xform.rot.vx;
    g_party_mark.trans = g_map_xform.trans;
    g_party_mark.trans.vy -= 0x6A;
}
#else
/* 62%: the image holds &obj.tmd in a register and derives coord2 from it;
   the statement order around it is not settled. */
INCLUDE_ASM("s2d/nonmatchings/game/scenecoords", S2dBindModels);
#endif
