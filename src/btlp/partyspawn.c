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
extern u_char   *g_btl_enemy_gfx_start;

extern u_short  BtlLoadMemberGfx(int member, int actor);
extern BtlObj  *BtlSpawnMemberObj(int key, int col, int row, short gfx,
                                  int member);
extern BtlObj  *BtlSpawnActorObj(int model, const long *pos);
extern void     BtlPartyResetGfx(void);

/* One counter walks every member loop and the grid, and the column counter
   also searches for a free cell - the image keeps each in one saved register.
   The flags are cleared through a pointer of their own. The script row is
   taken the way the other member motions take it, and the hand test is
   written as the && the branches show. */
void BtlSpawnParty(void)
{
    BtlActor *a;
    BtlObj   *obj;
    int       key;
    int       member;
    int       i;
    int       col;
    BtlActor *p;
    const u_char *set;
    int       row;

    i = BTL_MEMBERS - 1;
    p = &g_btl_actors[BTL_MEMBERS - 1];
    do {
        p->flags = 0;
        i--;
        p--;
    } while (i >= 0);

    for (i = 0; i < BTL_MEMBERS; i++) {
        if (g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status == BTL_STATUS_DEAD) {
            for (col = 0; col < GRID_CELLS; col++) {
                if (g_btl_formation[col] == CELL_EMPTY) {
                    g_btl_formation[col] = i;
                    break;
                }
            }
        }
    }

    i = 0;
    for (row = 0; row < GRID_W; row++) {
        for (col = 0; col < GRID_W; col++) {
            member = g_btl_formation[i];
            if (member != 0xFF) {
                a = &g_btl_actors[member];
                key = a->c.key;
                if (a->c.equip[0] != BTL_HAND_EMPTY
                    && a->c.equip[0] != BTL_HAND_A
                    && a->c.equip[0] != BTL_HAND_B) {
                    a->script_pick = 1;
                } else {
                    a->script_pick = 0;
                }
                obj = BtlSpawnMemberObj(key, col << 1, row,
                                        BtlLoadMemberGfx(key, member), member);
                a->obj = obj;
                obj->mark_num = member;
                a->obj->actor = &g_btl_actors[member];
                a->obj->mark = BtlSpawnActorObj(
                    *(signed char *)&a->c.status, &a->obj->x);
                a->obj->mark->shift_x = BTL_SHADOW_X;
                a->obj->mark->shift = BTL_SHADOW_Y;
                a->obj->mark->attached->shift_x = BTL_SHADOW_X2;
                a->obj->mark->attached->shift = BTL_SHADOW_Y;

                if ((signed char)a->c.status == BTL_STATUS_DEAD) {
                    set = &g_btl_member_scripts[key * BTL_SCRIPT_ROW];
                    BtlObjSetScript(a->obj,
                        a->obj->scripts[set[a->script_pick * BTL_SCRIPT_SET]]);
                    a->obj->attr |= BTL_OBJ_HIDDEN;
                    a->obj->shadow->attr |= BTL_OBJ_HIDDEN;
                    a->obj->mark->attr |= BTL_OBJ_HIDDEN;
                    a->obj->mark->attached->attr |= BTL_OBJ_HIDDEN;
                } else {
                    set = &g_btl_member_scripts[key * BTL_SCRIPT_ROW];
                    BtlObjSetScript(a->obj,
                        a->obj->scripts[set[a->script_pick * BTL_SCRIPT_SET]]);
                }
                BtlApplyPersona(a);
                BtlRecalcStats(a);

                /* Everything the battle keeps on a member, back to nothing. */
                *(int *)&a->casts = 0;
                a->flags = 0;
                a->unkCC = 0;
                a->damage_dealt = 0;
                a->stage[0] = 0;
                a->stage[1] = 0;
                a->stage[2] = 0;
                a->stage[6] = 0;
                a->stage[3] = 0;
                a->stage[4] = 0;
                a->stage[5] = 0;
                a->stage[6] = 0;
                a->level_up = 0;
                a->unk56_up = 0;
                a->unk74 = 0;
                a->unk78 = 0;
                a->unkD0 = 0;
                a->wound = 0;
                a->build = 0;
                a->unkD3 = 0;
                a->unkD4 = 0;
                a->counter = 0;
                a->revive_mark = 0;
                a->revive_slot = 0;
                a->unkDF = 0;
                *(int *)&a->won_share = 0;
                a->marker = 0;
                a->resume_motion = 0;
                a->resume_phase = 0;
                BtlDeriveBattleStats(a);
            }
            i++;
        }
    }

    for (i = 0; i < BTL_MEMBERS; i++) {
        if (g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status == BTL_STATUS_DEAD) {
            g_btl_formation[g_btl_actors[i].obj->row * GRID_W
                            + (g_btl_actors[i].obj->col2 >> 1)] = 0xFF;
        }
    }

    g_btl_enemy_gfx_start = g_btl_gfx_next;
    DrawSync(0);
    BtlPartyResetGfx();
}

