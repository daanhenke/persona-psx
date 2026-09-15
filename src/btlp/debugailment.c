/* Persona 1 (JP) - the two fighter editors on the debug page.  BTLP only.
 *   0x800A065C BtlDebugMemberAilment  0x800A09F0 BtlDebugEnemyAilment
 *
 * Rows 2 and 3 of g_btl_debug_actions, and the same routine twice over: one
 * walks the party with BtlPickMember, the other the field with BtlPickEnemy,
 * and both switch every slot on their side pickable first so nothing is
 * greyed out.
 *
 * Once a fighter is chosen the page edits two bytes of its record - the
 * ailment code and the level it is at - and shows the answer on the fighter
 * itself: the marker object is taken back out of hiding, given the script for
 * the new code out of g_btl_actor_gfx, and its attached piece given the level
 * mark, with the code plus 0x40 written into the piece's own index. Up steps
 * the level round its three values and carries into the code, which wraps at
 * the twenty-fourth; down does the same the other way. A confirm leaves the
 * fighter as edited and goes back to the pick. The member's editor also
 * singles the member out while it works, and gives the ailment two turns to
 * run when it lets go.
 *
 * Both answer zero, the way every handler on that page does, so the round
 * carries on as though nothing had been ordered.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/input.h>
#include <persona/btlp/menu.h>
#include <persona/btlp/object.h>
#include <persona/btlp/pick.h>
#include <persona/btlp/round.h>
#include <persona/btlp/sound.h>
#include <persona/btlp/stage.h>
#include <persona/btlp/status.h>

/* How many codes and levels the editor steps through. */
#define EDIT_CODES  24
#define EDIT_LEVELS 3

/* How many slots the enemies' side has, and the turns an ailment set from
   the member's editor is given. */
#define EDIT_ENEMIES 9
#define EDIT_TURNS   2

/* The marker brought up to date with the record's ailment. */
#define EDIT_SHOW_MARK(a)                                                     \
    BtlObjSetScript((a)->obj->mark,                                           \
                    *(BtlSeqStep **)(g_btl_actor_gfx                          \
                                     + *(signed char *)&(a)->c.status * 4     \
                                     + BTL_GFX_SCRIPTS));                     \
    BtlObjSetScript((a)->obj->mark->attached,                                 \
                    g_btl_ail_level_marks[*(signed char *)&(a)->c.ail_level]); \
    (a)->obj->mark->unkCE = (a)->c.status + BTL_MARK_BIAS

int BtlDebugMemberAilment(void)
{
    BtlActor *a;
    int       slot;
    int       keys;
    int       i;

    BtlDrawFrame();
    g_btl_target_slot = 0;
    for (;;) {
        switch (g_btl_step) {
        case 0:
            for (i = 0; i < BTL_PARTY; i++) {
                g_btl_actors[i].pickable = 1;
            }
            slot = BtlPickMember(&g_btl_target_slot);
            if (slot == BTL_PICK_WAIT) {
                break;
            }
            if (slot == -1) {
                BtlSePlay(1, 2);
                return 0;
            }
            BtlSePlay(1, 1);
            BtlSingleOutMember(slot);
            a = &g_btl_actors[slot];
            BtlObjClearAttr(a->obj->mark, BTL_OBJ_HIDDEN);
            EDIT_SHOW_MARK(a);
            g_btl_step++;
            break;

        case 1:
            keys = (u_short)BtlMenuKey();
            if (keys & PAD_UP) {
                BtlSePlay(1, 0);
                if ((signed char)++a->c.ail_level >= EDIT_LEVELS) {
                    a->c.ail_level = 0;
                    if ((signed char)++a->c.status >= EDIT_CODES) {
                        a->c.status = 0;
                    }
                }
                EDIT_SHOW_MARK(a);
            }
            if (keys & PAD_DOWN) {
                BtlSePlay(1, 0);
                if (--*(signed char *)&a->c.ail_level < 0) {
                    a->c.ail_level = EDIT_LEVELS - 1;
                    if (--*(signed char *)&a->c.status < 0) {
                        a->c.status = EDIT_CODES - 1;
                    }
                }
                EDIT_SHOW_MARK(a);
            }
            if (g_btl_pad1_edge & g_btl_key_confirm) {
                a->ail_turns = EDIT_TURNS;
                BtlSePlay(1, 1);
                g_btl_marker_obj[slot]->attr &= ~BTL_MARK_CHOSEN;
                BtlPartyResetGfx();
                BtlShowAilmentMarks(1);
                g_btl_step--;
            }
            break;
        }
        BtlDrawFrame();
    }
}

int BtlDebugEnemyAilment(void)
{
    BtlActor *a;
    int       slot;
    int       keys;
    int       i;

    BtlDrawFrame();
    g_btl_enemy_slot = 0;
    for (;;) {
        switch (g_btl_step) {
        case 0:
            for (i = 0; i < EDIT_ENEMIES; i++) {
                g_btl_combatants[i].pickable = 1;
            }
            slot = BtlPickEnemy(&g_btl_enemy_slot);
            if (slot == BTL_PICK_WAIT) {
                break;
            }
            if (slot == -1) {
                BtlSePlay(1, 2);
                return 0;
            }
            BtlSePlay(1, 1);
            a = &g_btl_combatants[slot];
            BtlObjClearAttr(a->obj->mark, BTL_OBJ_HIDDEN);
            EDIT_SHOW_MARK(a);
            g_btl_step++;
            break;

        case 1:
            keys = (u_short)BtlMenuKey();
            if (keys & PAD_UP) {
                BtlSePlay(1, 0);
                if ((signed char)++a->c.ail_level >= EDIT_LEVELS) {
                    a->c.ail_level = 0;
                    if ((signed char)++a->c.status >= EDIT_CODES) {
                        a->c.status = 0;
                    }
                }
                EDIT_SHOW_MARK(a);
            }
            if (keys & PAD_DOWN) {
                BtlSePlay(1, 0);
                if (--*(signed char *)&a->c.ail_level < 0) {
                    a->c.ail_level = EDIT_LEVELS - 1;
                    if (--*(signed char *)&a->c.status < 0) {
                        a->c.status = EDIT_CODES - 1;
                    }
                }
                EDIT_SHOW_MARK(a);
            }
            if (g_btl_pad1_edge & g_btl_key_confirm) {
                BtlSePlay(1, 1);
                BtlEnemiesResetGfx();
                BtlShowAilmentMarks(1);
                g_btl_step--;
            }
            break;
        }
        BtlDrawFrame();
    }
}
