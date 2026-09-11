/* Persona 1 (JP) - the move whose effect sweeps across the far side.
 * BTLP only.
 *   0x800B77B0 BtlFxStartSweep
 *
 * Fifteen records - five across and three deep - laid over the side the
 * acting fighter is not on, at the same thirty pixels a column the fighters
 * stand on and two of their rows apart.
 *
 * Where BtlOpenFxGrid scatters its cells out of a table, this one sweeps: the
 * row nearest the caster arrives first and each one behind it eight frames
 * later, which is why the row's timer is counted from the other end when it is
 * an enemy acting.
 *
 * Everything the move reaches is put on the effect's blue before the first
 * record is made.
 */
#include <decomp/types.h>
#include <decomp/include_asm.h>
#include <persona/btlp/formation.h>
#include <persona/btlp/object.h>
#include <persona/btlp/round.h>
#include <persona/btlp/spellfx.h>

/* The blue every fighter the move reaches is walked to. */
#define FX_SWEEP_R 0
#define FX_SWEEP_G 0x80
#define FX_SWEEP_B 0xFF

/* The sheet: five of the field's columns, three rows two apart. */
#define FX_SWEEP_W 5
#define FX_SWEEP_H 3

#define FX_SWEEP_X0 (PLACE_COL_ORG * PLACE_FIXED)
#define FX_SWEEP_DX (PLACE_COL_W * PLACE_FIXED)
#define FX_SWEEP_DY (2 * PLACE_ROW_H * PLACE_FIXED)

/* Where each side's sheet starts: the party's back row, and the enemies'
   front one. */
#define FX_SWEEP_Y_PARTY \
    ((PLACE_ROW_ORG + 4 * PLACE_ROW_H) * PLACE_FIXED)
#define FX_SWEEP_Y_ENEMY (PLACE_COL_ORG * PLACE_FIXED)

/* Which of a set each record stands for, and how far apart the rows are in
   time. */
#define FX_MARK_SWEEP 0x10
#define FX_SWEEP_STEP 8

#ifdef NON_MATCHING
BtlObj *BtlFxStartSweep(void)
{
    BtlObj *o;
    BtlObj *after;
    long    pos[3];
    long    x;
    long    y_party;
    long    y_enemy;
    int     when;
    int     row;
    int     col;
    int     cell;

    row   = FX_SWEEP_H - 1;
    cell  = FX_SWEEP_W * FX_SWEEP_H - 1;
    after = 0;

    BtlTintTargets(g_btl_actor_turn, FX_SWEEP_R, FX_SWEEP_G, FX_SWEEP_B);

    g_btl_fx_def.scripts = ((const u_long ***)g_btl_unused_gfx)[0];
    y_party = FX_SWEEP_Y_PARTY;
    y_enemy = FX_SWEEP_Y_ENEMY;

    for (; row >= 0; row--)
    {
        col = 0;
        x   = FX_SWEEP_X0;
        for (; col < FX_SWEEP_W; col++)
        {
            if (g_btl_actor_turn < BTL_PARTY)
            {
                pos[0] = x;
                pos[1] = y_enemy;
            }
            else
            {
                pos[0] = x;
                pos[1] = y_party;
            }
            pos[2] = 0;

            o = BtlObjAlloc(&g_btl_fx_def, FX_OBJ_GROUP, after, FX_OBJ_DRAW, 0,
                            pos, FX_OBJ_CD, FX_OBJ_CE);
            o->mark_num = cell + FX_MARK_SWEEP;
            o->attr     = FX_OBJ_ATTR;
            o->attached = after;
            /* The row's own delay, taken into a local of its own: written
               straight into the record the compiler lifts it out of the
               column loop, where the image works it out afresh each time. */
            if (g_btl_actor_turn < BTL_PARTY)
            {
                when = (FX_SWEEP_H - 1 - row) * FX_SWEEP_STEP;
            }
            else
            {
                when = row * FX_SWEEP_STEP;
            }
            o->timer = when;
            after = o;
            x    += FX_SWEEP_DX;
            cell--;
        }
        y_party -= FX_SWEEP_DY;
        y_enemy -= FX_SWEEP_DY;
    }
    return o;
}
#else
INCLUDE_ASM("btlp/nonmatchings/fxsweep", BtlFxStartSweep);
#endif
