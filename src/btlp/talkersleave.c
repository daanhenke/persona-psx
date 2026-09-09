/* Persona 1 (JP) - the demons walking away from a negotiation.  BTLP only.
 *   0x80067D9C BtlTalkersLeave
 *
 * Nine places call this when a negotiation ends with the group leaving rather
 * than fighting. Every enemy the offer involves is put on motion 0xE - the
 * leaving animation - and the group's voice bank is opened for it, chosen from
 * the speaker's own object so the right voice says goodbye.
 *
 * The speaker goes first, on the frame this is called; the rest are staggered
 * by a random multiple of two frames from twenty, so they do not all turn at
 * once. Then the frame keeps drawing until none of them is left: an enemy that
 * has walked off clears its own key, so the wait ends when the mask no longer
 * finds one standing.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/battle.h>

/* The motion an enemy is put on to leave. */
#define BTL_MOTION_LEAVE 0xE

/* The bank the group's goodbye is opened in, and how its index comes out of
   the speaker's object. */
#define BTL_LEAVE_BANK 6
#define BTL_LEAVE_BIAS 5

/* How long the others wait before they follow: twenty frames plus an even
   number under ten. */
#define BTL_LEAVE_DELAY 0x14
#define BTL_LEAVE_SPREAD 5

extern const BtlSoundBank g_btl_slot_banks[];

extern short BtlPickTalkTarget(short mask);
extern void BtlSoundClose(int slot);

void BtlTalkersLeave(void)
{
    BtlActor *e;
    int       i;
    int       left;
    int       mask;

    BtlEnemiesReset();
    i = 0;
    g_btl_talk_target = BtlPickTalkTarget(g_btl_offer[g_btl_offer_slot].used);
    e = &g_btl_enemies[g_btl_talk_target];
    BtlSoundOpen(g_btl_slot_banks, BTL_LEAVE_BANK,
                 (e->obj->unkCD >> 1) - BTL_LEAVE_BIAS);
    BtlDrawFrame();
    e->obj->motion = BTL_MOTION_LEAVE;
    e->obj->timer = 0;

    e = g_btl_enemies;
    do {
        if (g_btl_talk_target != i
            && ((g_btl_offer[g_btl_offer_slot].used >> i) & 1) != 0
            && e->c.key != 0) {
            e->obj->motion = BTL_MOTION_LEAVE;
            e->obj->timer = rand() % BTL_LEAVE_SPREAD * 2 + BTL_LEAVE_DELAY;
        }
        i++;
        e++;
    } while (i < BTL_ENEMIES);

    left = BTL_ENEMIES;
    do {
        e = g_btl_enemies;
        i = 0;
        /* The mask is read once a pass here, where the first loop reads it
           per enemy; that is the difference between the two. */
        mask = g_btl_offer[g_btl_offer_slot].used;
        while (i < BTL_ENEMIES) {
            if (((mask >> i) & 1) != 0 && e->c.key != 0) {
                break;
            }
            i++;
            e++;
        }
        if (i == left) {
            break;
        }
        BtlDrawFrame();
    } while (1);
    BtlSoundClose(BTL_LEAVE_BANK);
}
