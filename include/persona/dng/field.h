#ifndef PERSONA_DNG_FIELD_H
#define PERSONA_DNG_FIELD_H

/* Persona 1 (JP) - the 3D dungeon field.  DNG only.
 *
 * The field is drawn from one big scene buffer at 0x800C0000: every object
 * the floor can show gets a libgs object handle, a coordinate system and a
 * rotation, each in its own parallel array. The overlay's entry points
 * g_scene at the buffer and g_dng at the game state block at 0x801F0000,
 * and reaches both through those pointers from then on.
 */
#include <decomp/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

/* How many objects the scene has room for. */
#define SCENE_OBJS 968

typedef struct {
    u_char        pad0[0x1518];
    GsDOBJ2       objs[SCENE_OBJS];   /* 0x01518 */
    GsCOORDINATE2 coords[SCENE_OBJS]; /* 0x05198 */
    SVECTOR       rots[SCENE_OBJS];   /* 0x18018 */
} DngScene;

/* The game state block as the field sees it. Only the fields the overlay
   reaches are laid out; the rest belongs to the save. */
typedef struct {
    u_char  pad0[0x15A0];
    u_short map;         /* 0x15A0 */
    u_char  pad15A2[0x15C8 - 0x15A2];
    /* The door being opened: for each of its two halves, which axis of the
       translation moves and which scene object it is. */
    short   door_axis[2]; /* 0x15C8 */
    u_short door_obj[2];  /* 0x15CC */
} DngState;

extern DngScene *g_scene;
extern DngState *g_dng;

/* The countdown clock some floors run against, in hours, minutes, seconds
   and frames. It only runs while g_clock_on is set. */
extern signed char g_clock_hours;
extern signed char g_clock_min;
extern signed char g_clock_sec;
extern signed char g_clock_frame;
extern u_char g_clock_on;

/* Either of these stops the countdown without stopping the play clock. */
extern u_char g_clock_hold;
extern u_char g_clock_freeze;

/* Per map, the time the clock face counts down from. */
extern u_char g_clock_limit_hours[];
extern u_char g_clock_limit_min[];

/* The save's play clock. Everything but the hours rolls over at 60. */
extern u_char g_playtime_hours;
extern u_char g_playtime_min;
extern u_char g_playtime_sec;
extern u_char g_playtime_frame;

/* Counts frames round to 60 whatever else is running. */
extern u_char g_field_frames;

/* A door's two halves slide apart by (g_door_dx, g_door_dy) a frame for
   g_door_frames frames while g_door_state is 1. */
extern u_short g_door_state;
extern u_short g_door_frames;
extern short   g_door_dx;
extern short   g_door_dy;

/* Two vertex lists that bob back and forth, one step a call, turning round
   every `period` calls; the counters and directions are per channel. */
extern SVECTOR *g_bob_verts_a;
extern SVECTOR *g_bob_verts_b;
extern int      g_bob_count[];
extern int      g_bob_step[];

void CoordSetRot(SVECTOR *rot, GsCOORDINATE2 *coord);

void FieldBobVerts(int ch, int v, int period);
void FieldFadePrims(u_char *prim);
void FieldClockTick(int frames);
void FieldDoorSlide(void);
void FieldClockHands(u_short flags, int obj);

#endif
