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

/* The selection window's layout; kind 0 means none, cleared for each new
   floor. */
typedef struct {
    u_short first;   /* 0x0 */
    u_short y;       /* 0x2 */
    u_short count;   /* 0x4 */
    u_short rows;    /* 0x6 */
    u_char  kind;    /* 0x8 */
    u_char  pad9[3];
    u_long *src;     /* 0xC */
} DngWindow;

/* A mapped TMD's object list. A struct of one member, not a bare pointer:
   indexed, gcc then adds the index after the scene's address, as the image
   does. */
typedef struct {
    u_long *tmd;
} SceneModel;

/* The flat sprites drawn over the field: exactly as many as fit before the
   objects. */
#define FIELD_SPRITES 150

typedef struct {
    GsSPRITE      sprites[FIELD_SPRITES]; /* 0x00000 */
    GsDOBJ2       objs[SCENE_OBJS];   /* 0x01518 */
    GsCOORDINATE2 coords[SCENE_OBJS]; /* 0x05198 */
    SVECTOR       rots[SCENE_OBJS];   /* 0x18018 */
    GsOT          world_ot[2];        /* 0x19E58 the 3D field's, one per display buffer */
    u_char        pad19E80[0x1AE80 - 0x19E80];
    GsOT          ot[2];              /* 0x1AE80 one per display buffer */
    u_char        pad1AEA8[0x36364 - 0x1AEA8];
    GsSPRITE      backdrop;           /* 0x36364 */
    u_char        pad36388[0x3640A - 0x36388];
    short         msg_scroll_a;       /* 0x3640A the message box's scroll */
    u_char        pad3640C[6];
    short         msg_scroll_b;       /* 0x36412 */
    u_char        pad36414[0x3649C - 0x36414];
    GsSPRITE      sky;                /* 0x3649C scrolls sideways as the party turns */
    u_char        pad364C0[0x364D4 - 0x364C0];
    PACKET        packets[2][0x1C000]; /* 0x364D4 one per display buffer */
    GsF_LIGHT     light;              /* 0x6E4D4 the field's one flat light */
    u_char        pad6E4E4[0x6E504 - 0x6E4E4];
    int           pad_held;           /* 0x6E504 the pad buttons held */
    int           pad_new;            /* 0x6E508 and newly pressed */
    SceneModel    models[257];        /* 0x6E50C the floor's TMDs, from 1 */
    u_char        from_x, from_y;     /* 0x6E910 the tile a step leaves */
    u_char        pad6E912[2];
    long          sin, cos;           /* 0x6E914 of the party's angle */
    u_char        pad6E91C[0x6E97C - 0x6E91C];
    u_long        glyph[16][2];       /* 0x6E97C a glyph decoded for upload */
    DngWindow     win;                /* 0x6E9FC the selection window */
    u_char        pad6EA0C[0x6EACC - 0x6EA0C];
    signed char   lift_x, lift_y;     /* 0x6EACC which lift on the floor */
    u_char        lift;               /* 0x6EACE its row of g_lift_stops */
    u_char        pad6EACF;
    u_char        lift_at;            /* 0x6EAD0 the floor its indicator shows */
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
    u_char   enc_calm;   /* 0x15B0 steps left before the next encounter can
                            be rolled; cleared coming back from S2D or ADV */
    u_char   no_enc;     /* 0x15B1 set, no encounters are rolled */
    u_char   map_seen_only; /* 0x15B2 the minimap hides tiles not yet seen */
    u_char   pad15B3;
    u_short  last_map;   /* 0x15B4 the map the pack index was last copied for */
    u_short  last_floor; /* 0x15B6 and its floor then */
    u_char   pad15B8;
    u_char   exit_bits;  /* 0x15B9 the 0xC0 bits of the room byte of the
                            exit last taken into ADV */
    u_char   pad15BA[0x15BC - 0x15BA];
    u_char   tick_flags; /* 0x15BC */
    u_char   pad15BD;
    u_short  area;       /* 0x15BE the automap's map id */
    u_short  room;       /* 0x15C0 and its room within the map */
    u_short  scene6_id;  /* 0x15C2 where an exit into scene 6 leads; ADV's
                            scripts set it too */
    u_char   pad15C4[0x15C8 - 0x15C4];
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
    u_char  models[8]; /* 0x0 the models its eight objects show */
    u_char  icon;    /* 0x8 its minimap cell */
    u_char  pad9;
    u_short flags;   /* 0xA */
} TileDef;

#define TILE_KIND     0x1F   /* what the tile is, when TILE_SPECIAL is set */
#define TILE_MUSIC    0x200  /* stepping on it starts the floor's music */
#define TILE_SPECIAL  0x800
#define TILE_CLOCK    0x1000
#define TILE_EVENT    0x4000 /* a spot in g_floor_events is on it */
#define TILE_ZONE_TUNE 0x2000 /* the zone's tunes play on it */
#define TILE_QUIET    0x400  /* its event does not start on stepping */

#define TILE_KIND_DOOR 4
#define TILE_KIND_LOCK 1  /* a locked door: its event opens it */

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
#define BGM_ZONE    0x01 /* the zone's tunes are the ones playing */
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
/* A model a tile can show: its TMD, the object attribute, its offset from
   the tile's corner and its rotation in degrees. */
typedef struct {
    u_char  tmd;
    u_char  pad1[3];
    u_long  attr;     /* 0x4 */
    u_char  pad8[4];
    short   x, y, z;  /* 0xC */
    short   rx, ry, rz; /* 0x12 */
} ModelDef;

extern ModelDef *g_model_defs;
extern u_long   *g_pack_tmd_tab;

/* Per scene object; nothing here says more. */
extern int D_8009CD50[];
extern int D_8009DC70[];
extern int D_8009EB90[];
extern u_char *g_floor_info;
extern u_char *g_floor_objs;
extern u_char *g_floor_spots;
extern u_char *g_floor_events;

/* A spot of g_floor_events, eight bytes: its tile, the facings it answers
   to (as g_facing_bits), the story flag that retires it and the event it
   runs. */
typedef struct {
    u_char  x, y;
    u_char  facings;
    u_char  pad3;
    u_short flag;   /* 0x4 */
    u_char  event;  /* 0x6 */
    u_char  pad7;
} FloorSpot;

/* Per facing, its bit in a spot's facings. */
extern u_long g_facing_bits[];

int FieldSpotEvent(int mode);
int FieldRollEncounter(void);

/* The floor's flag word: FLOOR_QUIET, and below it the story flag that
   turns its encounters on. The next word is the story flag choosing
   between its two encounter sets. A set is 31 encounter ids (0xFFFF
   empty) and a rare one, rolled on tiles with ENC_RARE. */
typedef struct {
    u_short glyphs[10];
    u_short flags;       /* 0x14 */
    u_short enc_set;     /* 0x16 */
    u_short enc[2][32];  /* 0x18 */
} FloorInfo;
#define FLOOR_INFO ((FloorInfo *)g_floor_info)
#define FLOOR_ENC_FLAG  (FLOOR_FLAGS & 0x7FFF)
#define ENC_NONE 0xFFFF

/* Each tile of g_floor_objs: its encounter rate (1-3, 0 none) in the low
   bits, ENC_RARE, and ENC_NO_COMMON for a tile that only has the rare one. */
#define ENC_RATE      0x3
#define ENC_RARE      0x80
#define ENC_NO_COMMON 0x40

/* One in g_enc_rate_div[rate] steps rolls an encounter. */
extern u_short g_enc_rate_div[];

/* The base of the hero's surprise roll, per moon phase group, and each
   phase's group. */
extern u_short g_enc_moon_base[];
extern u_char  g_moon_group[];

/* The encounter handed to the battle: its id, the map it came from, and
   the hero's surprise roll - 0 not made, 1 held, 2 failed (the screen
   flashes). In the save area and reached by address. */
#define g_enc_id    (*(u_short *)0x801F5350)
#define g_enc_map   (*(u_char *)0x801F5354)
#define g_enc_surprise (*(u_char *)0x801F5355)
int func_80073A64(int event);

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
int  FieldUpdate(int noclip);
void func_80069A7C(void);
void func_80069EB4(void);
/* Swaps the playing tune pair (handles 15 and 16) between the floor's own
   (sequences 12, 13) and a zone's (15, 16) as the party steps on or off a
   TILE_ZONE_TUNE tile; the new tune starts unless `quiet`. */
void FieldZoneTunes(int quiet);
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

/* An exit of g_floor_spots, 18 bytes: its tile, the scene it leads to
   (g_exit_scenes by the low bits of kind), the story flag choosing between
   its two destination ids, where in the destination it lands, and the
   rectangle of the automap taking it reveals. */
typedef struct {
    u_char  x, y;
    u_char  kind;          /* 0x2 */
    u_char  pad3;
    u_short flag;          /* 0x4 */
    u_short id[2];         /* 0x6 */
    u_char  unk4;          /* 0xA */
    u_char  to_x, to_y;    /* 0xB */
    u_char  room;          /* 0xD low 3 bits the room, 0xC0 for ADV */
    u_char  seen_x, seen_y; /* 0xE */
    u_char  seen_w, seen_h; /* 0x10 */
} FloorExit;

/* Per exit kind, the scene it leads to. */
extern int g_exit_scenes[];

/* How many of the exits on the party's tile to pass over before the one
   taken; used up as they are. */
extern u_char g_exit_skip;

/* Where a scene change leads, in the save area: the map id, the tile and
   the room (the same bytes the encounter is handed in). */
#define g_dest_id   g_enc_id
#define g_dest_unk4 g_enc_map
#define g_dest_room g_enc_surprise
extern u_char g_dest_x;
extern u_char g_dest_y;

/* Per walk direction, the axis a door's halves slide along. */
extern u_short g_door_axes[];

void FieldTakeExit(void);
int  FieldUseTile(void);
int  FieldOpenDoor(void);
void FieldInitGraph(void);
void FieldSetFloor(void);
void FieldEnterFrom(void);

u_long *FieldMapTmd(u_long *tmd);
void FieldSetPrimClut(u_short *list, u_short clut);

/* The message box's cursor, text colour and upload rectangle, the palette
   of each text colour, and the window table. */
extern int     g_msg_col;
extern int     g_msg_line;
extern u_char  g_msg_color;
extern RECT    g_msg_rect;
extern u_short g_msg_cluts[];
extern u_long *g_pack_msg_tab;
extern u_long  g_win_default[];
extern u_long  g_win_kinds[];

void FieldMsgPutGlyph(u_short n);
void FieldMsgClear(void);
void FieldMsgNewLine(void);
void FieldMsgClearLine(int line);
void FieldMsgSetWindow(int n);
void FieldMsgTint(u_char *cells, u_char color, int col, int line, int count);
void FieldMsgPrintBytes(u_char *s, u_char n);
void FieldMsgPrint();      /* old-style */
int  FieldFindMember(u_char key);
void FieldMsgPrintCodes(); /* old-style */
void FieldMsgSetStyle(u_int style);

/* Set together by FieldMsgSetStyle; nothing here says more. */
extern u_char D_8009FAE0;
extern int    D_8009FAE4;
extern int    g_msg_styles[];
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

/* The ambient sequence, faded out on floors with FLOOR_QUIET set, and
   whether it is playing. */
#define g_ambient_seq (g_seq_handles[0])
extern u_char g_ambient_on;

/* The floor's flag word in its info block. */
#define FLOOR_FLAGS  (((u_short *)g_floor_info)[10])
#define FLOOR_QUIET  0x8000

/* The field's sequence data, loaded at SEQ_BASE with a table of offsets at
   its head; an offset of 0 is a sequence the floor does not have. */
#define SEQ_BASE    0x801CD000
#define SEQ_OFFSETS ((u_long *)SEQ_BASE)

/* How many of the handles the field's own sequences use, and the VABs. */
#define FIELD_SEQS 19
#define FIELD_VABS 3

/* Per sequence, the handle it is opened into. */
extern signed char g_seq_slot[];

/* Which display buffer is being drawn into. */
extern int g_draw_buf;

/* The shattering battle transition: the screen cut into FX_TILES tiles,
   each a POLY_FT4 with a position and a rotation, kept in the scene's
   object, coordinate and rotation space; g_fx_shift turns them all. */
#define FX_COLS  10
#define FX_ROWS  8
#define FX_TILES (FX_COLS * FX_ROWS)
extern POLY_FT4 *g_fx_tiles;
extern SVECTOR  *g_fx_pos;
extern SVECTOR  *g_fx_rots;
extern SVECTOR   g_fx_shift;

/* Where the tiles are drawn from, and the top two corners of a tile. */
extern VECTOR    g_fx_depth;
extern SVECTOR   g_fx_top_left;
extern SVECTOR   g_fx_top_right;

void FieldFxBegin(void);
void FieldFxRun(int kind);
void FieldFxStep(int kind);
void FieldFxSetup(void);

/* The wavy transition's per-column stretch speed, and the frames each
   column waits before it starts to stretch; the shattering one keeps each
   tile's fall speed and delay in them. */
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

/* A cell of the lift indicator's texture, and the patterns built of them. */
typedef struct {
    int w, h, u, v;
} LiftCell;
extern LiftCell g_lift_cells[];
extern int      g_lift_patterns[][7];
extern int      g_lift_pattern_of[];

/* Per lift, the floor each button stops at, and - the second half of the
   same rows - the floor each position in the lift leaves from. */
extern u_char g_lift_stops[][12];
extern u_char g_lift_from[][12];

void FieldSetLiftDigits(int n);
void FieldBuildScene(int reload);
void FieldPlaceObject(int obj, int model, int x, int y);
void FieldRideLift(int button);
/* Fades the floor's ambient sequence in or out as the floor asks (unless
   `keep`), then stops or restarts the second floor tune by whether the
   party stands on an entry. */
void FieldSyncMusic(int keep);
void FieldNudge(int frames, int dy);

void FieldStartBattle(void);
void FieldPlayJingle(); /* old-style: callers pass two or three ints */
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
void FieldOpenSound(void);
/* Whether the sound was left running by the ADV scene the field comes back
   from: D_801F5358 clear and the exit taken into it without bit 0. Needs
   persona/main/state.h. */
extern short D_801F5358;
#define SOUND_KEPT \
    (g_state_prev == GAME_STATE_ADV && D_801F5358 == 0 && !(g_dng->exit_bits & 1))
void FieldCloseSound(void);
void FieldSetView(int keep_height);
void FieldInitLight(void);

#endif
