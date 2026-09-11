/* Persona 1 (JP) - picking an enemy.  BTLP only.
 *   0x8009ACD0 BtlPickEnemy   0x8009AFF8 BtlPickEnemyLit
 *
 * The other side of pickmember.c, and one frame of it each, with the cursor's
 * slot passed by pointer so the caller can hold it between frames. A cursor
 * sitting on an empty record is walked forward before anything else, which is
 * what keeps it off an enemy that has just been killed.
 *
 * BtlPickEnemy draws all nine to match: the one under the cursor is tinted
 * and put on the picked motion, everyone else goes back to their own palette
 * at full brightness - or is left at a fifth of it, palette untouched, if
 * they are not a target this time.
 *
 * BtlPickEnemyLit does none of that. It only puts the nine back to full
 * brightness, and only on a frame the cursor actually moved on, so whatever
 * the caller had tinted goes back by itself.
 *
 * Both answer the slot on confirm, -1 on cancel and -2 on the third key, with
 * the enemies' graphics put back first; -0x100 means the player has not
 * decided yet. A cursor already off the end answers -1 to any key at all.
 */
#include <decomp/types.h>
#include <decomp/libc.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/input.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/object.h>
#include <persona/btlp/sound.h>

/* The motion the picked enemy is put on. */
#define PICK_MOTION 10

/* How bright the others are drawn, and how fast they get there. */
#define PICK_LIT  0x80
#define PICK_DIM  0x20
#define PICK_FADE 8

/* Cleared on every enemy the cursor is not on, and set again by the lit
   sweep. */
#define BTL_OBJ_PICKED 0x400000

/* One actor's palette. */
#define CLUT_BYTES 0x200

extern u_char *g_btl_enemy_clut;
extern u_char *g_btl_enemy_clut_base;
/* Three variables, not an array - the original reaches each on its own. */
extern const u_char g_btl_tint_pick_r;
extern const u_char g_btl_tint_pick_g;
extern const u_char g_btl_tint_pick_b;

extern void BtlTintActorClut(int actor, int r, int g, int b);


int BtlPickEnemy(short *slot)
{
    BtlObj *obj;
    u_short edge;
    int     keys;
    int     i;
    u_long  keep;
    u_long  attr;

    keys = BtlMenuKey();
    if (g_btl_combatants[*slot].c.key == 0) {
        *slot = BtlPickableNext(*slot);
    }
    if ((keys & PAD_LEFT) != 0) {
        *slot = BtlPickablePrev(*slot);
        BtlSePlay(PICK_SE_BANK, PICK_SE_MOVE);
    }
    if ((keys & PAD_RIGHT) != 0) {
        *slot = BtlPickableNext(*slot);
        BtlSePlay(PICK_SE_BANK, PICK_SE_MOVE);
    }
    i = 0;

    /* The mask is a local so it stays in a register for the whole loop. */
    keep = ~BTL_OBJ_PICKED;
    do {
        if (g_btl_combatants[i].c.key != 0) {
            obj = g_btl_combatants[i].obj;
            if (i == *slot) {
                BtlTintActorClut(i + BTL_PARTY, g_btl_tint_pick_r,
                                 g_btl_tint_pick_g, g_btl_tint_pick_b);
                BtlObjSetMotion(obj, PICK_MOTION);
            } else {
                /* Both arms are written out whole, colour and fade and all.
                   gcc merges the tails back down to the one the image has -
                   the stores and the call - and what is left in each arm is
                   the colour, which is where the image materialises it. */
                if (g_btl_combatants[i].pickable != 0) {
                    memcpy(g_btl_enemy_clut + i * CLUT_BYTES,
                           g_btl_enemy_clut_base + i * CLUT_BYTES, CLUT_BYTES);
                    BtlObjSetMotion(obj, 0);
                    BtlObjSetPhase(obj, 0);
                    obj->rgb_to[0] = PICK_LIT;
                    obj->rgb_to[1] = PICK_LIT;
                    obj->rgb_to[2] = PICK_LIT;
                    BtlObjSetFade(obj, PICK_FADE);
                } else {
                    BtlObjSetMotion(obj, 0);
                    BtlObjSetPhase(obj, 0);
                    obj->rgb_to[0] = PICK_DIM;
                    obj->rgb_to[1] = PICK_DIM;
                    obj->rgb_to[2] = PICK_DIM;
                    BtlObjSetFade(obj, PICK_FADE);
                }
                attr = obj->attr;
                obj->attr = attr & keep;
            }
        }
        i++;
    } while (i < BTL_ENEMIES);

    if (*slot < 0) {
        if (g_btl_pad1_edge != 0) {
            return -1;
        }
        return BTL_PICK_WAIT;
    }
    edge = g_btl_pad1_edge;
    if ((edge & g_btl_key_confirm) != 0) {
        BtlEnemiesResetGfx();
        return *slot;
    }
    if ((edge & g_btl_key_cancel) != 0) {
        BtlEnemiesResetGfx();
        return -1;
    }
    if ((edge & g_btl_key_abort) == 0) {
        return BTL_PICK_WAIT;
    }
    BtlEnemiesResetGfx();
    return -2;
}

int BtlPickEnemyLit(short *slot)
{
    BtlObj   *obj;
    u_short   edge;
    int       keys;
    int       i;
    u_long    attr;

    keys = BtlMenuKey();
    if (g_btl_combatants[*slot].c.key == 0) {
        *slot = BtlPickableNext(*slot);
    }
    if ((keys & PAD_LEFT) != 0) {
        *slot = BtlPickablePrev(*slot);
        BtlSePlay(PICK_SE_BANK, PICK_SE_MOVE);
    }
    if ((keys & PAD_RIGHT) != 0) {
        *slot = BtlPickableNext(*slot);
        BtlSePlay(PICK_SE_BANK, PICK_SE_MOVE);
    }
    if ((keys & (PAD_LEFT | PAD_RIGHT)) != 0) {
        i = 0;
        do {
            if (g_btl_combatants[i].c.key != 0) {
                obj = g_btl_combatants[i].obj;
                attr = obj->attr;
                obj->rgb_to[0] = PICK_LIT;
                obj->rgb_to[1] = PICK_LIT;
                obj->rgb_to[2] = PICK_LIT;
                obj->rgb[0] = PICK_LIT;
                obj->rgb[1] = PICK_LIT;
                obj->rgb[2] = PICK_LIT;
                obj->attr = attr | BTL_OBJ_PICKED;
            }
            i++;
        } while (i < BTL_ENEMIES);
    }

    if (*slot < 0) {
        if (g_btl_pad1_edge != 0) {
            return -1;
        }
        return BTL_PICK_WAIT;
    }
    edge = g_btl_pad1_edge;
    if ((edge & g_btl_key_confirm) != 0) {
        BtlEnemiesResetGfx();
        return *slot;
    }
    if ((edge & g_btl_key_cancel) != 0) {
        BtlEnemiesResetGfx();
        return -1;
    }
    if ((edge & g_btl_key_abort) == 0) {
        return BTL_PICK_WAIT;
    }
    BtlEnemiesResetGfx();
    return -2;
}
