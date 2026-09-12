/* Persona 1 (JP) - the move drawn as a ring of thirty-two pieces breaking
 * outward.  BTLP only.
 *   0x800BE870 BtlFxStart88
 *
 * A start handler out of g_btl_spell_fx. Thirty-two records, all standing on
 * the same point a little way toward the camera, each given the angle it
 * leaves along and the step that carries it there - the sine and the cosine of
 * that angle, twelve units apiece. The records arrive three frames apart, so
 * the ring opens rather than appearing whole.
 *
 * The angles are handed out in two halves that sweep opposite ways from the
 * same place, which is what makes the ring open symmetrically rather than
 * turning. Which place that is depends on the side acting: the party's ring
 * opens about the top of the circle and an enemy's about the bottom.
 */
#include <decomp/include_asm.h>
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* Records in the ring, and how much later than the one before each arrives. */
#define FX_88_PIECES 0x20
#define FX_88_STEP   3

/* How far toward the camera the whole ring stands. */
#define FX_88_Z (-0x18 * PLACE_FIXED)

/* The wave tables run 0x200 to the turn, so an angle is kept to nine bits and
   the halves are 0x10 records wide with four units between them. */
#define FX_88_TURN   0x200
#define FX_88_HALF   (FX_88_PIECES / 2)
#define FX_88_SPREAD 4

/* Where each half starts. The party's two meet at the top of the circle and
   the enemies' at the bottom; each half then sweeps away from its own start,
   one upward through the table and one back down it. */
#define FX_88_PARTY_UP   0x1E0
#define FX_88_PARTY_DOWN 0x20
#define FX_88_ENEMY_UP   0xE0
#define FX_88_ENEMY_DOWN 0x120

/* How fast a piece leaves, in the wave table's own units. */
#define FX_88_SPEED 12

/* Ninety-four of the ninety-five instructions, in the image's registers and
   the image's order. The one that is out is the chain link: the image fills
   the slot between the angle store and the read back with it and gcc here
   hoists it to the top of the block, and none of eight placements, three
   spellings of the mask or a block boundary round either statement moves it.
   Left guarded until the permuter finds the shape. */
#ifdef NON_MATCHING
BtlObj *BtlFxStart88(void)
{
    BtlObj *o;
    BtlObj *after;
    long    pos[3];
    int     i;
    int     angle;

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    i = FX_88_PIECES - 1;
    after = 0;
    pos[0] = 0;
    pos[1] = 0;
    pos[2] = FX_88_Z;
    do {
        o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, after, FX_OBJ_DRAW, 0,
                        pos, FX_OBJ_CD, FX_OBJ_CE);
        o->mark_num = i + FX_MARK_HEAD;
        o->timer = (FX_88_PIECES - 1 - i) * FX_88_STEP;
        o->attached = after;
        o->attr |= FX_OBJ_ATTR;
        if (g_btl_actor_turn < BTL_PARTY) {
            if (i < FX_88_HALF) {
                angle = i * FX_88_SPREAD + FX_88_PARTY_UP;
            } else {
                angle = FX_88_PARTY_DOWN - (i - FX_88_HALF) * FX_88_SPREAD;
            }
        } else {
            if (i < FX_88_HALF) {
                angle = i * FX_88_SPREAD + FX_88_ENEMY_UP;
            } else {
                angle = FX_88_ENEMY_DOWN - (i - FX_88_HALF) * FX_88_SPREAD;
            }
        }
        o->angle = angle & (FX_88_TURN - 1);
        after = o;
        /* The angle is read back off the record rather than kept, which is
           what puts the two table reads where the image has them. */
        o->step_x = g_btl_wave_sin[o->angle] * FX_88_SPEED;
        o->step_y = g_btl_wave_cos[o->angle] * FX_88_SPEED;
        i--;
    } while (i >= 0);
    return o;
}
#else
INCLUDE_ASM("btlp/nonmatchings/fxspell88", BtlFxStart88);
#endif
