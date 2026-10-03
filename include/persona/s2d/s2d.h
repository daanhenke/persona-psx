/* Persona 1 (JP) - S2D's own state, shared by its units. */
#ifndef PERSONA_S2D_S2D_H
#define PERSONA_S2D_S2D_H

#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

/* The screen fade: its colour, drawn as one semi-transparent quad. */
typedef struct {
    /* 0x00 */ u_char rgb[4];
    /* 0x04 */ int    unk04;
    /* 0x08 */ int    unk08;
} S2dFade;

extern S2dFade g_fade;
extern POLY_F4 g_fade_poly;

/* The camera the map is drawn through: its rotation and position. */
extern short g_s2d_cam_rx;
extern short g_s2d_cam_ry;
extern short g_s2d_cam_rz;
extern long  g_s2d_cam_x;
extern long  g_s2d_cam_y;
extern long  g_s2d_cam_z;

/* The exit the running map asks for (-1 while none), and which way the map
   is turned. */
extern short g_s2d_exit;
extern short g_s2d_heading;

/* The camera's turn towards the heading: its speed and its angle. */
extern short g_cam_turn_speed;
extern short g_cam_turn_angle;

/* The field's message window's state block. */
extern u_short D_800B91EC[];

/* Frames since the map began. */
extern u_int g_s2d_frame;

/* Which way the party faces, 0-3. */
extern short g_s2d_facing;

/* The ordering tables' tag buffers, carved out of the work buffer. */
extern GsOT_TAG *g_ot_tag_back;
extern GsOT_TAG *g_ot_tag_map;
extern GsOT_TAG *g_ot_tag_obj;
extern GsOT_TAG *g_ot_tag_layer[3];
extern GsOT_TAG *g_ot_tag_front;

/* A placed object: the ordering table it is sorted into (MAPOBJ_MAP,
   MAPOBJ_MAP2 or any other for the object layer), the model, the coordinate
   system it is drawn in and that system's transform. The map's own pieces
   are 160 of these in the second work buffer. */
typedef struct {
    /* 0x00 */ short         kind;
    /* 0x02 */ short         unk02;
    /* 0x04 */ GsDOBJ2       obj;
    /* 0x14 */ GsCOORDINATE2 coord;
    /* 0x64 */ SVECTOR       rot;
    /* 0x6C */ VECTOR        trans;
    /* 0x7C */ VECTOR        scale;
} S2dMapObj;

#define MAPOBJ_MAP  0
#define MAPOBJ_MAP2 0x80

extern S2dMapObj *g_map_objs;
extern int        g_map_obj_count;

typedef struct {
    SVECTOR rot;
    VECTOR  trans;
    VECTOR  scale;
} S2dXform;

extern GsCOORDINATE2 g_root_coord;
extern VECTOR        g_s2d_cam_scale;
extern GsCOORDINATE2 g_s2d_cam_coord;
extern S2dXform      g_map_xform;
extern GsRVIEW2      g_s2d_view;
extern GsRVIEW2      g_s2d_view2;
extern GsF_LIGHT     g_s2d_light;
/* The party's model and the mark drawn above it, the compass's five pieces
   (hung off the first), and three more. */
extern S2dMapObj     g_party_obj;
extern S2dMapObj     g_party_mark;
extern S2dMapObj     g_compass_objs[5];
/* Where the compass's pieces sit about its centre. */
extern VECTOR        g_compass_pos[5];
extern S2dMapObj     D_800B0B78;
extern S2dMapObj     D_800B0C04;
extern S2dMapObj     D_800B0C90;
extern S2dMapObj     D_800B0F1C;

/* The ordering tables, one pair per layer, and which of each pair is being
   built this frame. */
extern GsOT g_ot_back[2];
extern GsOT g_ot_map[2];
extern GsOT g_ot_obj[2];
extern GsOT g_ot_layer[3][2];
extern GsOT g_ot_front[2];

/* The draw modes added ahead of the object and back layers, and the draw
   environment read back each frame. */
extern DR_MODE g_scene_drmode[2];
extern DR_MODE g_menu_drmode[2];
extern DRAWENV g_s2d_drawenv;
extern int  g_draw_side;

/* Saved across a battle: where the party stood. */
extern u_char g_btl_map_id;
extern u_char g_btl_facing;
extern u_char g_btl_pos_x;
extern u_char g_btl_pos_y;

/* The field's state block: the mode the frame's update dispatches on and,
   from 0xC8, the script table S2dMarkScript searches. */
typedef struct {
    /* 0x000 */ short  unk00;
    /* 0x002 */ u_char unk02;   /* kept in the save block at 0x801F266B */
    /* 0x003 */ u_char unk03;
    /* 0x004 */ int    unk04;
    /* 0x008 */ short  unk08;
    /* 0x00A */ short  unk0A;
    /* 0x00C */ int    unk0C;
    /* 0x010 */ int    unk10;
    /* 0x014 */ int    unk14[3];
    /* 0x020 */ int    unk20;
    /* 0x024 */ int    mode;
    /* 0x028 */ int    unk28;
    /* 0x02C */ int    unk2C[3];
    /* 0x038 */ int    unk38;
    /* 0x03C */ int    unk3C;
    /* 0x040 */ int    unk40;
    /* 0x044 */ int    unk44;
    /* 0x048 */ u_char unk48[0x80];
    /* 0x0C8 */ int    scripts[120];
} S2dField;

extern S2dField D_800A4CFC;

/* Per map: which palette effect it runs (0xFF for none), and more. */
typedef struct {
    u_short effect;
    u_short pad[11];
} S2dMapInfo;

extern S2dMapInfo g_map_info[];

typedef struct {
    u_short mask;
    u_short pad;
} S2dKey;

/* The actions S2D reads the pad for, in the order its code tests them. */
extern S2dKey g_s2d_keys[10];

extern u_char g_map_pos_x;
extern u_char g_map_pos_y;
extern short  g_map_side;

#endif
