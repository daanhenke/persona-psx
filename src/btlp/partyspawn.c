/* Persona 1 (JP) - putting the party on the field.  BTLP only.
 *   0x80085898 BtlSpawnParty
 *
 * Run once from ovl_btlp_entry. The five actor flag words are cleared, then any
 * member already dead is appended to the first free place in g_btl_formation,
 * so the dead stand where nobody else does.
 *
 * The formation is walked as a five by five grid. A place naming a member picks
 * which of its two script sets to use from the member's first equipment entry,
 * loads its graphics into a VRAM slot, spawns the object at that place with a
 * shadow beside it, points the object back at the actor, and clears everything
 * the battle keeps on a member. A dead member is spawned hidden and its square
 * given back at the end.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <libgte.h>
#include <libgpu.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/formation.h>

/* Five members, and the formation grid they stand on. */
#define BTL_MEMBERS      5

/* Char.status for a member who is already down. */
#define BTL_STATUS_DEAD 0x11

/* Which of the two script sets a member uses: the first for an empty hand and
   for these two entries, the second for anything else. */
#define BTL_HAND_EMPTY 0
#define BTL_HAND_A     0xA3
#define BTL_HAND_B     0xAF

/* Ten scripts a set, four sets a member. */
#define BTL_SCRIPT_SET  10
#define BTL_SCRIPT_ROW  0x28

/* Where a member's shadow sits relative to the member. */
#define BTL_SHADOW_X   0x80000
#define BTL_SHADOW_Y   0xFFD00000
#define BTL_SHADOW_X2  0x140000

extern u_char    g_btl_member_scripts[];
extern u_char   *g_btl_gfx_next;
extern u_char   *g_btl_enemy_gfx_start;

extern u_short  BtlLoadMemberGfx(int member, int actor);
extern BtlObj  *BtlSpawnMemberObj(int key, int col, int row, int gfx,
                                  int member);
extern BtlObj  *BtlSpawnActorObj(int model, const long *pos);
extern void     BtlApplyPersona(BtlActor *a);
extern void     BtlRecalcStats(BtlActor *a);
extern void     BtlPartyResetGfx(void);

#ifdef NON_MATCHING
void BtlSpawnParty(void)
{
    BtlActor *a;
    BtlObj   *obj;
    int       key;
    int       member;
    int       i;
    int       n;
    int       col;
    int       row;

    i = BTL_MEMBERS - 1;
    a = &g_btl_actors[BTL_MEMBERS - 1];
    do {
        a->flags = 0;
        i--;
        a--;
    } while (i >= 0);

    member = 0;
    i = 0;
    do {
        if (g_btl_actors[member].c.key != 0
            && g_btl_actors[member].c.status == BTL_STATUS_DEAD) {
            for (n = 0; n < GRID_CELLS; n++) {
                if (g_btl_formation[n] == CELL_EMPTY) {
                    g_btl_formation[n] = member;
                    break;
                }
            }
        }
        member++;
        i += sizeof(BtlActor);
    } while (member < BTL_MEMBERS);

    i = 0;
    for (row = 0; row < GRID_W; row++) {
        for (col = 0; col < GRID_W; col++) {
            member = g_btl_formation[i];
            if (member != 0xFF) {
                a = &g_btl_actors[member];
                key = a->c.key;
                if (a->c.equip[0] == BTL_HAND_EMPTY
                    || a->c.equip[0] == BTL_HAND_A
                    || a->c.equip[0] == BTL_HAND_B) {
                    a->script_pick = 0;
                } else {
                    a->script_pick = 1;
                }
                obj = BtlSpawnMemberObj(key, col << 1, row,
                                        BtlLoadMemberGfx(key, member), member);
                a->obj = obj;
                obj->mark_num = member;
                a->obj->actor = a;
                a->obj->mark = BtlSpawnActorObj(
                    *(signed char *)&a->c.status, &a->obj->x);
                a->obj->mark->shift_x = BTL_SHADOW_X;
                a->obj->mark->shift = BTL_SHADOW_Y;
                a->obj->mark->attached->shift_x = BTL_SHADOW_X2;
                a->obj->mark->attached->shift = BTL_SHADOW_Y;

                if (a->c.status == BTL_STATUS_DEAD) {
                    BtlObjSetScript(a->obj,
                        a->obj->scripts[g_btl_member_scripts[
                            a->script_pick * BTL_SCRIPT_SET
                            + key * BTL_SCRIPT_ROW]]);
                    a->obj->attr |= BTL_OBJ_HIDDEN;
                    a->obj->shadow->attr |= BTL_OBJ_HIDDEN;
                    a->obj->mark->attr |= BTL_OBJ_HIDDEN;
                    a->obj->mark->attached->attr |= BTL_OBJ_HIDDEN;
                } else {
                    BtlObjSetScript(a->obj,
                        a->obj->scripts[g_btl_member_scripts[
                            a->script_pick * BTL_SCRIPT_SET
                            + key * BTL_SCRIPT_ROW]]);
                }
                BtlApplyPersona(a);
                BtlRecalcStats(a);

                /* Everything the battle keeps on a member, back to nothing. */
                *(int *)&a->pad68[8] = 0;
                a->flags = 0;
                a->unkCC = 0;
                *(int *)&a->pad68[0] = 0;
                a->offered = 0;
                a->unkE1[0] = 0;
                a->unkE1[1] = 0;
                a->unkE1[5] = 0;
                a->unkE1[2] = 0;
                a->unkE1[3] = 0;
                a->unkE1[4] = 0;
                a->unkE1[5] = 0;
                a->padCD[0] = 0;
                a->padCD[1] = 0;
                a->unk74 = 0;
                *(int *)&a->pad78[0] = 0;
                a->padCD[3] = 0;
                a->padCD[4] = 0;
                a->unkD2 = 0;
                a->unkD3 = 0;
                a->unkD4 = 0;
                a->unkD5 = 0;
                a->unkDC = 0;
                a->unkDB = 0;
                a->unkDF = 0;
                *(int *)&a->pad68[4] = 0;
                a->marker = 0;
                a->unkC5 = 0;
                a->unkC6 = 0;
                BtlDeriveBattleStats(a);
            }
            i++;
        }
    }

    member = 0;
    i = 0;
    do {
        member++;
        if (g_btl_actors[i].c.key != 0
            && g_btl_actors[i].c.status == BTL_STATUS_DEAD) {
            g_btl_formation[g_btl_actors[i].obj->row * GRID_W
                            + (g_btl_actors[i].obj->col2 >> 1)] = 0xFF;
        }
        i++;
    } while (member < BTL_MEMBERS);

    g_btl_enemy_gfx_start = g_btl_gfx_next;
    DrawSync(0);
    BtlPartyResetGfx();
}
#else
INCLUDE_ASM("btlp/nonmatchings/partyspawn", BtlSpawnParty);
#endif

