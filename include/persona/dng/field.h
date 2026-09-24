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

/* The flat sprites drawn over the field: exactly as many as fit before the
   objects. */
#define FIELD_SPRITES 150

typedef struct {
    GsSPRITE      sprites[FIELD_SPRITES]; /* 0x00000 */
    GsDOBJ2       objs[SCENE_OBJS];   /* 0x01518 */
    GsCOORDINATE2 coords[SCENE_OBJS]; /* 0x05198 */
    SVECTOR       rots[SCENE_OBJS];   /* 0x18018 */
    u_char        pad19E58[0x1AE80 - 0x19E58];
    GsOT          ot[2];              /* 0x1AE80 one per display buffer */
    u_char        pad1AEA8[0x36364 - 0x1AEA8];
    GsSPRITE      backdrop;           /* 0x36364 */
    u_char        pad36388[0x3649C - 0x36388];
    GsSPRITE      sky;                /* 0x3649C scrolls sideways as the party turns */
    u_char        pad364C0[0x364D4 - 0x364C0];
    PACKET        packets[2][0x1C000]; /* 0x364D4 one per display buffer */
    GsF_LIGHT     light;              /* 0x6E4D4 the field's one flat light */
    u_char        pad6E4E4[0x6E910 - 0x6E4E4];
    u_char        from_x, from_y;     /* 0x6E910 the tile a step leaves */
    u_char        pad6E912[2];
    long          sin, cos;           /* 0x6E914 of the party's angle */
    u_char        pad6E91C[0x6E97C - 0x6E91C];
    u_long        glyph[16][2];       /* 0x6E97C a glyph decoded for upload */
    u_char        pad6E9FC[8];
    u_char        flag6EA04;          /* 0x6EA04 cleared for a new floor */
} DngScene;

/* The game state block as the field sees it. Only the fields the overlay
   reaches are laid out; the rest belongs to the save. */
typedef struct {
    u_char   pad0[0x1580];
    GsRVIEW2 view;       /* 0x1580 the camera as last saved */
    u_short  map;        /* 0x15A0 */
    u_short  floor;      /* 0x15A2 which of the map's floors */
    u_char   pos[3];     /* 0x15A4 the party's tile, [POS_X] and [POS_Y];
                            a step indexes it with g_dir_axis */
    u_char   facing;     /* 0x15A7 0-3, counting anticlockwise */
    u_char   walk_dir;   /* 0x15A8 the facing a step goes towards */
    u_char   pad15A9[3];
    long     angle;      /* 0x15AC 0-0xFFF, the view's heading */
    u_char   flag15B0;   /* 0x15B0 cleared coming back from S2D or ADV */
    u_char   pad15B1;
    u_char   map_seen_only; /* 0x15B2 the minimap hides tiles not yet seen */
    u_char   pad15B3[0x15BC - 0x15B3];
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

/* FieldSetFloor reads the floor as a plain halfword off the state pointer,
   not as a member: read as g_dng->floor, gcc 2.6 takes a struct member and a
   fixed global as unable to alias and hoists every read above the stores in
   between, where the image rereads it after each one. */
#define DNG_FLOOR (*(u_short *)((u_char *)g_dng + 0x15A2))

extern DngScene *g_scene;
extern DngState *g_dng;

/* The floor is a grid of tile numbers FLOOR_W wide; each tile number picks a
   definition whose flags say what stepping on it does. */
#define FLOOR_W 24

typedef struct {
    u_char  pad0[8];
    u_char  icon;    /* 0x8 its minimap cell */
    u_char  pad9;
    u_short flags;   /* 0xA */
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
#define g_floor_tune_a (g_seq_handles[13])
#define g_floor_tune_b (g_seq_handles[14])

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
#define g_bump_seq (g_seq_handles[12])

/* A tile that stops a step: off the grid, tile 0 or TILE_SOLID. A
   TILE_KIND_ONEWAY tile refuses the facings whose bit is set from bit 5. */
#define TILE_SOLID       0x8000
#define TILE_KIND_ONEWAY 2

/* How far a bump pushes the view in, per frame. */
#define BUMP_SPEED 18

/* Counted down a step at a time while the tick flags are all set; reaching
   zero raises g_effect_over. */
#define EFFECT_STEPS (*(u_char *)0x801F29A9)
/* The moon's phase, advanced a step at a time; 16 phases. */
#define MOON_PHASE   (*(u_char *)0x801F2B30)
extern u_char g_moon_cells[];
extern u_char g_effect_over;

/* Set while the field is faded in. */
extern u_char g_field_lit;

/* Played in place of the floor's tune while the party stands still. */
extern short g_idle_seq;

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
#define INDEX_BASE  0x801DD000

/* The pack's and the index's offset tables, as the entry point finds them:
   one entry for the models, one per floor for the rest. */
extern u_long *g_pack_model_tab;
extern u_long *g_pack_cell_tab;
extern u_long *g_pack_obj_tab;
extern u_long *g_pack_spot_tab;
extern u_long *g_pack_event_tab;
extern u_long *g_index_tile_tab;
extern u_long *g_index_info_tab;
extern u_long *g_index_grid_tab;

/* The current floor's entries. g_floor_spots is a list the field matches
   against the party's tile; g_floor_events carries what a spot starts. */
extern u_char *g_model_defs;
extern u_char *g_floor_info;
extern u_char *g_floor_objs;
extern u_char *g_floor_spots;
extern u_char *g_floor_events;

/* Cleared by FieldSetFloor; nothing here says more about them. */
extern int    D_8009FDEC;
extern int    D_800993B8;
extern u_char D_8009FE3C;
extern u_char D_8009FAD0;
extern u_char D_800A059C;
extern int    D_800A02E0;
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
void TimLoadAt(u_long *tim, int x, int y);

void func_80065978(void);
void func_80069A7C(void);
void func_80069EB4(void);
void func_8007192C(void);
void func_800713B0(int kind);
int  func_8006C9C8(void);
void func_80070DAC(int arg);
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

void FieldFadeOut(void);
void FieldFadeIn(void);
int  FieldPauseBgm(void);

int  FieldOpenDoor(void);
void FieldInitGraph(void);
void FieldSetFloor(void);
void FieldEnterFrom(void);

u_long *FieldMapTmd(u_long *tmd);
void FieldSetPrimClut(u_short *list, u_short clut);
void FieldDecodeGlyph(u_short n);
void FieldSetMoonIcon(void);
/* Defined old-style, so declared without a prototype: callers pass ints. */
void FieldInitSprite();

void FieldLoadWallCluts(void);
void FieldInitStrip(void);
void FieldRebuildMap(void);
void FieldSetCell(int x, int y);
void FieldResetSound(void);

/* Every sequence libsnd has open for the field, -1 when free. In the save
   area and reached by address. */
#define SEQ_HANDLES     32
#define g_seq_handles   ((short *)0x801F537C)
#define g_vab_handles   ((short *)0x801F535C)

/* How many of the handles the field's own sequences use, and the VABs. */
#define FIELD_SEQS 19
#define FIELD_VABS 3

/* Per sequence, the handle it is opened into. */
extern signed char g_seq_slot[];

/* Which display buffer is being drawn into. */
extern int g_draw_buf;

/* What a battle transition moves: the scene's objects and the offset it
   applies to them. */
extern GsDOBJ2       *g_fx_objs;
extern GsCOORDINATE2 *g_fx_coords;
extern SVECTOR       *g_fx_rots;
extern SVECTOR        g_fx_shift;

void FieldFxBegin(void);
void FieldFxRun(int kind);

/* The wavy transition's per-column stretch speed, and the frames each
   column waits before it starts to stretch. */
extern int g_wave_speed[];
extern int g_wave_delay[];

void FieldFadeFxBegin(void);
void FieldFadeFxRun(void);
void FieldFadeFxStep(void);
void FieldFadeFxSetup(void);
void FieldWaveFxBegin(void);
void FieldWaveFxRun(void);
void FieldWaveFxStep(void);
void FieldIrisFxBegin(void);
void FieldIrisFxRun(void);
void FieldIrisFxStep(void);
void FieldIrisFxSetup(void);

void FieldShake(int dir);
void FieldNudge(int frames, int dy);

void FieldStartBattle(void);
void FieldPlayJingle(u_char n, short loops);
void FieldPlaySeq(); /* old-style: callers pass ints */

/* The fog libgs is given, the scale its depth is built from, and the
   distance it starts at. */
extern GsFOGPARAM g_fog;
extern double     g_fog_scale;
extern int        g_fog_near;

/* The floor's entry tile, laid out like the state's pos: x at [0], y at
   [2]; 0xFF when the floor has none. */
extern u_char g_entry_pos[3];

/* Set when poison lands on the field; nothing here says more. */
extern u_char D_800993C6;

void FieldDamageFloor(int div);
void FieldPoisonFloor(void);

int  FieldFindEntry(void);
void FieldLoadGfx(void);

void FieldFadeSeqs(void);
void FieldCloseSound(void);
void FieldSetView(int keep_height);
void FieldInitLight(void);

#endif
