/* Persona 1 (JP) - one frame for each of the last four object groups.
 * BTLP only.
 *   0x8008AF80 BtlTickEffects  0x8008AFC4 BtlTickPersona
 *   0x8008B0C0 BtlTickEnemy    0x8008B124 BtlTickMember
 *
 * g_btl_obj_tick has one of these per group and BtlTickObjects calls it on
 * every record in that group. The last three are the same routine three times
 * over: the record's `motion` picks a handler out of the group's own table and
 * the record is put back to phase nought where the table has nothing.
 *
 * The handlers take no arguments - they are reached through the table, and the
 * record is already in the argument register when the call is made.
 *
 * The two differences are worth naming. A summoned Persona is lifted: its
 * height is g_btl_persona_hover read a quarter as fast as the record ages, so
 * sixteen entries cover sixty-four frames of bobbing, and that is added to the
 * position the record was put at rather than replacing it. Its colour also
 * pulses between half grey and a quarter of it, and only the leading layer of
 * the summon does that. A fighter's record is instead left alone entirely
 * while it is held.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* Still assembly: what an effect record that follows the negotiation gets
   instead of the move's own step. */
extern void func_800BFE14();

/* The four groups' motion tables. Nothing calls an entry by hand. */
extern void (*g_btl_persona_motion[])();
extern void (*g_btl_enemy_motion[])();
extern void (*g_btl_member_motion[])();

/* How far a summoned Persona floats, one entry per four frames of its age. */
extern long g_btl_persona_hover[];

#define HOVER_ENTRIES 16
#define HOVER_SHIFT   2
#define HOVER_FIXED   16

/* The motion a Persona is on while it is being drawn in, which does its own
   colour work and must not be pulsed under. */
#define PERSONA_ARRIVING 4

/* The two greys it pulses between, as the pair of shorts the record keeps
   them in and on their own. */
#define PERSONA_LIT      0x80
#define PERSONA_DIM      0x40
#define PERSONA_LIT_PAIR 0x00800080
#define PERSONA_DIM_PAIR 0x00400040

void BtlTickEffects(BtlObj *o)
{
    if ((o->attr & BTL_OBJ_ATTR_2000) != 0) {
        func_800BFE14();
    } else {
        BtlFxObjTick(o);
    }
}

void BtlTickPersona(BtlObj *o)
{
    void (*motion)();
    short n;

    motion = g_btl_persona_motion[o->motion];
    if (motion != 0) {
        motion();
    } else {
        o->phase = 0;
    }
    o->z = (g_btl_persona_hover[(o->age >> HOVER_SHIFT) & (HOVER_ENTRIES - 1)]
            << HOVER_FIXED) + o->z2;
    if (o->motion != PERSONA_ARRIVING && (o->attr & BTL_OBJ_TRAIL) == 0) {
        /* The first two channels are tested as the one word the record keeps
           them in, which is what the image compares. */
        if (*(long *)&o->rgb[0] == PERSONA_LIT_PAIR
            && o->rgb[2] == PERSONA_LIT) {
            n = PERSONA_DIM;
        } else if (*(long *)&o->rgb[0] == PERSONA_DIM_PAIR
                   && o->rgb[2] == PERSONA_DIM) {
            n = PERSONA_LIT;
        } else {
            return;
        }
        o->rgb_to[0] = n;
        o->rgb_to[1] = n;
        o->rgb_to[2] = n;
    }
}

void BtlTickEnemy(BtlObj *o)
{
    void (*motion)();

    motion = g_btl_enemy_motion[o->motion];
    if (motion != 0 && (o->attr & BTL_OBJ_HELD) == 0) {
        motion();
    } else {
        o->phase = 0;
    }
}

void BtlTickMember(BtlObj *o)
{
    void (*motion)();

    motion = g_btl_member_motion[o->motion];
    if (motion != 0 && (o->attr & BTL_OBJ_HELD) == 0) {
        motion();
    } else {
        o->phase = 0;
    }
}
