/* Persona 1 (JP) - the ring of eight that opens out and closes again, and the
 * move that lights the arena for it.  BTLP only.
 *   0x800B8ECC BtlFxStart2A  0x800B8FF4 BtlFxStep2A  0x800B921C BtlFxStart2B
 *
 * BtlFxStart2A stands eight records round a circle - an eighth of a turn
 * apart on the sine and cosine tables, and each turned an eighth further than
 * the last so the ring faces outward - with no scale at all to begin with. The
 * page they are drawn from is put into subtractive blending as each is made.
 *
 * BtlFxStep2A walks each record round the circle eight steps of the table a
 * frame and works its position out afresh every time, so the ring turns as
 * well as opening. The scale is what the phases are about: the first grows it
 * by an eighth of itself plus a little until it is whole, the second shrinks
 * it the same way until it is nearly gone, and the third arms the hit on the
 * one record that carries the head mark and frees the other seven.
 *
 * BtlFxStart2B is a different move: it throws the arena white and puts one
 * ordinary record on the fighter whose turn order the acting one holds.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* How many records the ring is made of, and how far apart they stand on the
   wave tables and in their own rotation. */
#define FX_2A_COUNT (BTL_WAVE_TURN / 8)
#define FX_2A_RING  8
#define FX_2A_TURN  0x1000
#define FX_2A_STEP  (FX_2A_TURN / FX_2A_RING)

/* The wave tables are 12.12 and the field is 16.16, so every reading is
   shifted six to stand the ring a hundred and sixty units out. */
#define FX_2A_OUT 6

/* What a record of the ring starts as, and how it is laid: face on, no scale
   until the first phase grows it. */
#define FX_2A_ATTR 0x4001
#define FX_2A_TILT 0xE0

/* The upload slot the ring is drawn from. */
#define FX_2A_SLOT 0x1D

/* How the scale is walked: an eighth of itself plus eight a frame on the way
   out, an eighth of itself off on the way back, and where each stops. */
#define FX_2A_GROW  8
#define FX_2A_EIGHTH 8
#define FX_2A_FULL  0x1000
#define FX_2A_GONE  0x11
#define FX_2A_HOLD  0x3C

/* How far the ring turns each frame, and how wide it stands - the tables are
   scaled by 92 and lifted eighty units clear of the field. */
#define FX_2A_SPIN  8
#define FX_2A_WIDE  92
#define FX_2A_LIFT  0x500000

/* What the last phase leaves behind. */
#define FX_2A_DONE  0x80

/* What move 0x2B throws the arena to, how fast, and how long its own record
   stands. */
#define FX_2B_FULL  0xFF
#define FX_2B_FADE  0x60
#define FX_2B_TIMER 4

BtlObj *BtlFxStart2A(void)
{
    BtlObj *o;
    BtlObj *prev;
    long    pos[3];
    int     i;

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    i    = FX_2A_RING - 1;
    prev = 0;
    do {
        pos[0] = g_btl_wave_sin[i * FX_2A_COUNT] << FX_2A_OUT;
        pos[1] = g_btl_wave_cos[i * FX_2A_COUNT] << FX_2A_OUT;
        pos[2] = 0;
        o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, prev, FX_OBJ_DRAW, 0, pos,
                        FX_OBJ_CD, FX_OBJ_CE);
        g_btl_tpage[FX_2A_SLOT] =
            (g_btl_tpage[FX_2A_SLOT] & ~BTL_TPAGE_BLEND) | BTL_TPAGE_SUBTRACT;
        o->attached = prev;
        prev = o;
        o->rot.vy  = (i + 1) * FX_2A_STEP;
        o->attr    = FX_2A_ATTR;
        o->rot.vx  = FX_2A_TILT;
        o->scale_x = 0;
        o->scale_y = 0;
        o->mark_num = i + FX_MARK_HEAD;
        o->angle    = i * FX_2A_COUNT;
    } while (--i >= 0);
    return o;
}

void BtlFxStep2A(BtlObj *o)
{
    long wide;
    long grew;

    o->angle = (o->angle + FX_2A_SPIN) & BTL_WAVE_MASK;
    o->rot.vy = o->angle * FX_2A_RING + FX_2A_STEP;
    o->x = g_btl_wave_sin[o->angle] * FX_2A_WIDE;
    wide = g_btl_wave_cos[o->angle] * FX_2A_WIDE;
    o->y = (g_btl_actor_turn < BTL_PARTY) ? wide - FX_2A_LIFT
                                          : wide + FX_2A_LIFT;

    switch (o->phase) {
    case 0:
        grew = o->scale_x + FX_2A_GROW;
        o->scale_x = grew + o->scale_x / FX_2A_EIGHTH;
        o->scale_y = o->scale_x;
        if (o->scale_x < FX_2A_FULL) {
            break;
        }
        o->scale_x = FX_2A_FULL;
        o->scale_y = FX_2A_FULL;
        o->timer = FX_2A_HOLD;
        o->phase++;
        break;
    case 1:
        if (o->timer != 0) {
            break;
        }
        o->scale_x = o->scale_x - o->scale_x / FX_2A_EIGHTH;
        if (o->scale_x >= FX_2A_GONE) {
            break;
        }
        o->attr = (o->attr | BTL_OBJ_STATIC) & ~BTL_OBJ_ANIMATING;
        o->phase++;
        break;
    case 2:
        if (o->mark_num == FX_MARK_HEAD) {
            o->phase = FX_2A_DONE;
            o->attr |= BTL_OBJ_HIDDEN;
            o->children = (u_char)g_btl_spell_fx[g_btl_fx_move].group;
            BtlArmHitChain();
        } else {
            BtlObjFree(o);
        }
        break;
    default:
        BtlFxFinish01(o);
        break;
    }
}

BtlObj *BtlFxStart2B(void)
{
    int slot;

    slot = g_btl_actors[g_btl_actor_turn].order;
    g_btl_arena_rgb[0] = FX_2B_FULL;
    g_btl_arena_rgb[1] = FX_2B_FULL;
    g_btl_arena_rgb[2] = FX_2B_FULL;
    g_btl_arena_fade = FX_2B_FADE;
    return BtlOpenFxObj2(slot, FX_2B_TIMER);
}
