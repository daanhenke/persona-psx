/* Persona 1 (JP) - a member's record walking on and off in a scene.
 * BTLP only.
 *   0x8008D02C BtlMemberMotion0D
 *
 * Entry 0x0D of g_btl_member_motion, named for the motion like the rest of the
 * table. What it does depends on the record's kind.
 *
 * Kinds 0x9A and 0x9B take one script and hold it two seconds, take a second
 * and glide along their steps, are put down at their row's height, take a third
 * and wait for it to finish before going idle.
 *
 * Kinds 0xBD to 0xC0 take their script with a sound and wait for it, glide
 * along their steps, are put down at their row's height and held a second,
 * then take a closing script with a second sound and wait for that.
 *
 * Every other kind just glides, and once its steps are spent is put down at
 * its row's height - on the other side's baseline if it belongs to it - and
 * goes idle.
 */
#include <decomp/types.h>
#include <persona/btlp/object.h>
#include <persona/btlp/sound.h>

/* The two runs of kinds with a scene of their own. */
#define WALK_A_FIRST 0x9A
#define WALK_A_LAST  0x9B
#define WALK_B_FIRST 0xBD
#define WALK_B_LAST  0xC0

/* The scripts each run takes, out of the model's table. */
#define WALK_A_OPEN  45
#define WALK_A_GLIDE 46
#define WALK_A_CLOSE 47
#define WALK_B_OPEN  29
#define WALK_B_CLOSE 48

/* How long each run holds, and the bank and sounds the second makes. */
#define WALK_A_HOLD  0x78
#define WALK_B_HOLD  0x3C
#define WALK_BANK    3
#define WALK_B_SOUND_OPEN  0
#define WALK_B_SOUND_CLOSE 1

/* Where a row is put down: twenty pixels a row, from either side's baseline.
   The attribute bit that says the record belongs to the other side. */
#define WALK_ROW_H      20
#define WALK_ROW_ORG    (-0x8C)
#define WALK_ROW_OTHER  0x3C
#define WALK_OTHER_SIDE 0x200

void BtlMemberMotion0D(BtlObj *o)
{
    if (o->kind == WALK_A_FIRST || o->kind == WALK_A_LAST) {
        switch (o->phase) {
        case 0:
            BtlObjSetScript(o, (BtlSeqStep *)o->scripts[WALK_A_OPEN]);
            o->timer = WALK_A_HOLD;
            o->phase++;
            break;
        case 1:
            if (o->timer != 0) {
                break;
            }
            BtlObjSetScript(o, (BtlSeqStep *)o->scripts[WALK_A_GLIDE]);
            o->phase++;
            break;
        case 2:
            o->y += o->step_y;
            if (--o->steps != 0) {
                break;
            }
            o->y2 = o->y = ((u_char)o->row * WALK_ROW_H + WALK_ROW_ORG) << 16;
            o->phase++;
            break;
        case 3:
            BtlObjSetScript(o, (BtlSeqStep *)o->scripts[WALK_A_CLOSE]);
            o->phase++;
            break;
        case 4:
            if (o->attr & BTL_OBJ_ANIMATING) {
                break;
            }
            o->motion = 0;
            o->phase = 0;
            break;
        }
    } else if (o->kind >= WALK_B_FIRST && o->kind <= WALK_B_LAST) {
        switch (o->phase) {
        case 0:
            BtlObjSetScript(o, (BtlSeqStep *)o->scripts[WALK_B_OPEN]);
            BtlSePlay(WALK_BANK, WALK_B_SOUND_OPEN);
            o->phase++;
            break;
        case 1:
            if ((o->attr & BTL_OBJ_ANIMATING) == 0) {
                o->phase++;
            }
            break;
        case 2:
            o->y += o->step_y;
            if (--o->steps != 0) {
                break;
            }
            o->timer = WALK_B_HOLD;
            o->y2 = o->y = ((u_char)o->row * WALK_ROW_H + WALK_ROW_ORG) << 16;
            o->phase++;
            break;
        case 3:
            if (o->timer != 0) {
                break;
            }
            BtlObjSetScript(o, (BtlSeqStep *)o->scripts[WALK_B_CLOSE]);
            BtlSePlay(WALK_BANK, WALK_B_SOUND_CLOSE);
            o->phase++;
            break;
        case 4:
            if (o->attr & BTL_OBJ_ANIMATING) {
                break;
            }
            o->motion = 0;
            o->phase = 0;
            break;
        }
    } else {
        o->y += o->step_y;
        if (--o->steps == 0) {
            o->y = ((o->attr & WALK_OTHER_SIDE)
                        ? (u_char)o->row * WALK_ROW_H + WALK_ROW_ORG
                        : (u_char)o->row * WALK_ROW_H + WALK_ROW_OTHER) << 16;
            o->motion = 0;
            o->phase = 0;
            o->y2 = o->y;
        }
    }
}
