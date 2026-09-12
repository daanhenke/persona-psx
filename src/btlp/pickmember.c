/* Persona 1 (JP) - picking a party member.  BTLP only.
 *   0x8009B1D0 BtlPickMember
 *
 * One frame of it, with the cursor's slot passed by pointer so the caller can
 * hold it between frames.
 *
 * Left and right walk the cursor over the members that are standing. All five
 * are then drawn to match: the one under the cursor is tinted and put on the
 * picked motion, and everyone else goes back to their own colours at full
 * brightness - or at a fifth of it, and with their palette left alone, if they
 * are not one of the side being picked from.
 *
 * The answer is the slot on confirm, -1 on cancel and -2 on the third key,
 * with the party's graphics put back first; -0x100 means the player has not
 * decided yet. A cursor already off the end answers -1 to any key at all,
 * which is how a pick with nothing to pick is backed out of.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/input.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/sound.h>

/* PadRead's two sideways bits, as BtlMenuKey hands them back. */

/* The click, and the motion the picked member is put on. */
#define PICK_MOTION  10

/* How bright the others are drawn, and how fast they get there. */
#define PICK_LIT  0x80
#define PICK_DIM  0x20
#define PICK_FADE 8

/* One actor's palette. */
#define CLUT_BYTES 0x200

/* Nothing decided this frame. */

extern u_char  *g_btl_actor_clut;
extern u_char  *g_btl_actor_clut_base;
/* Three variables, not an array - the original reaches each on its own. */
extern const u_char g_btl_tint_pick_r;
extern const u_char g_btl_tint_pick_g;
extern const u_char g_btl_tint_pick_b;

extern void  BtlTintActorClut(int actor, int r, int g, int b);

int BtlPickMember(short *slot)
{
    BtlObj *obj;
    u_short edge;
    int     keys;
    int     i;
    u_long  keep;
    u_long  attr;

    keys = BtlMenuKey();
    if ((keys & PAD_LEFT) != 0) {
        *slot = BtlCursorPrev(*slot);
        BtlSePlay(PICK_SE_BANK, PICK_SE_MOVE);
    }
    if ((keys & PAD_RIGHT) != 0) {
        *slot = BtlCursorNext(*slot);
        BtlSePlay(PICK_SE_BANK, PICK_SE_MOVE);
    }
    i = 0;

    /* The mask is a local so it stays in a register for the whole loop. */
    keep = ~BTL_OBJ_PICKED;
    do {
        if (g_btl_actors[i].c.key != 0) {
            obj = g_btl_actors[i].obj;
            if (i == *slot) {
                BtlTintActorClut(i, g_btl_tint_pick_r, g_btl_tint_pick_g,
                                 g_btl_tint_pick_b);
                BtlObjSetMotion(obj, PICK_MOTION);
            } else {
                if (g_btl_actors[i].pickable != 0) {
                    memcpy(g_btl_actor_clut + i * CLUT_BYTES,
                           g_btl_actor_clut_base + i * CLUT_BYTES, CLUT_BYTES);
                    BtlObjSetMotion(obj, 0);
                    BtlObjSetRgb(obj, PICK_LIT, PICK_LIT, PICK_LIT);
                } else {
                    BtlObjSetMotion(obj, 0);
                    BtlObjSetRgb(obj, PICK_DIM, PICK_DIM, PICK_DIM);
                }
                BtlObjSetFade(obj, PICK_FADE);
                /* Through a local, and the mask on the right: either the
                   other way round costs a register. */
                attr = obj->attr;
                obj->attr = attr & keep;
            }
        }
        i++;
    } while (i < BTL_PARTY);

    if (*slot < 0) {
        if (g_btl_pad1_edge != 0) {
            return -1;
        }
        return BTL_PICK_WAIT;
    }
    edge = g_btl_pad1_edge;
    if ((edge & g_btl_key_confirm) != 0) {
        BtlPartyResetGfx();
        return *slot;
    }
    if ((edge & g_btl_key_cancel) != 0) {
        BtlPartyResetGfx();
        return -1;
    }
    if ((edge & g_btl_key_abort) == 0) {
        return BTL_PICK_WAIT;
    }
    BtlPartyResetGfx();
    return -2;
}
