/* Persona 1 (JP) - choosing what a command is aimed at.  BTLP only.
 *   0x800A2E20 BtlPickTargetMember   0x800A2F84 BtlPickTargetParty
 *   0x800A31BC BtlPickTargetEnemies  0x800A330C BtlHoldMessage
 *
 * Three of the ways a chosen command finds its target, one per shape. Each
 * answers 1 once the fighter has been given an order and a target mask, -1
 * when the player backed out and -2 on the third key; the caller puts the
 * menu back either way.
 *
 * BtlPickTargetMember is the one with a cursor: it walks the party with
 * BtlPickMember and keeps the chosen slot's marker lit by hand, because the
 * marker is not part of what that routine draws.
 *
 * The other two have nothing to choose - the whole party, or every enemy that
 * can be reached - so they light all of them at once and then only wait for a
 * key. The party one tints every member the picker made pickable and the
 * enemy one every enemy, both through the same three colour bytes.
 *
 * BtlPickTargetParty is 98.03%: the marker table is taken again at the bottom
 * of every turn, which is what stops gcc walking it and leaves the actor's
 * object as the only walker - but the address is then materialised there as
 * well as at the top of the body, where the image has only the one copy.
 *
 * BtlHoldMessage is the odd one out: it puts a line up and turns the frame
 * over until any key at all, then takes it back and answers zero. It is a
 * two-step state machine on g_btl_step rather than a loop, because the frame
 * it draws is the round's.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/input.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/stage.h>
#include <persona/btlp/text.h>

/* Set on the marker of whoever is being aimed at, and cleared off the rest. */
#define MARK_CHOSEN 0x1000000

/* How bright everything is drawn while a target is being chosen, and the
   motion a member is put on once they are one. */
#define TARGET_LIT    0x80
#define TARGET_MOTION 10

/* Where the held line goes. */
#define HOLD_X 0x10
#define HOLD_Y 0x94

/* The slot the cursor is on, which starts where the turn is. */
extern short g_btl_target_slot;

extern u_char       D_800CF94C[];
extern const u_char g_btl_tint_pick_r;

extern void BtlTintActorClut(int actor, int r, int g, int b);
extern int  BtlPickMember(short *slot);
extern void BtlOpenItemBoard(void);
extern void func_800C5600(void);
extern void func_800C5A00(void);
extern void func_800C56CC(int slot);
extern int  func_80094C40(void);


int BtlPickTargetMember(BtlActor *a)
{
    int pick;
    int i;

    BtlDrawFrame();
    BtlRefreshMarkers();
    func_800C5600();
    func_800C5A00();
    g_btl_target_slot = g_btl_actor_turn;

    for (;;) {
        pick = BtlPickMember(&g_btl_target_slot);
        if (pick >= 0) {
            break;
        }
        /* The two ways out share their tail, and the image shares it by
           branching rather than by having gcc merge two copies. */
        if (pick != -2) {
            if (pick < -1) {
                goto sweep;
            }
            if (pick != -1) {
                goto sweep;
            }
            BtlRetractMarkers();
            func_800C56CC(g_btl_actor_turn);
            BtlOpenItemBoard();
        }
        BtlEnemiesResetGfx();
        return pick;
    sweep:
        i = 0;
        do {
            g_btl_marker_obj[i]->attr &= ~MARK_CHOSEN;
            i++;
        } while (i < BTL_PARTY);
        g_btl_marker_obj[g_btl_target_slot]->attr |= MARK_CHOSEN;
        BtlDrawFrame();
    }

    BtlEnemiesResetGfx();
    a->order = g_btl_target_slot;
    a->targets = 1 << g_btl_target_slot;
    return 1;
}

#ifdef NON_MATCHING
int BtlPickTargetParty(BtlActor *a)
{
    BtlObj      **marks;
    const u_char *tint;
    int           i;

    BtlDrawFrame();
    BtlRefreshMarkers();
    func_800C5600();
    func_800C5A00();
    i = 0;
    marks = g_btl_marker_obj;
    tint = &g_btl_tint_pick_r;
    do {
        marks[i]->attr &= ~(MARK_CHOSEN | BTL_OBJ_PICKED);
        marks[i]->rgb[0] = TARGET_LIT;
        marks[i]->rgb[1] = TARGET_LIT;
        marks[i]->rgb[2] = TARGET_LIT;
        if (g_btl_actors[i].pickable != 0) {
            g_btl_actors[i].obj->attr &= ~BTL_OBJ_PICKED;
            g_btl_actors[i].obj->rgb[0] = TARGET_LIT;
            g_btl_actors[i].obj->rgb[1] = TARGET_LIT;
            g_btl_actors[i].obj->rgb[2] = TARGET_LIT;
            g_btl_actors[i].obj->motion = TARGET_MOTION;
            BtlTintActorClut(i, tint[0], tint[1], tint[2]);
            marks[i]->attr |= MARK_CHOSEN;
        }
        i++;
        /* Taken again at the bottom of every turn: assigned only before the
           loop it is invariant and gcc walks it, and the actor's object stops
           being the only walker. */
        marks = g_btl_marker_obj;
    } while (i < BTL_PARTY);

    for (;;) {
        if ((g_btl_pad1_edge & g_btl_key_confirm) != 0) {
            a->order = g_btl_actor_turn;
            a->targets = func_80094C40();
            BtlEnemiesResetGfx();
            break;
        }
        if ((g_btl_pad1_edge & g_btl_key_cancel) != 0) {
            func_800C56CC(g_btl_actor_turn);
            BtlRetractMarkers();
            BtlOpenItemBoard();
            BtlEnemiesResetGfx();
            return -1;
        }
        if ((g_btl_pad1_edge & g_btl_key_abort) != 0) {
            func_800C56CC(g_btl_actor_turn);
            BtlEnemiesResetGfx();
            return -2;
        }
        BtlDrawFrame();
    }
    return 1;
}
#else
INCLUDE_ASM("btlp/nonmatchings/targetpick", BtlPickTargetParty);
#endif

int BtlPickTargetEnemies(BtlActor *a)
{
    const u_char *tint;
    int           motion;
    int           i;

    BtlDrawFrame();
    BtlSetPickable();
    /* The counter first, then the motion and the colours: all three are set
       up ahead of the loop and the image has them in that order. */
    i = 0;
    motion = TARGET_MOTION;
    tint = &g_btl_tint_pick_r;
    do {
        if (g_btl_combatants[i].pickable != 0) {
            g_btl_combatants[i].obj->motion = motion;
            BtlTintActorClut(i + BTL_PARTY, tint[0], tint[1], tint[2]);
        }
        i++;
    } while (i < BTL_ENEMIES);

    for (;;) {
        if ((g_btl_pad1_edge & g_btl_key_confirm) != 0) {
            a->order = BtlSlowestOrder() + BTL_PARTY;
            a->targets = BtlPickableMask();
            break;
        }
        if ((g_btl_pad1_edge & g_btl_key_cancel) != 0) {
            BtlEnemiesResetGfx();
            return -1;
        }
        if ((g_btl_pad1_edge & g_btl_key_abort) != 0) {
            BtlEnemiesResetGfx();
            return -2;
        }
        BtlDrawFrame();
    }
    return 1;
}

int BtlHoldMessage(void)
{
    for (;;) {
        switch (g_btl_step) {
        case 0:
            BtlOpenMessage(0, 0, D_800CF94C, HOLD_X, HOLD_Y);
            g_btl_step++;
            break;
        case 1:
            if (g_btl_pad1_edge != 0) {
                BtlCloseMessage(0);
                return 0;
            }
            break;
        }
        BtlDrawFrame();
    }
}
