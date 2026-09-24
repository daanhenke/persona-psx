/* Persona 1 (JP) - the move that sends the whole enemy side away.  BTLP only.
 *   0x800BFA04 BtlFxStepE8
 *
 * The head record waits for its script, shuts the field's music down and lays
 * a sheet over everything, then puts every enemy still standing on the leaving
 * motion a frame apart and raises the trail bit on each. When they are gone it
 * closes the five voice slots and puts itself back on phase nought.
 *
 * None of that happens in a fight that may not be run from - the record still
 * lays its sheet and holds, so the move looks the same, but nobody leaves.
 *
 * The sheet's own cells uncover themselves and free themselves when their
 * script has played out.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/spellfx.h>

/* The music slot the field's own track is on, and the five voice slots. */
#define FX_E8_MUSIC 3
#define FX_E8_VOICE 7
#define FX_E8_VOICES 5

/* Which staged table the sheet comes from, and the mark its cells carry. */
#define FX_E8_SHEET 1
#define FX_E8_MARK  0x11

/* How long the record waits for the sheet to arrive, how long it then holds
   past the last enemy leaving, and how much later each enemy starts. */
#define FX_E8_SETTLE  0x1E
#define FX_E8_HOLD    0x3C
#define FX_E8_STAGGER 2

/* The motion an enemy leaves on, and the bit it is given as it goes. Nothing
   else in the overlay sets that bit, so what reads it is not established. */
#define FX_E8_MOTION 0xE
#define FX_E8_GOING  0x800

extern void BtlFxReopenVoices(void);

/* As in BtlFxStep72, walk the enemy object fields by actor stride while
   retaining the full slot for the actor checks. */
void BtlFxStepE8(BtlObj *o)
{
    BtlObj *n;
    int     i;
    int     slot;

    if (o->mark_num == FX_MARK_HEAD) {
        switch (o->phase) {
        case 0:
            if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
                break;
            }
            if (g_btl_no_escape == 0) {
                BtlSoundClose(FX_E8_MUSIC);
                BtlFxReopenVoices();
            }
            n = BtlOpenFxGrid(FX_E8_SHEET);
            BtlObjSetKind(n, o->kind);
            BtlObjSetMotion(n, o->motion);
            BtlObjSetMarkNum(n, FX_E8_MARK);
            o->timer = FX_E8_SETTLE;
            o->phase++;
            break;
        case 1:
            if (o->timer != 0) {
                break;
            }
            i = 0;
            if (g_btl_no_escape == 0) {
                for (slot = BTL_PARTY; slot < BTL_ACTORS; slot++) {
                    if (g_btl_actors[slot].c.key == 0) {
                        continue;
                    }
                    if ((signed char)g_btl_actors[slot].c.status
                            == BTL_STATUS_DOWN) {
                        continue;
                    }
                    if ((g_btl_actors[slot].flags & BTL_ACTOR_OUT) != 0) {
                        continue;
                    }
                    (*(BtlObj **)((u_char *)&g_btl_enemies[0].obj
                                  + (slot - BTL_PARTY) * sizeof(BtlActor)))->motion = FX_E8_MOTION;
                    (*(BtlObj **)((u_char *)&g_btl_enemies[0].obj
                                  + (slot - BTL_PARTY) * sizeof(BtlActor)))->timer = i * FX_E8_STAGGER;
                    i++;
                    (*(BtlObj **)((u_char *)&g_btl_enemies[0].obj
                                  + (slot - BTL_PARTY) * sizeof(BtlActor)))->attr |= FX_E8_GOING;
                }
            }
            o->timer = i * FX_E8_STAGGER + FX_E8_HOLD;
            o->phase++;
            break;
        case 2:
            if (o->timer != 0) {
                break;
            }
            if (g_btl_no_escape == 0) {
                for (slot = 0; slot < FX_E8_VOICES; slot++) {
                    BtlSoundClose(slot + FX_E8_VOICE);
                }
            }
            o->motion = 0;
            o->phase = 0;
            break;
        default:
            break;
        }
        return;
    }
    switch (o->phase) {
    case 0:
        if (o->timer != 0) {
            break;
        }
        o->attr &= ~(BTL_OBJ_HIDDEN | BTL_OBJ_STATIC);
        o->phase++;
        break;
    case 1:
        if ((o->attr & BTL_OBJ_ANIMATING) != 0) {
            break;
        }
        BtlObjFree(o);
        break;
    default:
        break;
    }
}
