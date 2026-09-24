/* Persona 1 (JP) - the plain step and finish most moves are played out with.
 * BTLP only.
 *   0x800C09D0 BtlFxStep01  0x800C0E54 BtlFxFinish01
 *
 * BtlFxStep01 is the step column's entry for 138 of the 247 moves. A record
 * waits out its timer and shows itself; then, each frame, a handful of moves
 * carry their records along as well as animating them - a glide for the two
 * throws, a standing step that grows for the two rising sheets, a slide toward
 * the far side for 0x28 and a fade to black for 0x1B - and the six moves from
 * FX_STEP_TINT_FIRST tint the arena. The record that carries the head mark
 * waits for its script to end and then arms the hit: move 0x2C first opens two
 * more layers over it out of its own script table, and move 0xE0 takes hold of
 * the acting fighter's object. Every other record grows a little if it is the
 * one being aimed at, and frees itself once its script is done.
 *
 * BtlFxFinish01 is what the head record is handed once the hit is armed. It
 * lets the fight resolve the fighter the chain stands on, walks the acting
 * fighter's target mask on to the next one still standing and resolves that,
 * and when there are none left waits half a second and lets the effect sound
 * slots go.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/spellfx.h>

extern void BtlFxResolveHit(BtlObj *o);

/* The moves the step treats specially beyond the shared motion: one fades to
   black, one opens two more layers over its head, and one takes hold of the
   acting fighter. */
#define FX_01_DARK  0x1B
#define FX_01_SPLIT 0x2C
#define FX_01_GRAB  0xE0

#define FX_01_DARK_FADE 16

/* Move 0x2C's head waits this long before it shows, and its two extra layers
   come out of these entries of its script table, with this attribute. */
#define FX_01_SPLIT_WAIT  8
#define FX_01_SPLIT_FIRST 2
#define FX_01_SPLIT_END   4
#define FX_01_SPLIT_ATTR  (BTL_OBJ_SHIFT_SCREEN | BTL_OBJ_NO_SHADOW)

/* The motion move 0xE0 puts the fighter's object on. */
#define FX_01_GRAB_MOTION 8

/* How much an aimed-at record grows each frame; twice as much for 0x1B. */
#define FX_01_GROW      0x80
#define FX_01_GROW_DARK 0x100

/* The hit the head arms. */
#define FX_01_HITS 9

/* The finish's phases after FX_STEP_DONE: resolve the fighter being walked
   to, step the walk, find the next one, and wait. */
#define FX_01_RESOLVE 0x81
#define FX_01_NEXT    0x82
#define FX_01_FIND    0x83
#define FX_01_WAIT    0x84

#define FX_01_WAIT_FRAMES 30

/* The effect sound slots, which the finish gives back one at a time. */
#define FX_01_SLOT_FIRST 7
#define FX_01_SLOT_LAST  12

/* 98.47%: the one instruction out of place is the load of g_btl_fx_target.
   The image reads it after the phase store and before the attribute's `or`.
   Written ahead of the phase store, the read cannot be scheduled past it -
   both are bytes, and cc1 cannot tell the two addresses apart. Written after
   it, the first scheduling pass carries the read down to the byte store into
   g_btl_shake_on, and `slot` trades a1 for a0 with the group byte. No order of
   the four statements, no type for `slot` and no place for the shake store
   puts the read between the two. Read through an array view of the byte
   (`extern u_char t[] __asm__("g_btl_fx_target")`) after the phase store, the
   read stays behind that store (99.10%), but then comes out ahead of the
   attribute and move loads rather than after them. */
#ifdef NON_MATCHING
void BtlFxStep01(BtlObj *o)
{
    BtlObj *n;
    const u_long ***scripts;
    int     i;
    int     slot;

    if (o->mark_num == FX_COPY_MARK) {
        return;
    }
    switch (o->phase) {
    case 0:
        if (o->timer != 0) {
            return;
        }
        o->attr &= ~(BTL_OBJ_HIDDEN | BTL_OBJ_STATIC);
        o->phase++;
        if (o->kind == FX_01_SPLIT) {
            o->timer = FX_01_SPLIT_WAIT;
        }
        return;
    case 1:
        if (o->timer != 0) {
            return;
        }
        switch (o->kind) {
        case FX_01_DARK:
            o->rgb_to[0] = 0;
            o->rgb_to[1] = 0;
            o->rgb_to[2] = 0;
            o->fade = FX_01_DARK_FADE;
            break;
        case FX_STEP_THROW:
        case FX_STEP_THROW2:
            if (o->steps-- > 0) {
                o->y += o->step_y;
            }
            break;
        case FX_STEP_RISE:
        case FX_STEP_RISE2:
            o->y += o->step_y;
            o->step_y += g_btl_actor_turn < BTL_PARTY ? -FX_STEP_FALL
                                                      : FX_STEP_FALL;
            break;
        case FX_STEP_SLIDE:
            o->y += g_btl_actor_turn < BTL_PARTY ? -FX_STEP_SLIDE_BY
                                                 : FX_STEP_SLIDE_BY;
            break;
        }
        if ((u_short)(o->kind - FX_STEP_TINT_FIRST) < FX_STEP_TINT_MOVES) {
            g_btl_arena_rgb[1] = FX_STEP_TINT_G;
            g_btl_arena_rgb[2] = FX_STEP_TINT_B;
            g_btl_arena_rgb[0] = FX_STEP_TINT_R;
            g_btl_arena_fade = FX_STEP_TINT_FADE;
        }
        if (o->mark_num == 0 || o->mark_num == FX_MARK_HEAD) {
            if (o->kind == FX_01_SPLIT) {
                g_btl_fx_def.attr = FX_01_SPLIT_ATTR;
                i = FX_01_SPLIT_FIRST;
                scripts = &g_btl_fx_def.scripts;
                do {
                    *scripts = (const u_long **)o->scripts[i];
                    n = BtlObjAlloc((BtlObjDef *)(scripts - 1), FX_OBJ_GROUP,
                                    NULL, FX_OBJ_DRAW, 0, &o->x, FX_OBJ_CD,
                                    FX_OBJ_CE);
                    n->kind = o->kind;
                    n->scripts = o->scripts;
                    n->mark_num = FX_COPY_MARK;
                    i++;
                } while (i < FX_01_SPLIT_END);
            }
            if (o->kind != FX_01_SPLIT && (o->attr & BTL_OBJ_ANIMATING) != 0) {
                return;
            }
            if (o->kind == FX_01_GRAB) {
                o->actor->obj->attr |= BTL_OBJ_CARRIED;
                o->actor->resume_motion = o->actor->obj->motion;
                o->actor->resume_phase = o->actor->obj->phase;
                o->actor->hit_amount = 0;
                o->actor->obj->motion = FX_01_GRAB_MOTION;
                o->actor->obj->phase = 0;
            }
            slot = g_btl_fx_target;
            o->phase = FX_STEP_DONE;
            o->attr |= BTL_OBJ_HIDDEN;
            o->children = (u_char)g_btl_spell_fx[g_btl_fx_move].group;
            g_btl_hits_left = FX_01_HITS;
            g_btl_hit_walk = -1;
            g_btl_shake_on = 0;
            g_btl_hit_slot = slot;
            g_btl_hit_mask = 1;
            g_btl_actors[g_btl_actor_turn].targets &= ~(1 << slot);
            return;
        }
        if ((o->attr & BTL_OBJ_PICKED) != 0) {
            o->scale_x += g_btl_fx_move == FX_01_DARK ? FX_01_GROW_DARK
                                                      : FX_01_GROW;
            o->scale_y += g_btl_fx_move == FX_01_DARK ? FX_01_GROW_DARK
                                                      : FX_01_GROW;
        }
        if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
            return;
        }
        BtlObjFree(o);
        return;
    default:
        g_btl_spell_fx[g_btl_fx_move].finish(o);
        return;
    }
}
#else
INCLUDE_ASM("btlp/nonmatchings/fxstep01", BtlFxStep01);
#endif

void BtlFxFinish01(BtlObj *o)
{
    int slot;

    switch (o->phase) {
    case FX_STEP_DONE:
        BtlFxReopenVoices();
        o->actor->hit_amount = 0;
        o->attr &= ~BTL_OBJ_CARRIED;
        o->phase++;
        return;
    case FX_01_RESOLVE:
        BtlFxResolveHit(o);
        return;
    case FX_01_NEXT:
        if (o->timer != 0) {
            return;
        }
        g_btl_hits_left--;
        /* An if, not a ternary: the image preloads the 1 in the branch's
           delay slot and stores once behind both arms. */
        if (g_btl_hit_walk < 0) {
            g_btl_hit_mask = 1;
        } else {
            g_btl_hit_mask <<= 1;
        }
        g_btl_hit_walk++;
        o->phase++;
        return;
    case FX_01_FIND:
        for (; g_btl_hit_walk < BTL_ACTORS;
             g_btl_hit_walk++, g_btl_hit_mask <<= 1) {
            if ((g_btl_actors[g_btl_actor_turn].targets
                 & g_btl_hit_mask) != 0
                && g_btl_actors[g_btl_hit_walk].c.key != 0
                && (signed char)g_btl_actors[g_btl_hit_walk].c.status
                       != BTL_STATUS_DOWN
                && (g_btl_actors[g_btl_hit_walk].flags & BTL_ACTOR_OUT) == 0) {
                g_btl_hit_slot = g_btl_hit_walk;
                break;
            }
        }
        if (g_btl_hit_walk >= BTL_ACTORS) {
            o->timer = FX_01_WAIT_FRAMES;
            o->phase++;
        } else {
            o->phase = FX_01_RESOLVE;
        }
        return;
    case FX_01_WAIT:
        if (o->timer != 0) {
            return;
        }
        slot = FX_01_SLOT_FIRST;
        do {
            BtlSoundClose(slot);
            slot++;
        } while (slot < FX_01_SLOT_LAST);
        o->motion = 0;
        o->phase = 0;
        return;
    }
}
