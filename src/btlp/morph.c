/* Persona 1 (JP) - a fighter changing into something else.  BTLP only.
 *   0x800B5950 BtlStepMorph  0x800B599C BtlMorphPair
 *   0x800B5A98 BtlMorphTrio
 *
 * One object motion, in two versions the encounter picks between: the fight
 * that starts on 0x20 can turn into any of three things and the one on 0x22
 * into either of two. Both do the same work - arm the script for the shape
 * the actor is in, give the actor the character key that shape fights as,
 * load that key's numbers over it, and let the rest of the battle know its
 * attacks have changed.
 *
 * The motion then waits: phase 1 does nothing until the object stops running
 * its script, at which point the motion clears itself and the object goes
 * back to being idle.
 *
 * BtlStepMorph never touches its argument - it is passed straight through in
 * the register it arrived in, which is why the two halves are written as
 * plain calls rather than as one routine with a table.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>

/* The two fights that have a shape to change into. */
#define BTL_ENCOUNTER_TRIO 0x20
#define BTL_ENCOUNTER_PAIR 0x22

/* The shapes, as the actor's form byte spells them, and the character key
   each one fights as. */
#define MORPH_PAIR_SECOND 0x25
#define MORPH_PAIR_KEY_A  0xAA
#define MORPH_PAIR_KEY_B  0xAB

#define MORPH_TRIO_SECOND 0x2B
#define MORPH_TRIO_THIRD  0x2D
#define MORPH_TRIO_KEY_A  0xAC
#define MORPH_TRIO_KEY_B  0xAD
#define MORPH_TRIO_KEY_C  0xAE

extern short g_btl_encounter;

extern void BtlActorFromDef(BtlActor *a, int key);
extern void BtlRefreshAttacks(void);

void BtlMorphPair(BtlObj *obj);
void BtlMorphTrio(BtlObj *obj);

void BtlStepMorph(BtlObj *obj)
{
    if (g_btl_encounter != BTL_ENCOUNTER_TRIO) {
        if (g_btl_encounter == BTL_ENCOUNTER_PAIR) {
            BtlMorphPair(obj);
        }
    } else {
        BtlMorphTrio(obj);
    }
}

void BtlMorphPair(BtlObj *obj)
{

    switch (obj->phase) {
    case 0:
        BtlObjSetScript(obj,
                        (BtlSeqStep *)obj->scripts[obj->actor->form]);
        if (obj->actor->form == MORPH_PAIR_SECOND) {
            obj->kind = MORPH_PAIR_KEY_B;
            obj->actor->c.key = MORPH_PAIR_KEY_B;
            BtlActorFromDef(obj->actor, MORPH_PAIR_KEY_B);
        } else {
            obj->kind = MORPH_PAIR_KEY_A;
            obj->actor->c.key = MORPH_PAIR_KEY_A;
            BtlActorFromDef(obj->actor, MORPH_PAIR_KEY_A);
        }
        BtlRefreshAttacks();
        obj->phase = obj->phase + 1;
        break;
    case 1:
        if ((obj->attr & BTL_OBJ_BUSY_MASK) != BTL_OBJ_BUSY) {
            obj->motion = 0;
            obj->phase = 0;
        }
        break;
    }
}

void BtlMorphTrio(BtlObj *obj)
{

    switch (obj->phase) {
    case 0:
        BtlObjSetScript(obj,
                        (BtlSeqStep *)obj->scripts[obj->actor->form]);
        switch (obj->actor->form) {
        case MORPH_TRIO_SECOND:
            obj->kind = MORPH_TRIO_KEY_B;
            obj->actor->c.key = MORPH_TRIO_KEY_B;
            BtlActorFromDef(obj->actor, MORPH_TRIO_KEY_B);
            break;
        case MORPH_TRIO_THIRD:
            obj->kind = MORPH_TRIO_KEY_C;
            obj->actor->c.key = MORPH_TRIO_KEY_C;
            BtlActorFromDef(obj->actor, MORPH_TRIO_KEY_C);
            break;
        default:
            obj->kind = MORPH_TRIO_KEY_A;
            obj->actor->c.key = MORPH_TRIO_KEY_A;
            BtlActorFromDef(obj->actor, MORPH_TRIO_KEY_A);
            break;
        }
        BtlRefreshAttacks();
        obj->phase = obj->phase + 1;
        break;
    case 1:
        if ((obj->attr & BTL_OBJ_BUSY_MASK) != BTL_OBJ_BUSY) {
            obj->motion = 0;
            obj->phase = 0;
        }
        break;
    }
}
