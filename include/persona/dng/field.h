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
    u_char        pad19E58[0x364AC - 0x19E58];
    u_short       sky_angle;          /* 0x364AC the backdrop turns with the party */
    u_char        pad364AE[0x6E910 - 0x364AE];
    u_char        from_x, from_y;     /* 0x6E910 the tile a step leaves */
    u_char        pad6E912[2];
    long          sin, cos;           /* 0x6E914 of the party's angle */
} DngScene;

/* The game state block as the field sees it. Only the fields the overlay
   reaches are laid out; the rest belongs to the save. */
typedef struct {
    u_char   pad0[0x1580];
    GsRVIEW2 view;       /* 0x1580 the camera as last saved */
    u_short  map;        /* 0x15A0 */
    u_char   pad15A2[2];
    u_char   pos[3];     /* 0x15A4 the party's tile, [POS_X] and [POS_Y];
                            a step indexes it with g_dir_axis */
    u_char   facing;     /* 0x15A7 0-3, counting anticlockwise */
    u_char   walk_dir;   /* 0x15A8 the facing a step goes towards */
    u_char   pad15A9[3];
    long     angle;      /* 0x15AC 0-0xFFF, the view's heading */
    u_char   pad15B0[0x15BC - 0x15B0];
    u_char   tick_flags; /* 0x15BC */
    u_char   pad15BD;
    u_short  area;       /* 0x15BE the automap's map id */
    u_short  room;       /* 0x15C0 and its room within the map */
    u_char   pad15C2[0x15C8 - 0x15C2];
    /* The door being opened: for each of its two halves, which axis of the
       translation moves and which scene object it is. */
    short   door_axis[2]; /* 0x15C8 */
    u_short door_obj[2];  /* 0x15CC */
} DngState;

#define POS_X 0
#define POS_Y 2

extern DngScene *g_scene;
extern DngState *g_dng;

/* The floor is a grid of tile numbers FLOOR_W wide; each tile number picks a
   definition whose flags say what stepping on it does. */
#define FLOOR_W 24

typedef struct {
    u_char  pad0[0xA];
    u_short flags;
} TileDef;

#define TILE_KIND     0x1F   /* what the tile is, when TILE_SPECIAL is set */
#define TILE_MUSIC    0x200  /* stepping on it starts the floor's music */
#define TILE_SPECIAL  0x800
#define TILE_CLOCK    0x1000

#define TILE_KIND_DOOR 4

extern u_char (*g_floor_grid)[FLOOR_W];
extern TileDef *g_tile_defs;

/* The flags of the tile last stepped on, and its kind when special. */
extern int g_tile_flags;
extern int g_tile_kind;

/* Where the music was last started from; 0xF0 and up is nowhere. */
extern u_char g_music_x;
extern u_char g_music_y;

/* Per map, which of the two floor tunes it plays, and the two tunes. */
extern u_char g_map_music[][2];
extern short  g_floor_tune_a;
extern short  g_floor_tune_b;

extern u_char g_quest_bits;

/* The view the field is drawn from while walking, as two separate vectors
   rather than a GsRVIEW2 - the code never derives one's address from the
   other's. A step moves the eye and the target together, 30 units a frame along the facing's axis, and ends
   300 units on from where it began. */
extern long g_view_eye[3];
extern long g_view_at[3];
extern long g_step_from_eye;
extern long g_step_from_at;

/* Per facing, which axis of the view a step moves along (0 x, 2 z) and
   which way, and which way the party's tile moves along it. */
extern u_char g_dir_axis[];
extern long   g_dir_step[];
extern int    g_dir_tile_step[];

/* A step remembers the tile it leaves and, for a step undone, the one
   coordinate it changed. */
extern u_char g_walk_from_x;
extern u_char g_walk_from_y;
extern u_char g_walk_undo;

/* Any mode bits set, a step is refused without walking. FIELD_MODE_DOWN
   marks the stairs as leading down a floor. */
#define FIELD_MODE_DOWN 0x80
extern int g_field_mode;

/* The floor's tune pauses while the party stands; the first step resumes
   it, slowed down, unless the tune is off. */
#define BGM_RESUMED 0x80
extern u_char g_bgm_flags;
extern u_char g_bgm_off;
extern short  g_bgm_seq;

/* Whether the stairs just taken changed floor, and which way: 1 up, 2 down,
   0 not at all. */
extern int g_stair_floor;

/* Debugging: walls and one-way tiles stop nothing. */
extern u_char g_noclip;

/* The thud of walking into a wall. */
extern short g_bump_seq;

/* A tile that stops a step: off the grid, tile 0 or TILE_SOLID. A
   TILE_KIND_ONEWAY tile refuses the facings whose bit is set from bit 5. */
#define TILE_SOLID       0x8000
#define TILE_KIND_ONEWAY 2

/* How far a bump pushes the view in, per frame. */
#define BUMP_SPEED 18

/* Counted down a step at a time while the tick flags are all set; reaching
   zero raises g_effect_over. */
#define EFFECT_STEPS (*(u_char *)0x801F29A9)
#define STEP_CLOCK   (*(u_char *)0x801F2B30)
extern u_char g_effect_over;

/* A turn is nine frames of TURN_SPEED, then snaps to a quarter turn. */
#define TURN_SPEED   96
#define QUARTER_TURN 0x400

#define STEP_SPEED 30
#define STEP_LEN   300

/* The floor's pack, loaded at PACK_BASE, and its index at PACK_INDEX: a
   word saying where in the index the floor's counts start, then the count
   of entries in each of the pack's offset tables. Both are reached by
   address - the byte reads index a literal, which is why the image adds the
   index before the base. */
#define PACK_BASE   0x80130000
#define PACK_INDEX  ((u_char *)0x801DD000)
#define g_pack_sel  (*(int *)0x801DD000)
extern u_long *g_pack_images;
extern u_long *g_pack_tims;

extern u_char g_field_hold;

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
void UploadImageRows(void *desc, u_short x, u_short y, short rows);
void TimLoad(u_long *tim, int nopal);

void func_80065978(void);
void func_80069A7C(void);
void func_80069EB4(void);
void func_8006E988(int x, int y);
void func_8006FFF4(void);
void func_8006A4D0(void);
int  func_8006C9C8(void);
void func_8006CF40(void);
void func_80070DAC(int arg);
void func_8006A3CC(void);
void func_8006D33C(int arg);

void FieldBobVerts(int ch, int v, int period);
void FieldFadePrims(u_char *prim);
void FieldClockTick(int frames);
void FieldDoorSlide(void);
void FieldClockHands(u_short flags, int obj);

void FieldReload(void);
void FieldEnterTile(void);
void FieldStepView(void);
void FieldStepEnd(void);
void FieldSaveView(void);
void FieldStepBegin(void);

int  FieldWalk(int ret);
void FieldTurn(int turn);
void FieldSetHeading(int turn);

void FieldStairs(int dir);
void FieldHop(int dir);
int  FieldStepTick(void);
int  FieldTileOneWay(void);
int  FieldTileSolid(void);
void FieldBumpWall(void);
void FieldLoadAhead(void);

#endif
