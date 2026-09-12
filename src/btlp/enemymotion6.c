/* Persona 1 (JP) - the motion an enemy acts on.  BTLP only.
 *   0x800B4A88 BtlEnemyMotion06
 *
 * Entry 6 of g_btl_enemy_motion, and the only thing in it that decides
 * anything: it works out which of the four routines in g_btl_enemy_attack
 * plays the move out, and hands over to it. A row that is empty leaves the
 * record where it is, with its phase back at zero.
 *
 * Four answers, in the order they are ruled out:
 *
 *   the move has no effect at all   the move is replaced with a plain swing
 *                                   and the first routine takes it
 *   the move is 0xDB or 0xE0        the first routine as well
 *   the move is 0xA3 to 0xE3        the second
 *   anything else                   the species says, out of a byte per
 *                                   the record's own interruption index
 *
 * and then one species overrules all of it: 0xC2 always takes the fourth.
 *
 * The species record is left in g_btl_species_now on the way past, because
 * what runs next reads it from there rather than working it out again.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>
#include <persona/btlp/spellfx.h>

/* The species table, and the record this motion leaves behind. Both are the
   0x30-byte record g_btl_models is the other view of; only a byte well inside
   it is wanted here, so it is walked as bytes. */
extern const u_char  g_btl_species[];
extern const u_char *g_btl_species_now;
#define BTL_SPECIES_BYTES 0x30

/* Where the per-species answer sits in that record. */
#define SPECIES_ATTACK 0x16

/* The four routines, and the plain swing a move with no effect falls back
   on. */
extern void (*g_btl_enemy_attack[])(void);
#define ENEMY_MOVE_SWING 1

/* The moves the second routine plays out - everything from 0xA3 up - and the
   two inside that range which are not its. */
#define ENEMY_MOVE_SPLIT 0xA3
#define ENEMY_MOVE_SPAN  0x41
#define ENEMY_MOVE_PLAIN 0xDB
#define ENEMY_MOVE_PLAIN2 0xE0

/* The one species that plays every move the same way. */
#define ENEMY_SPECIES_FIXED 0xC2
#define ENEMY_ATTACK_FIXED  3

void BtlEnemyMotion06(BtlObj *o)
{
    BtlActor *a;
    int       which;

    a = o->actor;
    g_btl_species_now = &g_btl_species[o->kind * BTL_SPECIES_BYTES];

    if (g_btl_spell_fx[a->unkBD].start == 0) {
        a->move = ENEMY_MOVE_SWING;
        which = 0;
    } else {
        if (a->unkBD == ENEMY_MOVE_PLAIN2 || a->unkBD == ENEMY_MOVE_PLAIN) {
            which = 0;
        } else if ((u_int)(a->unkBD - ENEMY_MOVE_SPLIT) < ENEMY_MOVE_SPAN) {
            which = 1;
        } else {
            which = (g_btl_species_now + a->unkBA)[SPECIES_ATTACK];
        }
        if (o->kind == ENEMY_SPECIES_FIXED) {
            which = ENEMY_ATTACK_FIXED;
        }
    }

    if (g_btl_enemy_attack[which] != 0) {
        g_btl_enemy_attack[which]();
    } else {
        o->phase = 0;
    }
}
