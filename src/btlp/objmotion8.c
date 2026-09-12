/* Persona 1 (JP) - four of the motions a display record can be put through.
 * BTLP only.
 *   0x800AD2EC BtlObjMotion08  0x800AD354 BtlObjMotion09
 *   0x800AD3C8 BtlObjMotion0A  0x800AD448 BtlObjMotion0B
 *
 * A record carries what it is doing in BtlObj.motion, and the second object
 * group's frame handler reaches the handler for it through g_btl_obj_motion.
 * Nothing calls one by hand, so a handler is named for the motion it is the
 * entry for, the way the effect table's handlers are named for their move.
 *
 * These four are two pairs, and both pairs are the pick grid arriving and
 * leaving:
 *
 *   08 and 09 turn the record. rot.vy and rot.vz are angles - 0x1000 is a whole
 *   turn - and 08 steps them forward by a sixteenth and an eighth of one until
 *   rot.vy comes back round to zero, then stops and hands the first pick cursor
 *   its own motion. 09 steps them back by the same amounts and stops three
 *   quarters of the way round, where it gives the record up and clears the word
 *   it was registered in, so whatever was watching that word sees the grid go.
 *
 *   0A and 0B scale it. 0A grows both scales by an eighth of themselves plus
 *   eight until the first passes sixteen times unity, and clamps it there; 0B
 *   shrinks them by an eighth until the first is down to unity, then gives the
 *   record up and starts the grid's tail leaving on motion 09. Both walk the
 *   record's colour to grey while they run.
 *
 * All four raise the low bit of `draw` every frame, and the two that end clear
 * it again as they stop: that bit is what the drawing pass reads to decide
 * whether the record is drawn transformed or flat, and it has to be set while
 * an angle or a scale is worth anything.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>

/* The bit of `draw` that says the record is drawn through its angles and
   scales rather than straight. */
#define OBJ_DRAW_XFORM 0x1

/* A whole turn, and how far these two get through one per frame. */
#define OBJ_TURN       0x1000
#define OBJ_SPIN_X     0x40
#define OBJ_SPIN_Y     0x80

/* Where the record turning back stops - three quarters round. */
#define OBJ_SPIN_END   0xC00

/* What the pick cursor is put on once the grid has arrived. */
#define PICK_MOTION    0xA

/* What the grid's tail is put on once a cell has shrunk away. */
#define GRID_LEAVE_MOTION 9

/* The scale the grid grows to and the one it shrinks back to - unity is 0x100,
   so sixteen times it and one. The test is written against the first value
   past the limit. */
#define OBJ_SCALE_FULL 0x1000
#define OBJ_SCALE_GONE 0x81
#define OBJ_SCALE_STEP 8

/* The grey a record under either scale is walked to. */
#define OBJ_GREY 0x80

extern BtlObj *g_btl_grid_tail;
extern BtlObj *g_btl_pick_cursors[];

void BtlObjMotion08(BtlObj *obj)
{
    obj->draw |= OBJ_DRAW_XFORM;
    if ((obj->rot.vy & (OBJ_TURN - 1)) == 0) {
        obj->draw &= ~OBJ_DRAW_XFORM;
        obj->motion = 0;
        BtlObjSetMotion(g_btl_pick_cursors[0], PICK_MOTION);
    } else {
        obj->rot.vy += OBJ_SPIN_X;
        obj->rot.vz += OBJ_SPIN_Y;
    }
}

void BtlObjMotion09(BtlObj *obj)
{
    obj->draw |= OBJ_DRAW_XFORM;
    if ((obj->rot.vy & (OBJ_TURN - 1)) == OBJ_SPIN_END) {
        BtlObjFree(obj);
        if (obj->unk58 != 0) {
            *(long *)obj->unk58 = 0;
        }
    } else {
        obj->rot.vy -= OBJ_SPIN_X;
        obj->rot.vz -= OBJ_SPIN_Y;
    }
}

void BtlObjMotion0A(BtlObj *obj)
{
    obj->rgb_to[0] = OBJ_GREY;
    obj->rgb_to[1] = OBJ_GREY;
    obj->rgb_to[2] = OBJ_GREY;
    obj->draw |= OBJ_DRAW_XFORM;
    obj->scale_x += obj->scale_x / OBJ_SCALE_STEP + OBJ_SCALE_STEP;
    obj->scale_y += obj->scale_y / OBJ_SCALE_STEP + OBJ_SCALE_STEP;
    if (obj->scale_x >= OBJ_SCALE_FULL) {
        obj->scale_x = OBJ_SCALE_FULL;
        obj->motion = 0;
        obj->draw &= ~OBJ_DRAW_XFORM;
    }
}

void BtlObjMotion0B(BtlObj *obj)
{
    obj->rgb_to[0] = OBJ_GREY;
    obj->rgb_to[1] = OBJ_GREY;
    obj->rgb_to[2] = OBJ_GREY;
    obj->draw |= OBJ_DRAW_XFORM;
    obj->scale_x -= obj->scale_x / OBJ_SCALE_STEP;
    obj->scale_y -= obj->scale_y / OBJ_SCALE_STEP;
    if (obj->scale_x < OBJ_SCALE_GONE) {
        BtlObjFree(obj);
        BtlObjSetMotion(g_btl_grid_tail, GRID_LEAVE_MOTION);
    }
}
