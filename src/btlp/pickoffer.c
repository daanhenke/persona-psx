/* Persona 1 (JP) - choosing which offer the negotiation is looking at.
 *   BTLP only.  0x80067FA0 BtlPickOffer
 *
 * One frame of it, with the slot passed by pointer so the caller can hold it
 * between frames. Left and right walk over the three offer slots and keep
 * walking past any that is free, so a slot nobody is on is never landed on;
 * with all three free the walk does not terminate, which is why the caller
 * only opens the pick once an offer exists.
 *
 * The enemies show which offer is under the cursor: each bit of the offer's
 * `used` mask is one enemy slot, and the ones it names are put on the picked
 * motion at full brightness while the rest go dim. Confirm and cancel put all
 * nine back to normal first.
 *
 * The answer is the slot on confirm and -1 on cancel or abort; -0x100 means the
 * player has not decided yet.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/offer.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/input.h>
#include <persona/btlp/sound.h>

/* The clicks, and the motion the named enemies are put on. */
#define PICK_SE_BANK    1
#define PICK_SE_CONFIRM 1
#define PICK_SE_CANCEL  2
#define PICK_SE_MOVE    3
#define PICK_MOTION     10

/* How bright an enemy is drawn, and how fast it gets there. */
#define PICK_LIT   0x80
#define PICK_DIM   0x40
#define PICK_FADE  8
#define PICK_RESET_FADE 0x10

/* Nothing decided this frame. */
#define BTL_PICK_WAIT (-0x100)

extern int  BtlMenuKey(void);

/* One step round the three offers, wrapping at both ends. The argument is
   evaluated at every use, as the image does it. */
#define PICK_WRAP(n) \
    ((n) < 0 ? BTL_OFFERS - 1 : ((n) > BTL_OFFERS - 1 ? 0 : (n)))

/* Whether anyone is on an offer. A statement expression, and that matters:
   the block inside it stops gcc copying the walk's exit test in front of the
   loop, so the test stays at the loop head, reloads the slot, and the two
   wraps share their tails as the image has them. A plain condition has the
   test copied behind the first wrap, where cse reuses the stored value. */
#define PICK_USED(n) ({ g_btl_offer[n].used; })

/* The key word is an int holding the pad bits as a u_short, which is what
   keeps the image's mask; the frame the old `unused[8]` stood in for is the
   wrap ternaries stored through the short pointer. */
int BtlPickOffer(short *slot)
{
    BtlActor *a;
    u_short   edge;
    int       keys;
    int       i;

    keys = (u_short)BtlMenuKey();
    if ((keys & PAD_LEFT) != 0) {
        BtlSePlay(PICK_SE_BANK, PICK_SE_MOVE);
        *slot = PICK_WRAP(*slot - 1);
        while (PICK_USED(*slot) == 0) {
            *slot = PICK_WRAP(*slot - 1);
        }
    }
    if ((keys & PAD_RIGHT) != 0) {
        BtlSePlay(PICK_SE_BANK, PICK_SE_MOVE);
        *slot = PICK_WRAP(*slot + 1);
        while (PICK_USED(*slot) == 0) {
            *slot = PICK_WRAP(*slot + 1);
        }
    }

    edge = g_btl_pad1_edge;
    if ((edge & (g_btl_key_cancel | g_btl_key_abort)) != 0) {
        BtlSePlay(PICK_SE_BANK, PICK_SE_CANCEL);
        i = 0;
        a = g_btl_enemies;
        do {
            i++;
            if (a->c.key != 0) {
                BtlObjSetMotion(a->obj, 0);
                BtlObjSetRgb(a->obj, PICK_LIT, PICK_LIT, PICK_LIT);
                BtlObjSetFade(a->obj, PICK_RESET_FADE);
            }
            a++;
        } while (i < BTL_ENEMIES);
        return -1;
    }
    if ((edge & g_btl_key_confirm) != 0) {
        BtlSePlay(PICK_SE_BANK, PICK_SE_CONFIRM);
        i = 0;
        a = g_btl_enemies;
        do {
            i++;
            if (a->c.key != 0) {
                BtlObjSetMotion(a->obj, 0);
                BtlObjSetRgb(a->obj, PICK_LIT, PICK_LIT, PICK_LIT);
                BtlObjSetFade(a->obj, PICK_RESET_FADE);
            }
            a++;
        } while (i < BTL_ENEMIES);
        return *slot;
    }

    i = 0;
    a = g_btl_enemies;
    do {
        if (a->c.key != 0) {
            if ((g_btl_offer[*slot].used >> i & 1) != 0) {
                BtlObjSetMotion(a->obj, PICK_MOTION);
                BtlObjSetRgb(a->obj, PICK_LIT, PICK_LIT, PICK_LIT);
            } else {
                BtlObjSetMotion(a->obj, 0);
                BtlObjSetRgb(a->obj, PICK_DIM, PICK_DIM, PICK_DIM);
                BtlObjSetFade(a->obj, PICK_FADE);
            }
        }
        i++;
        a++;
    } while (i < BTL_ENEMIES);

    return BTL_PICK_WAIT;
}

