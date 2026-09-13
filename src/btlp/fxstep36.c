/* Persona 1 (JP) - the move drawn as a turning disc and the pieces going
 * round it.  BTLP only.
 *   0x800BA2AC BtlFxStart36  0x800BA4FC BtlFxStep36
 *
 * BtlFxStart36 builds the set over the far side of the field from whoever is
 * acting: eight pieces spaced evenly round the circle, each chained behind the
 * last and started dark, and then the disc on top of them, drawn at one and a
 * half times its size on the camera's own angles and held FX_36_HOLD frames.
 *
 * A step handler out of g_btl_spell_fx, called once a frame on each record of
 * the set. Which record it is is told by the mark: the head is the disc, and
 * it turns on its own axis a step at a time; a copy is one of the pieces going
 * round, and it is walked four units further round the wave tables each frame
 * and stood at that point of a circle ninety-two units across, over the far
 * side of the field.
 *
 * Whichever it is, once the record has stood for as long as the start handler
 * gave it, it is told to walk back to black, and the head arms the hit and the
 * copies free themselves once it has.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* How far the disc turns each frame, and how far round the circle a piece
   moves. The wave tables run 0x200 to the turn. */
#define FX_36_SPIN 0x40
#define FX_36_STEP 4
#define FX_36_TURN 0x200

/* How far out the pieces stand, and how far past the middle of the field the
   circle they go round is. */
#define FX_36_RADIUS 92
#define FX_36_Y      0x500000

/* How long the set stands, how fast it drains, and the phase the head is left
   on once the hit is armed. */
#define FX_36_HOLD 0x40
#define FX_36_FADE 2
#define FX_36_DONE 0x80

/* The pieces round the circle, how far apart they start on the wave tables,
   the colour they are walked to, and the disc's attributes and size. */
#define FX_36_PIECES     8
#define FX_36_SPACING    0x40
#define FX_36_LIT        0x80
#define FX_36_DISC_ATTR  (BTL_OBJ_ATTR_4000 | 0x4 | BTL_OBJ_NO_SHADOW)
#define FX_36_DISC_SCALE 0x1800

/* 75.20%. Every constant the two BtlObjAlloc calls take - the effect
   template, the position's address, the group, 0x1D and 0xE - and the fade's
   2 are kept in saved registers here and shared by both calls; the image
   loads each afresh where it is used and holds only the 0x80 in a saved
   register. It is not loop invariant motion: the loop written with a goto,
   as a for and as a while all keep the sharing, and so does taking the 0x80
   into a local. The pieces' fields are also stored through `after` in the
   image once it has been advanced. */
#ifdef NON_MATCHING
BtlObj *BtlFxStart36(void)
{
    BtlObj *o;
    BtlObj *after;
    long    pos[3];
    int     i;

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[1];
    i = FX_36_PIECES - 1;
    after = NULL;
    do {
        pos[0] = g_btl_wave_sin[i * FX_36_SPACING] * FX_36_RADIUS;
        pos[1] = g_btl_wave_cos[i * FX_36_SPACING] * FX_36_RADIUS
                 + (g_btl_actor_turn < BTL_PARTY ? -FX_36_Y : FX_36_Y);
        pos[2] = 0;
        o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, after, FX_OBJ_DRAW, 0, pos,
                        FX_OBJ_CD, FX_OBJ_CE);
        o->attached = after;
        after = o;
        o->attr = BTL_OBJ_NO_SHADOW;
        o->mark_num = FX_COPY_MARK;
        o->angle = i * FX_36_SPACING;
        o->fade = FX_36_FADE;
        o->rgb[0] = 0;
        o->rgb[1] = 0;
        o->rgb[2] = 0;
        o->rgb_to[0] = FX_36_LIT;
        o->rgb_to[1] = FX_36_LIT;
        o->rgb_to[2] = FX_36_LIT;
        i--;
    } while (i >= 0);

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    pos[0] = 0;
    pos[1] = g_btl_actor_turn < BTL_PARTY ? -FX_36_Y : FX_36_Y;
    pos[2] = 0;
    o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, after, FX_OBJ_DRAW, 0, pos,
                    FX_OBJ_CD, FX_OBJ_CE);
    o->attr = FX_36_DISC_ATTR;
    o->scale_x = FX_36_DISC_SCALE;
    o->scale_y = FX_36_DISC_SCALE;
    o->mark_num = FX_MARK_HEAD;
    o->fade = FX_36_FADE;
    o->attached = after;
    o->rgb[0] = 0;
    o->rgb[1] = 0;
    o->rgb[2] = 0;
    o->rgb_to[0] = FX_36_LIT;
    o->rgb_to[1] = FX_36_LIT;
    o->rgb_to[2] = FX_36_LIT;
    o->rot.vx = g_btl_cam_rot.vx;
    o->rot.vy = g_btl_cam_rot.vy;
    o->rot.vz = g_btl_intro_dist;
    BtlObjSetTimer(o, FX_36_HOLD);
    return o;
}
#else
INCLUDE_ASM("btlp/nonmatchings/fxstep36", BtlFxStart36);
#endif

void BtlFxStep36(BtlObj *o)
{
    switch (o->phase) {
    case 0:
        switch (o->mark_num) {
        case FX_MARK_HEAD:
            o->rot.vz -= FX_36_SPIN;
            break;
        case FX_COPY_MARK:
            o->angle = (o->angle + FX_36_STEP) & (FX_36_TURN - 1);
            o->x = g_btl_wave_sin[o->angle] * FX_36_RADIUS;
            /* The side the circle stands on is picked inside the y statement,
               not in front of it: written as an if of its own the whole
               cosine walk moves behind the branch. */
            o->y = g_btl_wave_cos[o->angle] * FX_36_RADIUS
                   + (g_btl_actor_turn < BTL_PARTY ? -FX_36_Y : FX_36_Y);
            break;
        default:
            break;
        }
        if (o->timer != 0) {
            break;
        }
        o->timer = FX_36_HOLD;
        o->fade = FX_36_FADE;
        o->rgb_to[0] = 0;
        o->rgb_to[1] = 0;
        o->rgb_to[2] = 0;
        o->phase++;
        break;
    case 1:
        if (o->timer != 0) {
            break;
        }
        switch (o->mark_num) {
        case FX_MARK_HEAD:
            o->phase = FX_36_DONE;
            BtlArmHitChain();
            o->children = (u_char)g_btl_spell_fx[g_btl_fx_move].group;
            break;
        case FX_COPY_MARK:
            BtlObjFree(o);
            break;
        default:
            break;
        }
        break;
    default:
        BtlFxFinish01(o);
        break;
    }
}
