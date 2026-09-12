/* Persona 1 (JP) - what a summoned Persona does while it is on the field.
 * BTLP only.
 *   0x800B0EC4 BtlPersonaMotion03  0x800B1164 BtlPersonaMotion04
 *   0x800B1254 BtlPersonaMotion02
 *
 * g_btl_persona_motion is the whole of the group the summon stands in: seven
 * entries, of which only three are routines, and these are those three. 03
 * serves motion 5 as well, so the arrival is written once and reached twice.
 *
 * A summon is six records deep - one leading layer and five trailing copies
 * four frames behind each other - and all six run the motion. Only the leading
 * one is allowed to do anything outside its own record, which is what every
 * BTL_OBJ_TRAIL test here is for: the trail must not swing at the enemy, stop
 * the music, or shake the field a second time.
 *
 *   03  Arriving. The first phase stretches the record upward an eighth or a
 *       sixteenth of its height a frame - the rate is halved for as long as
 *       the field is running at half speed - until it is past 0x3000, and the
 *       second squashes it back: the width grows to unity by the same
 *       fraction and the height falls to it by a much smaller one, so it
 *       lands rather than simply stopping. When both are exactly unity it
 *       goes on to the attack, and on the two scripted fights where a
 *       particular character's Persona lands, every other member standing on
 *       the field takes an impact of their own and is pushed back.
 *
 *   02  Attacking, once the summon's sound bank is in. One test on the
 *       fighter's move picks between the two routines that play it out. It
 *       also clears BTL_OBJ_STATIC on the way in, every frame, which is what
 *       lets the record be driven by hand while the attack runs.
 *
 *   04  Leaving. The colour is walked down to black four steps a frame and the
 *       record is either freed or put back on motion zero, by whether the
 *       artwork is one of the ones kept. A scene waiting to be played takes
 *       the whole thing over instead: nothing fades, and the request is simply
 *       raised.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/talk.h>
#include <libsnd.h>
#include <decomp/include_asm.h>

/* Set on a persona whose record is let go at the end rather than kept: the
   fade frees it instead of putting it back on motion zero, and the landing
   leaves it standing instead of sending it in. It comes off
   g_btl_persona_gfx, so it is a property of the artwork; it is the same bit
   enemyobj.c gives a body. */
#define PERSONA_ONE_SHOT 0x200

/* Unity for the two scales, and how far past it the rise goes. */
#define PERSONA_UNITY 0x1000
#define PERSONA_RISE  0x3000

/* What a frame is worth, at full speed and at half. The rise and the widening
   are a fraction of what is there; the settle is a much smaller one, which is
   what makes it a landing rather than a bounce. */
#define PERSONA_GROW       16
#define PERSONA_GROW_HALF  8
#define PERSONA_SETTLE     0x35
#define PERSONA_SETTLE_HALF 0x1A

/* The motion the landing hands on to, and how far the shock pushes a member
   who is standing on the field when it happens. */
#define PERSONA_MOTION_ATTACK 2
#define PERSONA_SHOCK_Z       (-0x300000)

/* The two scripted fights this happens in, and whose Persona it is. */
#define SHOCK_ENCOUNTER_A 1
#define SHOCK_KEY_A       6
#define SHOCK_ENCOUNTER_B 4
#define SHOCK_KEY_B       7

/* The moves the second of the two attack routines plays out: everything from
   0xA3 up except 0xDB. */
#define PERSONA_MOVE_SPLIT 0xA3
#define PERSONA_MOVE_PLAIN 0xDB

/* How far the colour walks toward black in a frame. */
#define PERSONA_FADE_OUT 4

/* The one the summon is played on. */
#define PERSONA_SEQ 1

/* Raised once the summon's sound bank is in and cleared as it is done with.
   The attack will not start without it. */
extern u_char g_btl_persona_ready;

/* Puts an impact down at a position and hands the record back. Only this unit
   and the two motions beside it in the block reach it. */
extern BtlObj *func_80084E10(int kind, const long *pos);

/* The two halves of an attack, both still in asm beside this unit. */
extern void func_800B12D0(BtlObj *o);
extern void func_800B16BC(BtlObj *o);

extern short g_btl_seq_handle[];

/* 99.35%: the settle's own copy of the height. Loaded before the test, the
   image's register is right and gcc lifts the divide's sign fixup out of the
   two arms; loaded inside the arm, the fixup stays where the image has it and
   the load wants a move. Seven shapes tried and 29,000 permuter iterations
   found nothing under the hand version. */
#ifdef NON_MATCHING
void BtlPersonaMotion03(BtlObj *o)
{
    /* Two locals, not one: the scale as it stands and the scale it is going
       to be. Written through the record instead, every arm reads it again and
       the addition comes out with its operands the other way round; written
       into the one local, the pair share a register the image keeps apart. */
    long scale;
    long drop;
    long step;
    int  i;

    switch (o->phase) {
    case 0:
        g_btl_talk_leaving = 0;
        if (o->timer == 0) {
            if (o->scale_y >= PERSONA_RISE) {
                o->scale_y = PERSONA_RISE;
                o->phase++;
            }
            scale = o->scale_y;
            if (g_btl_half_rate != 0) {
                o->scale_y = scale + scale / PERSONA_GROW_HALF;
            } else {
                o->scale_y = scale + scale / PERSONA_GROW;
            }
        }
        break;

    case 1:
        scale = o->scale_x;
        if (scale < PERSONA_UNITY) {
            if (g_btl_half_rate != 0) {
                step = scale / PERSONA_GROW_HALF;
            } else {
                step = scale / PERSONA_GROW;
            }
            o->scale_x = scale + step;
        } else {
            o->scale_x = PERSONA_UNITY;
        }

        if (o->scale_y > PERSONA_UNITY) {
            drop = o->scale_y;
            if (g_btl_half_rate != 0) {
                step = drop / PERSONA_SETTLE_HALF;
            } else {
                step = drop / PERSONA_SETTLE;
            }
            o->scale_y = drop - step;
        } else {
            o->scale_y = PERSONA_UNITY;
        }


        if (o->scale_x == PERSONA_UNITY && o->scale_y == PERSONA_UNITY) {
            if ((o->attr & PERSONA_ONE_SHOT) != 0) {
                o->motion = 0;
            } else {
                o->motion = PERSONA_MOTION_ATTACK;
                if ((o->attr & BTL_OBJ_TRAIL) == 0
                    && ((g_btl_encounter == SHOCK_ENCOUNTER_A
                         && o->actor->c.key == SHOCK_KEY_A)
                        || (g_btl_encounter == SHOCK_ENCOUNTER_B
                            && o->actor->c.key == SHOCK_KEY_B))) {
                    for (i = 0; i < BTL_PARTY; i++) {
                        /* The summoner is left alone; the key is read back
                           through the record each time round, which is where
                           the image takes it from. */
                        if (g_btl_actors[i].c.key != 0
                            && g_btl_actors[i].c.key != o->actor->c.key) {
                            func_80084E10(0, &g_btl_actors[i].obj->x)->z
                                += PERSONA_SHOCK_Z;
                        }
                    }
                }
            }
            o->phase = 0;
            o->age = 0;
        }
        break;
    }
}

#else
INCLUDE_ASM("btlp/nonmatchings/personamotion", BtlPersonaMotion03);
#endif

void BtlPersonaMotion04(BtlObj *o)
{
    switch (o->phase) {
    case 0:
        if (g_btl_scene_hud != 0) {
            g_btl_scene_wanted = 1;
            return;
        }
        if ((o->attr & BTL_OBJ_TRAIL) == 0) {
            SsSepStop(g_btl_seq_handle[PERSONA_SEQ], 0);
        }
        o->rgb_to[0] = 0;
        o->rgb_to[1] = 0;
        o->rgb_to[2] = 0;
        o->fade = PERSONA_FADE_OUT;
        o->phase++;
        /* fall through */

    case 1:
        if (o->rgb[0] == 0) {
            if ((o->attr & PERSONA_ONE_SHOT) != 0) {
                g_btl_talk_leaving = 0;
                BtlObjFree(o);
            } else {
                o->motion = 0;
                o->phase = 0;
            }
        }
        break;
    }
}

void BtlPersonaMotion02(BtlObj *o)
{
    u_long attr;
    int    move;

    if (g_btl_persona_ready != 0) {
        attr = o->attr;
        o->attr = attr & ~BTL_OBJ_STATIC;
        /* The rarer arm is written first: the image tests the move upward
           and falls through to the second routine, jumping away to the
           first. */
        if ((attr & BTL_OBJ_TRAIL) == 0) {
            move = o->actor->move;
            if (move >= PERSONA_MOVE_SPLIT && move != PERSONA_MOVE_PLAIN) {
                func_800B16BC(o);
            } else {
                func_800B12D0(o);
            }
        }
    }
}
