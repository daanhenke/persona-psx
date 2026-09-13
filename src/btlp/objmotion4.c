/* Persona 1 (JP) - two more of the motions a display record can be put
 * through.  BTLP only.
 *   0x800ACDA0 BtlObjMotion04  0x800ACF64 BtlObjMotion05
 *
 * Entries 4 and 5 of g_btl_obj_motion, named for the motion they answer to the
 * way the rest of the table is (see objmotion8.c).
 *
 * 04 shrinks a record away in two phases, the height first and then the
 * width, each losing a third of itself a frame until it is down to scale_to.
 * Between the two the record's attached piece is hidden, and a record flagged
 * for it is walked back to full colour and has its shadow let back on. Once
 * the width is down too the motion ends: the word the record was registered
 * in is cleared, and a record flagged to be kept is hidden rather than given
 * up. With g_btl_fast_anim raised the whole shrink is skipped and the record
 * ends at once.
 *
 * 05 flips a record over on rot.vx, a sixteenth of a turn a frame. At a
 * quarter turn it jumps to three quarters - the edge-on moment - takes the
 * script waiting for the other side, and is lit, dimly if it heads no children;
 * back round at zero it stops.
 */
#include <decomp/types.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>

/* The bit of `draw` that says the record is drawn through its angles and
   scales rather than straight. */
#define OBJ_DRAW_XFORM 0x1

/* A record kept when a motion ends it, and one relit as it shrinks. */
#define OBJ_KEEP   0x80
#define OBJ_RELIGHT 0x1000

/* The shrink's two phases, the width's being the last. */
#define SHRINK_WIDTH  0
#define SHRINK_HEIGHT 1

/* The flip: a whole turn, a frame's step, where it jumps, and the two phases. */
#define OBJ_TURN     0x1000
#define FLIP_STEP    0x80
#define FLIP_EDGE    0x400
#define FLIP_FAR     0xC00
#define FLIP_OUT     0
#define FLIP_BACK    1

/* Full colour and a dim one, and a fade that gets there in a frame. */
#define OBJ_LIT  0x80
#define OBJ_DIM  0x20
#define OBJ_SNAP 0xFF

void BtlObjMotion04(BtlObj *obj)
{
    if (g_btl_fast_anim != 0) {
        if (obj->attr & OBJ_KEEP) {
            obj->attr |= BTL_OBJ_HIDDEN;
            obj->draw |= OBJ_DRAW_XFORM;
            obj->motion = 0;
            obj->phase = 0;
            obj->scale_x = obj->scale_to;
            obj->scale_y = obj->scale_to;
            return;
        }
        if (obj->ref != NULL) {
            *obj->ref = NULL;
        }
        BtlObjFree(obj);
        return;
    }

    obj->draw |= OBJ_DRAW_XFORM;
    switch (obj->phase) {
    case SHRINK_HEIGHT:
        if (obj->timer == 0) {
            obj->scale_y -= obj->scale_y / 3;
            if (obj->scale_y < obj->scale_to) {
                obj->scale_y = obj->scale_to;
                if (obj->attached != NULL) {
                    BtlObjSetAttr(obj->attached, BTL_OBJ_HIDDEN);
                }
                if (obj->attr & OBJ_RELIGHT) {
                    obj->attr &= ~BTL_OBJ_NO_SHADOW;
                    obj->rgb_to[0] = OBJ_LIT;
                    obj->rgb_to[1] = OBJ_LIT;
                    obj->rgb_to[2] = OBJ_LIT;
                    obj->fade = OBJ_SNAP;
                }
                obj->phase--;
            }
        }
        break;

    case SHRINK_WIDTH:
        obj->scale_x -= obj->scale_x / 3;
        if (obj->scale_x < obj->scale_to) {
            obj->scale_x = obj->scale_to;
            obj->motion = 0;
            obj->phase = 0;
            if (obj->ref != NULL) {
                *obj->ref = NULL;
            }
            if (obj->attr & OBJ_KEEP) {
                BtlObjSetAttr(obj, BTL_OBJ_HIDDEN);
            } else {
                BtlObjFree(obj);
            }
        }
        break;
    }
}

void BtlObjMotion05(BtlObj *obj)
{
    int v; /* the child count, the colour and the fade all pass through it:
              three variables and the colour moves off v0 */

    switch (obj->phase) {
    case FLIP_OUT:
        obj->draw |= OBJ_DRAW_XFORM;
        if ((obj->rot.vx & (OBJ_TURN - 1)) == FLIP_EDGE) {
            obj->rot.vx = FLIP_FAR;
            if (obj->next_script != NULL) {
                BtlObjSetScript(obj, (BtlSeqStep *)obj->next_script);
            }
            if ((obj->attr & BTL_OBJ_TRAIL) == 0) {
                v = obj->children;
                if (v != 0) {
                    v = OBJ_LIT;
                } else {
                    v = OBJ_DIM;
                }
                obj->rgb_to[0] = v;
                obj->rgb_to[1] = v;
                obj->rgb_to[2] = v;
                v = OBJ_SNAP;
                obj->fade = v;
            }
            obj->phase++;
        } else {
            obj->rot.vx += FLIP_STEP;
        }
        break;

    case FLIP_BACK:
        if ((obj->rot.vx & (OBJ_TURN - 1)) == 0) {
            obj->motion = 0;
            obj->phase = 0;
            obj->draw &= ~OBJ_DRAW_XFORM;
        } else {
            obj->rot.vx += FLIP_STEP;
        }
        break;
    }
}
