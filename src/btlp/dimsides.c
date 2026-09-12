/* Persona 1 (JP) - putting one side of the field into the background.
 * BTLP only.
 *   0x800C5544 BtlDimParty  0x800C5600 BtlDimEnemies
 *
 * The pair BtlPickTargetMember opens a pick with, and the counterparts of
 * BtlPartyResetGfx and BtlEnemiesResetGfx: every live fighter on the side is
 * stopped where it stands and set walking toward an eighth of full brightness,
 * and its ailment marker is sent the same way so the marker does not stay lit
 * over a fighter that has gone dark.
 *
 * The two are not quite the same routine. The party's is walked by index and
 * writes the colour straight into the record; the enemies' reaches its records
 * through g_btl_combatants and hands the colour to BtlObjSetRgb, so whatever
 * each enemy is carrying goes dark with it. The enemies' also dims every
 * record whose slot is occupied, where the party's steps over anyone who is
 * down or out of the fight.
 *
 * Both re-read the marker for each channel rather than holding it: the write
 * before it is through that same pointer, so it has to be fetched again. Both
 * also reach for the record before they test whether the slot is worth
 * anything, which is where the image puts the load.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>

/* How dark a fighter in the background is drawn, and how fast it gets
   there. */
#define DIM_LEVEL 0x20
#define DIM_FADE  8

void BtlDimParty(void)
{
    BtlObj *o;
    int     i;

    i = 0;
    do {
        o = g_btl_actors[i].obj;
        if (g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
            o->motion = 0;
            o->fade = DIM_FADE;
            o->rgb_to[0] = DIM_LEVEL;
            o->rgb_to[1] = DIM_LEVEL;
            o->rgb_to[2] = DIM_LEVEL;
            o->mark->fade = DIM_FADE;
            o->mark->rgb_to[0] = DIM_LEVEL;
            o->mark->rgb_to[1] = DIM_LEVEL;
            o->mark->rgb_to[2] = DIM_LEVEL;
        }
        i++;
    } while (i < BTL_PARTY);
}

void BtlDimEnemies(void)
{
    BtlObj *o;
    int     i;

    i = 0;
    do {
        o = g_btl_combatants[i].obj;
        if (g_btl_combatants[i].c.key != 0) {
            o->motion = 0;
            o->fade = DIM_FADE;
            BtlObjSetRgb(o, DIM_LEVEL, DIM_LEVEL, DIM_LEVEL);
            o->mark->fade = DIM_FADE;
            o->mark->rgb_to[0] = DIM_LEVEL;
            o->mark->rgb_to[1] = DIM_LEVEL;
            o->mark->rgb_to[2] = DIM_LEVEL;
        }
        i++;
    } while (i < BTL_ENEMIES);
}
