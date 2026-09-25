/* Persona 1 (JP) - stepping forward into a door.  DNG only.
 *   0x8006A53C FieldStepDoor
 *
 * Before a step forward the field checks the tile the party stands on: a
 * spot event there runs, and a door on it opens - one of nine kinds that
 * slide or swing their halves six frames, the lift's doors (which open onto
 * the lift panel and close again after the ride) and the doors that open
 * onto an event and close behind it. Returns 3 when an event or the lift
 * ran, 2 when a door opened for the step to go through, 0 otherwise.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libsnd.h>
#include <persona/common/automap.h>
#include <persona/dng/field.h>

/* Per facing, which way a door's halves slide along its axis. */
extern short g_door_signs[];

/* A lift door's facing bits (flags 0x1E0 >> 5) turned into the facing. */
extern short g_bits_facing[];

/* The first scene object of the lift's cell, for the frame's drawing. */
extern int g_lift_objs;

/* A tile with this bit is a door of kind flags & TILE_KIND. */
#define TILE_DOORWAY 0x400
/* The lift's doors and the event doors, with their other bits clear. */
#define TILE_LIFT_MASK  0xDC1F
#define TILE_LIFT       2
#define TILE_EVENT_MASK 0xFE1F
#define TILE_EVENT_DOOR 3

#define STEP_THROUGH 2
#define STEP_EVENT   3

#define WINDOW 11

/* The first scene object of the party's cell of the view window. */
#define CELL ((u_char)(g_dng->pos[POS_Y] % WINDOW) * WINDOW + (u_char)(g_dng->pos[POS_X] % WINDOW))

/* Whether the door opens towards the way the step goes. */
#define FACES(flags) (((flags) >> (g_dng->walk_dir + 5)) & 1)

#define SPOT(i) (((FloorSpot *)g_floor_events)[i])

/* A door's slide sign, read as a plain short: the stores between the two
   reads may alias it. */
#define SIGN(f) (*(short *)((u_char *)g_door_signs + (f) * 2))

#define OBJ_T(o, axis) (g_scene->coords[o].coord.t[axis])
#define OBJ_SET(o)     CoordSetRot(&g_scene->rots[o], &g_scene->coords[o])

/* Sets the automap bit of tile (x, y) of the party's room. */
static inline void MarkSeen(int area, int room, u_int x, int y)
{
    *(g_map_seen + (g_map_base[area] + room) * MAP_BYTES + y * MAP_ROW_BYTES + (x >> 3)) |= 0x80 >> (x & 7);
}

/* 98.4%: every door kind, the event door and the lift's control flow are
   the image's. Two things differ, both in the lift block:
   - where sched2 puts FieldPlayJingle's 0x18 argument. Here it lands in
     the cell's multu gap, which pushes local-alloc's temps off a0/a1 (the
     image keeps both halves' door setup in v0/v1/a0/a1).
   - the saved no_enc spill: the image stores a word and reloads a byte,
     which is a u_char set through a paradoxical subreg. Every spelling of
     keep tried here gives sw/lw (int) or sb/lbu (u_char).
   SIGN() reads the sign as a plain short so the dy read stays after the dx
   store; the event door's two tests share one return so loop.c parks the
   spot search's found block ahead of it. */
#ifdef NON_MATCHING
int FieldStepDoor(void)
{
    int  flags;
    int  i;
    int  b;
    int  facing;
    int  sign;
    long t0, t1;
    int  keep;
    int  hold;
    int  r;
    int  n;

    if (g_noclip) {
        return 0;
    }
    g_exit_skip = 0;
    if (FieldSpotEvent(0)) {
        return STEP_EVENT;
    }
    flags = g_tile_defs[g_floor_grid[g_dng->pos[POS_Y]][g_dng->pos[POS_X]]].flags;
    if (flags & TILE_DOORWAY) {
        switch (flags & TILE_KIND) {
        case 0:
            if (!FACES(flags)) {
                break;
            }
            b = CELL * 8 + 3;
            FieldPauseBgm();
            FieldPlayJingle(0x13, 1);
            facing = g_dng->facing;
            for (i = 0; i < 6; i++) {
                OBJ_T(b, g_door_axes[facing]) += g_door_signs[facing] * 20;
                OBJ_SET(b);
                func_80065978();
            }
            return STEP_THROUGH;
        case 1:
            if (!FACES(flags)) {
                break;
            }
            b = CELL * 8;
            FieldPauseBgm();
            FieldPlayJingle(0x12, 1);
            for (i = 0; i < 6; i++) {
                g_scene->rots[b + 2].vy -= 0xAA;
                g_scene->rots[b + 3].vy += 0xAA;
                OBJ_SET(b + 2);
                OBJ_SET(b + 3);
                func_80065978();
            }
            return STEP_THROUGH;
        case 2:
            if (!FACES(flags)) {
                break;
            }
            b = CELL * 8;
            FieldPauseBgm();
            FieldPlayJingle(0x14, 1);
            for (i = 0; i < 6; i++) {
                g_scene->rots[b + 2].vy -= 0xAA;
                g_scene->rots[b + 3].vy += 0xAA;
                OBJ_SET(b + 2);
                OBJ_SET(b + 3);
                func_80065978();
            }
            return STEP_THROUGH;
        case 3:
            if (!FACES(flags)) {
                break;
            }
            b = CELL * 8;
            FieldPauseBgm();
            FieldPlayJingle(0x12, 1);
            for (i = 0; i < 6; i++) {
                g_scene->rots[b + 3].vy += 0xAA;
                OBJ_SET(b + 3);
                func_80065978();
            }
            return STEP_THROUGH;
        case 4:
            if (!FACES(flags)) {
                break;
            }
            b = CELL * 8;
            FieldPauseBgm();
            FieldPlayJingle(0x13, 1);
            if (g_dng->map == 0xF && g_dng->floor == 9) {
                for (i = 0; i < 6; i++) {
                    g_scene->rots[b + 3].vy -= 0xAA;
                    OBJ_SET(b + 3);
                    func_80065978();
                }
            } else {
                for (i = 0; i < 6; i++) {
                    g_scene->rots[b + 3].vy += 0xAA;
                    OBJ_SET(b + 3);
                    func_80065978();
                }
            }
            return STEP_THROUGH;
        case 5:
            if (!FACES(flags)) {
                return 0;
            }
            FieldPauseBgm();
            return STEP_THROUGH;
        case 6:
            if (!FACES(flags)) {
                return 0;
            }
            FieldPauseBgm();
            FieldPlayJingle(0x13, 1);
            n = CELL * 8;
            facing = g_dng->facing;
            sign = 1;
            b = n + 2;
            if (facing == 2) {
                g_exit_skip = 1;
                b = n + 3;
                sign = -1;
            }
            for (i = 0; i < 6; i++) {
                OBJ_T(b, g_door_axes[facing]) += sign * 20;
                OBJ_SET(b);
                func_80065978();
            }
            return STEP_THROUGH;
        case 7:
            if (!FACES(flags)) {
                return 0;
            }
            FieldPauseBgm();
            b = CELL * 8;
            facing = g_dng->facing;
            FieldPlayJingle((g_dng->map == 0 || g_dng->map == 0x24) && g_dng->floor == 3 ? 0x15 : 0x13, 1);
            for (i = 0; i < 6; i++) {
                OBJ_T(b + 2, g_door_axes[facing]) -= g_door_signs[facing] * 20;
                OBJ_T(b + 3, g_door_axes[facing]) += g_door_signs[facing] * 20;
                OBJ_SET(b + 2);
                OBJ_SET(b + 3);
                func_80065978();
            }
            return STEP_THROUGH;
        case 8:
            if (!FACES(flags)) {
                return 0;
            }
            FieldPauseBgm();
            b = CELL * 8 + 3;
            FieldPlayJingle(0x14, 1);
            for (i = 0; i < 9; i++) {
                OBJ_T(b, 1) -= 30;
                OBJ_SET(b);
                func_80065978();
            }
            return STEP_THROUGH;
        }
        return 0;
    }

    if ((flags & TILE_LIFT_MASK) == TILE_LIFT) {
        /* The lift: its doors slide open onto the panel, and shut again
           after the ride (or when the panel closes). */
        if (!FACES(flags)) {
            return 0;
        }
        FieldPauseBgm();
        facing = g_bits_facing[(flags & 0x1E0) >> 5];
        b = CELL * 8;
        t0 = OBJ_T(b + 2, g_door_axes[facing]);
        t1 = OBJ_T(b + 3, g_door_axes[facing]);
        g_lift_objs = b;
        g_dng->door_obj[1] = b + 3;
        g_dng->door_obj[0] = b + 2;
        g_door_dx = SIGN(facing) * -15;
        g_door_dy = SIGN(facing) * 15;
        g_door_state = 1;
        g_dng->door_axis[0] = g_door_axes[facing];
        g_dng->door_axis[1] = g_door_axes[facing];
        g_door_frames = 10;
        FieldPlayJingle(0x18, 1);
        FieldLiftPanelOpen();
        keep = g_dng->no_enc;
        g_dng->no_enc = 1;
        g_scene->pad_new = 0;
        g_scene->pad_held = PAD_UP;
        FieldUpdate(1);
        func_80065978();
        MarkSeen(g_dng->area, g_dng->room, g_dng->pos[POS_X], g_dng->pos[POS_Y]);
        OBJ_T(b + 2, g_door_axes[facing]) = t0;
        OBJ_T(b + 3, g_door_axes[facing]) = t1;
        OBJ_SET(b + 2);
        OBJ_SET(b + 3);
        if (g_floor_tune_a != -1) {
            SsSeqPause(g_floor_tune_a);
        }
        if (g_floor_tune_b != -1) {
            SsSeqPause(g_floor_tune_b);
        }
        D_800A02E0 = 1;
        g_scene->pad_new = 0;
        g_scene->pad_held = PAD_LEFT;
        FieldUpdate(0);
        func_80065978();
        g_scene->pad_held = PAD_LEFT;
        FieldUpdate(0);
        func_80065978();
        hold = g_field_hold;
        g_field_hold = 0x81;
        FieldLiftBoxes();
        while ((r = FieldLiftPanelStep()) == 0) {
            func_80065978();
        }
        if (r != -1) {
            FieldRideLift(r - 1);
        }
        FieldLiftBoxesOff();
        g_field_hold = hold;
        t0 = OBJ_T(b + 2, g_door_axes[facing]);
        t1 = OBJ_T(b + 3, g_door_axes[facing]);
        g_dng->door_obj[0] = b + 2;
        g_dng->door_obj[1] = b + 3;
        g_door_dx = SIGN(facing) * -15;
        g_door_dy = SIGN(facing) * 15;
        g_door_state = 1;
        g_dng->door_axis[0] = g_door_axes[facing];
        g_dng->door_axis[1] = g_door_axes[facing];
        g_door_frames = 10;
        FieldPlayJingle(0x18, 1);
        if (g_dng->map != 9) {
            if (g_floor_tune_a != -1 && g_map_music[g_dng->map][0] != 2) {
                SsPlayBack(g_floor_tune_a, 0, 0);
            }
            if (g_floor_tune_b != -1 && g_map_music[g_dng->map][1] != 2) {
                SsPlayBack(g_floor_tune_b, 0, 0);
            }
        }
        D_800A02E0 = 0;
        g_scene->pad_new = 0;
        g_scene->pad_held = PAD_UP;
        FieldUpdate(0);
        func_80065978();
        OBJ_T(b + 2, g_door_axes[facing]) = t0;
        OBJ_T(b + 3, g_door_axes[facing]) = t1;
        OBJ_SET(b + 2);
        OBJ_SET(b + 3);
        g_dng->no_enc = keep;
        return STEP_EVENT;
    }

    /* An event door: it opens, the party bumps up to it, the tile's event
       runs and the door shuts again. */
    if ((flags & TILE_EVENT_MASK) == TILE_EVENT_DOOR && FACES(flags)) {
    FieldPauseBgm();
    b = CELL * 8;
    FieldPlayJingle(0x13, 1);
    facing = g_dng->facing;
    for (i = 0; i < 6; i++) {
        OBJ_T(b + 2, g_door_axes[facing]) -= g_door_signs[facing] * 20;
        OBJ_T(b + 3, g_door_axes[facing]) += g_door_signs[facing] * 20;
        OBJ_SET(b + 2);
        OBJ_SET(b + 3);
        func_80065978();
    }
    FieldBumpWall();
    func_80065978();
    for (i = 0; ; i++) {
        if (SPOT(i).x == g_dng->pos[POS_X] && SPOT(i).y == g_dng->pos[POS_Y]) {
            FieldPauseBgm();
            FieldRunScript(SPOT(i).event);
            break;
        }
    }
    FieldPlayJingle(0x15, 1);
    for (i = 0; i < 6; i++) {
        OBJ_T(b + 2, g_door_axes[facing]) += g_door_signs[facing] * 20;
        OBJ_T(b + 3, g_door_axes[facing]) -= g_door_signs[facing] * 20;
        OBJ_SET(b + 2);
        OBJ_SET(b + 3);
        func_80065978();
    }
    return STEP_EVENT;
    }
    return 0;
}
#else
INCLUDE_ASM("dng/nonmatchings/field/fielddoor", FieldStepDoor);
#endif
