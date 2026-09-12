/* Persona 1 (JP) - two step handlers out of the middle of the effect block.
 * BTLP only.
 *   0x800B9774 BtlFxStep12  0x800B98E4 BtlFxStep2F
 *
 * BtlFxStep12 swings its record round a point: every frame it adds six times
 * the sine and the cosine of its own angle to its position and steps the angle
 * ten of the table's five hundred and twelve. It does that as many times as
 * the record's step count allows, and the record that carries the head mark
 * arms the hit when the count runs out. The record marked 0xFF is a copy that
 * only ever uncovers itself.
 *
 * BtlFxStep2F walks the whole of the far side to a green and puts the arena
 * there with it, then carries its own record down the screen. Every fifth
 * whole unit it passes it leaves two more records behind, built from the third
 * and fourth of its own script tables, so the move draws a trail. It stops
 * when the record has drifted a hundred and sixty units from the middle.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* The mark a copy carries - it is stepped, but does nothing of its own. */
#define FX_COPY_MARK 0xFF

/* What a step handler leaves behind once the hit is armed. */
#define FX_STEP_DONE 0x80

/* How far move 0x12's record is carried each frame, and how far round the
   tables it steps. */
#define FX_12_SWING 6
#define FX_12_SPIN  10
#define FX_12_HITS  3

/* The colour move 0x2F walks the far side and the arena to, and how fast. */
#define FX_2F_R 0
#define FX_2F_G 0xFF
#define FX_2F_B 0xC0
#define FX_2F_TINT_FADE 0xFF
#define FX_2F_ARENA_FADE 8

/* How far the record may drift from the middle before the move ends - the
   test is one unsigned comparison on the offset, so both directions at once. */
#define FX_2F_REACH 0x9FFFFF
#define FX_2F_SPAN  0x13FFFFE

/* How far it is carried each frame, how often it leaves a pair behind, how far
   behind the pair stands, and which of the script tables they come from. */
#define FX_2F_FALL  0xA0000
#define FX_2F_EVERY 5
#define FX_2F_BACK  0x140000
#define FX_2F_FIRST 2
#define FX_2F_LAST  4

/* What the pair is built from. */
#define FX_2F_ATTR  0x40001
#define FX_2F_HITS  3

void BtlFxStep12(BtlObj *o)
{
    switch (o->phase) {
    case 0:
        if (o->timer != 0) {
            break;
        }
        if (o->mark_num == FX_COPY_MARK) {
            o->attr &= ~(BTL_OBJ_HIDDEN | BTL_OBJ_STATIC);
            break;
        }
        o->attr &= ~(BTL_OBJ_HIDDEN | BTL_OBJ_STATIC);
        o->steps--;
        if (o->steps == 0) {
            o->phase++;
            break;
        }
        o->x += g_btl_wave_sin[o->angle] * FX_12_SWING;
        o->y += g_btl_wave_cos[o->angle] * FX_12_SWING;
        o->angle = (o->angle + FX_12_SPIN) & BTL_WAVE_MASK;
        break;
    case 1:
        if (o->mark_num == FX_MARK_HEAD) {
            o->phase = FX_STEP_DONE;
            o->children = FX_12_HITS;
            o->attr |= BTL_OBJ_HIDDEN;
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

/* 95.55%: structurally the image, one register out. The slot the far side is
   walked by is a saved register there and a temp here - there is no call in
   the loop for it to survive, so nothing in the source asks for one - and
   every other temp shifts along behind it. */
#ifdef NON_MATCHING
void BtlFxStep2F(BtlObj *o)
{
    BtlObj *copy;
    long    pos[3];
    long    y;
    int     start;
    int     slot;
    int     last;
    int     i;

    if (o->mark_num == FX_COPY_MARK) {
        return;
    }
    switch (o->phase) {
    case 0:
        if (o->timer != 0) {
            return;
        }
        if (g_btl_actor_turn >= BTL_PARTY) {
            start = 0;
            last  = BTL_PARTY;
        } else {
            start = BTL_PARTY;
            last  = BTL_ACTORS;
        }
        slot = start;
        for (; slot < last; slot++) {
            if (g_btl_actors[slot].c.key == 0) {
                continue;
            }
            if ((signed char)g_btl_actors[slot].c.status == BTL_STATUS_DOWN) {
                continue;
            }
            if ((g_btl_actors[slot].flags & BTL_ACTOR_OUT) != 0) {
                continue;
            }
            g_btl_actors[slot].obj->rgb_to[0] = FX_2F_R;
            g_btl_actors[slot].obj->rgb_to[1] = FX_2F_G;
            g_btl_actors[slot].obj->rgb_to[2] = FX_2F_B;
            g_btl_actors[slot].obj->fade = FX_2F_TINT_FADE;
        }
        g_btl_arena_rgb[1] = FX_2F_G;
        g_btl_arena_rgb[2] = FX_2F_B;
        g_btl_arena_rgb[0] = FX_2F_R;
        g_btl_arena_fade = FX_2F_ARENA_FADE;
        o->attr &= ~(BTL_OBJ_HIDDEN | BTL_OBJ_STATIC);
        if ((u_int)(o->y + FX_2F_REACH) > FX_2F_SPAN) {
            o->phase++;
            return;
        }
        y = o->y + ((g_btl_actor_turn < BTL_PARTY) ? -FX_2F_FALL : FX_2F_FALL);
        o->y = y;
        if ((y >> 16) / FX_2F_EVERY * FX_2F_EVERY != (y >> 16)) {
            return;
        }
        g_btl_fx_def.attr = FX_2F_ATTR;
        for (i = FX_2F_FIRST; i < FX_2F_LAST; i++) {
            g_btl_fx_def.scripts = (const u_long **)o->scripts[i];
            pos[0] = o->x;
            pos[1] = o->y - FX_2F_BACK;
            pos[2] = o->z;
            copy = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, 0, FX_OBJ_DRAW, 0,
                               &o->x, FX_OBJ_CD, FX_OBJ_CE);
            copy->kind = o->kind;
            copy->mark_num = FX_COPY_MARK;
            copy->scripts = o->scripts;
        }
        return;
    case 1:
        if (o->mark_num == 0) {
            o->attr |= BTL_OBJ_HIDDEN;
            BtlArmHitChain();
            o->phase = FX_STEP_DONE;
            o->children = FX_2F_HITS;
        } else {
            BtlObjFree(o);
        }
        return;
    default:
        BtlFinishMoveFx(o);
        return;
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/fxstep12", BtlFxStep2F);
#endif
