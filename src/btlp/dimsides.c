/* Persona 1 (JP) - putting one side of the field into the background.
 * BTLP only.
 *   0x800C5544 BtlDimParty  0x800C5600 BtlDimEnemies
 *   0x800C56CC BtlSingleOutMember
 *
 * BtlSingleOutMember puts one member in front of the rest of the party. Every
 * other live member has its palettes put back from the base copy and is sent
 * dark with its marker, and its marker frame loses BTL_MARK_CHOSEN; the member
 * singled out is put on motion 10, lit at once with its marker, and its frame
 * gains the bit.
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
#include <decomp/libc.h>
#include <persona/btlp/battle.h>
#include <persona/btlp/object.h>
#include <persona/btlp/clut.h>

/* A palette is 0x200 bytes, one page per actor. */
#define CLUT_BYTES 0x200

/* The member singled out: its motion, how bright it and its marker are drawn
   and how fast they get there, the fade the others are dimmed at, and the bit
   its marker frame carries. */
#define SINGLE_MOTION   10
#define SINGLE_LIT      0x80
#define SINGLE_LIT_FADE 0xFF
#define SINGLE_DIM_FADE 4

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

void BtlSingleOutMember(int slot)
{
    BtlObj *o;
    int     i;

    for (i = 0; i < BTL_PARTY; i++) {
        if (g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0) {
            o = g_btl_actors[i].obj;
            if (i != slot) {
                memcpy((u_char *)g_btl_actor_clut + i * CLUT_BYTES,
                       (u_char *)g_btl_actor_clut_base + i * CLUT_BYTES, CLUT_BYTES);
                memcpy((u_char *)g_btl_actor_clut_to + i * CLUT_BYTES,
                       (u_char *)g_btl_actor_clut_base + i * CLUT_BYTES, CLUT_BYTES);
                o->motion = 0;
                o->fade = SINGLE_DIM_FADE;
                o->rgb_to[0] = DIM_LEVEL;
                o->rgb_to[1] = DIM_LEVEL;
                o->rgb_to[2] = DIM_LEVEL;
                o->mark->fade = SINGLE_DIM_FADE;
                o->mark->rgb_to[0] = DIM_LEVEL;
                o->mark->rgb_to[1] = DIM_LEVEL;
                o->mark->rgb_to[2] = DIM_LEVEL;
                g_btl_marker_obj[i]->attr &= ~BTL_MARK_CHOSEN;
            } else {
                o->motion = SINGLE_MOTION;
                o->fade = SINGLE_LIT_FADE;
                o->rgb_to[0] = SINGLE_LIT;
                o->rgb_to[1] = SINGLE_LIT;
                o->rgb_to[2] = SINGLE_LIT;
                o->mark->motion = 0;
                o->mark->fade = SINGLE_LIT_FADE;
                o->mark->rgb_to[0] = SINGLE_LIT;
                o->mark->rgb_to[1] = SINGLE_LIT;
                o->mark->rgb_to[2] = SINGLE_LIT;
                g_btl_marker_obj[i]->attr |= BTL_MARK_CHOSEN;
            }
        }
    }
}
