/* Persona 1 (JP) - the move that turns the whole arena over to the camera.
 * BTLP only.
 *   0x800BCE68 BtlFxStart6E
 *
 * A start handler out of g_btl_spell_fx, and the widest-reaching one in the
 * table: rather than standing something on a fighter it opens one record over
 * the middle of the field, faces it squarely at the camera, and then repaints
 * the scene. The record is given no scale at all to begin with - its script
 * grows it - and the three angles it is drawn through are the camera's own two
 * plus the distance the opening pulled the camera back by, which is what keeps
 * it flat against the view however the field is turned.
 *
 * The scene is put on a dark cyan and the arena told to walk toward it two
 * steps a frame, so the field drains of colour behind the effect.
 *
 * Every short the tail writes goes through the one local. That is what the
 * image has - one register threaded from the first angle to the last fade -
 * and giving each value a name of its own lets the scheduler lift the fade
 * up into the load delays instead.
 */
#include <decomp/types.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* How far above the middle of the field the record stands - 16.16, so a
   hundred whole units. */
#define FX_6E_UP 0x640000

/* What the record starts as. Not the plain effect attribute: it is neither
   hidden nor static, and it carries the bit the whitening and the round-over
   scene both set. */
#define FX_6E_ATTR (BTL_OBJ_ATTR_4000 | 0x5)

/* The colour the scene is walked to - a quarter of full on green and blue and
   nothing on red - and how much of the gap the arena closes each frame. */
#define FX_6E_LIT  0x40
#define FX_6E_FADE 2

BtlObj *BtlFxStart6E(void)
{
    BtlObj *o;
    long    pos[3];
    short   n;

    pos[0] = 0;
    pos[1] = -FX_6E_UP;
    pos[2] = 0;
    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0, pos,
                    FX_OBJ_CD, FX_OBJ_CE);
    o->attr = FX_6E_ATTR;
    o->mark_num = 0;
    o->scale_x = 0;
    o->scale_y = 0;
    n = g_btl_cam_rot.vx;
    o->rot.vx = n;
    n = g_btl_cam_rot.vy;
    o->rot.vy = n;
    n = g_btl_intro_dist;
    o->rot.vz = n;
    n = FX_6E_LIT;
    g_btl_scene_rgb[1] = n;
    g_btl_scene_rgb[2] = n;
    g_btl_scene_rgb[0] = 0;
    n = FX_6E_FADE;
    g_btl_arena_fade = n;
    return o;
}
