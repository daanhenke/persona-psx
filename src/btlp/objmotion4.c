/* Persona 1 (JP) - three more of the motions a display record can be put
 * through.  BTLP only.
 *   0x800ACBD4 BtlObjMotion03
 *   0x800ACDA0 BtlObjMotion04  0x800ACF64 BtlObjMotion05
 *   0x800AD060 BtlObjMotion02
 *
 * 02 is the flip a party marker arrives and leaves with, on rot.vy and rot.vz
 * at the same pace as 05's: at the edge the record changes face and takes the
 * script for it, and once round it settles - the front record hands the
 * marker its standing script and gives itself up, the piece behind rejoins
 * the marker and moves back into line.
 *
 * Entries 3, 4 and 5 of g_btl_obj_motion, named for the motion they answer to
 * the way the rest of the table is (see objmotion8.c).
 *
 * 03 is the grow a board is put up with: the width first and then the height,
 * each gaining a third of itself a frame until it passes full size. Once its
 * timer is out the record is shown, unless it is flagged to stay hidden while
 * it grows; once the width is full it is shown unless flagged to stay hidden
 * after, and a record flagged to be relit is walked to its blue at once and
 * has its shadow turned off. The height reaching full size ends the motion.
 * With g_btl_fast_anim raised the record is put at full size at once.
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

/* The grow: full size, its two phases, the two flags that keep the record
   hidden while it grows and once it has, and the draw a relit record keeps
   its shadow under. */
#define GROW_FULL        0x1000
#define GROW_WIDTH       0
#define GROW_HEIGHT      1
#define GROW_HIDE_DURING 0x400
#define GROW_HIDE_AFTER  0x800
#define GROW_SHADOW_DRAW 9

void BtlObjMotion03(BtlObj *obj)
{
    if (g_btl_fast_anim != 0) {
        obj->attr &= ~BTL_OBJ_HIDDEN;
        obj->scale_x = GROW_FULL;
        obj->scale_y = GROW_FULL;
        obj->motion = 0;
        obj->phase = 0;
        obj->draw &= ~OBJ_DRAW_XFORM;
        if (obj->attr & OBJ_RELIGHT) {
            obj->rgb_to[0] = 0;
            obj->rgb_to[1] = 0;
            obj->rgb_to[2] = OBJ_LIT;
            obj->fade = OBJ_SNAP;
            obj->attr |= BTL_OBJ_NO_SHADOW;
        }
        return;
    }

    obj->draw |= OBJ_DRAW_XFORM;
    switch (obj->phase) {
    case GROW_WIDTH:
        if (obj->timer != 0) {
            break;
        }
        if ((obj->attr & GROW_HIDE_DURING) == 0) {
            obj->attr &= ~BTL_OBJ_HIDDEN;
        }
        obj->scale_x += obj->scale_x / 3;
        if (obj->scale_x > GROW_FULL) {
            obj->scale_x = GROW_FULL;
            if ((obj->attr & GROW_HIDE_AFTER) == 0) {
                obj->attr &= ~BTL_OBJ_HIDDEN;
            }
            if (obj->attr & OBJ_RELIGHT) {
                obj->rgb_to[2] = OBJ_LIT;
                obj->fade = OBJ_SNAP;
                obj->rgb_to[0] = 0;
                obj->rgb_to[1] = 0;
                if (obj->draw != GROW_SHADOW_DRAW) {
                    obj->attr |= BTL_OBJ_NO_SHADOW;
                }
            }
            obj->phase++;
        }
        break;

    case GROW_HEIGHT:
        obj->scale_y += obj->scale_y / 3;
        if (obj->scale_y > GROW_FULL) {
            obj->scale_y = GROW_FULL;
            obj->motion = 0;
            obj->phase = 0;
            obj->draw &= ~OBJ_DRAW_XFORM;
        }
        break;
    }
}

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

/* A shown marker's two records, as BtlShowMarker builds them: the front one
   carrying MARKER_FACE, and the piece behind it carrying MARKER_PIECE and its
   kind in scale_to. The kind scripts come before the slot scripts in
   g_btl_marker_scripts, and the marker stands MARKER_LIFT to the side while
   the flip runs. */
#define MARKER_FACE  0x100
#define MARKER_PIECE 0x400
#define MARKER_KINDS 7
#define MARKER_LIFT  0x210000

extern BtlSeqStep  D_800DA2B8[];
extern BtlSeqStep  D_800DA2C8[];
extern BtlSeqStep  D_800DA2E8[];
extern BtlSeqStep  D_800DA2F8[];
extern BtlSeqStep *g_btl_marker_scripts[];
extern BtlSeqStep *g_btl_marker_back_scripts[];
extern BtlObj     *g_btl_marker_shown[];

void BtlObjMotion02(BtlObj *obj)
{
    switch (obj->phase) {
    case FLIP_OUT:
        obj->draw |= OBJ_DRAW_XFORM;
        if ((obj->rot.vy & (OBJ_TURN - 1)) == FLIP_EDGE) {
            obj->attr ^= MARKER_FACE;
            obj->rot.vy = FLIP_FAR;
            obj->rot.vz = FLIP_FAR;
            /* The front record's arm is written first on both faces: the
               image falls through to it and branches to the piece's. */
            if (obj->attr & MARKER_FACE) {
                if ((obj->attr & MARKER_PIECE) == 0) {
                    BtlObjSetScript(obj, D_800DA2F8);
                } else {
                    BtlObjSetScript(obj, g_btl_marker_scripts[obj->scale_to]);
                }
            } else if ((obj->attr & MARKER_PIECE) == 0) {
                BtlObjSetScript(obj, D_800DA2E8);
            } else {
                BtlObjSetScript(obj,
                                g_btl_marker_scripts[MARKER_KINDS + obj->mark_num]);
                BtlPlaceMemberMarkers(obj->mark_num, 1);
            }
            obj->phase++;
        } else {
            obj->rot.vy += FLIP_STEP;
            obj->rot.vz += FLIP_STEP;
        }
        break;

    case FLIP_BACK:
        if ((obj->rot.vy & (OBJ_TURN - 1)) == 0) {
            obj->draw &= ~OBJ_DRAW_XFORM;
            if ((obj->attr & MARKER_PIECE) == 0) {
                BtlObjSetScript(g_btl_marker_obj[obj->mark_num],
                                (obj->attr & MARKER_FACE) ? D_800DA2C8
                                                          : D_800DA2B8);
                g_btl_marker_shown[obj->mark_num] = NULL;
                BtlObjFree(obj);
            } else {
                BtlObjLast(g_btl_marker_obj[obj->mark_num])->attached = obj;
                BtlObjSetScript(obj,
                                (obj->attr & MARKER_FACE)
                                    ? g_btl_marker_back_scripts[obj->scale_to]
                                    : g_btl_marker_scripts[MARKER_KINDS
                                                           + obj->mark_num]);
                BtlPlaceMemberMarkers(obj->mark_num, 0);
                obj->motion = 0;
                obj->phase = 0;
                obj->x += MARKER_LIFT;
            }
        } else {
            obj->rot.vy += FLIP_STEP;
            obj->rot.vz += FLIP_STEP;
        }
        break;
    }
}
