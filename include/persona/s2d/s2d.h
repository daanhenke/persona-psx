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

/* Which way the party faces, 0-3. */
extern short g_s2d_facing;

/* The ordering tables' tag buffers, carved out of the work buffer. */
extern GsOT_TAG *g_ot_tag_back;
extern GsOT_TAG *g_ot_tag_map;
extern GsOT_TAG *g_ot_tag_obj;
extern GsOT_TAG *g_ot_tag_layer[3];
extern GsOT_TAG *g_ot_tag_front;

/* Saved across a battle: where the party stood. */
extern u_char g_btl_map_id;
extern u_char g_btl_facing;
extern u_char g_btl_pos_x;
extern u_char g_btl_pos_y;

extern u_char g_map_pos_x;
extern u_char g_map_pos_y;
extern short  g_map_side;

#endif
