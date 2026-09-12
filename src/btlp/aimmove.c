/* Persona 1 (JP) - working out what a chosen move hits.  BTLP only.
 *   0x800B5E18 BtlAimMove
 *
 * Between choosing a move and playing it out, every fighter's action has to be
 * given an order - the one fighter it is aimed at - and a target mask. The
 * menu does that for a member it has put a cursor on; this does it for
 * everything that picks its own, which is every enemy action and every member
 * action the player was not asked about.
 *
 * The move id says which side it reaches before the record does. Ids 0xA3 to
 * 0xE3 and the free spells from SPELL_FREE_FIRST are spells and go through the
 * record's aim nibble; everything else is a weapon swing and goes through its
 * kind. Move 0 is a spell too, and 0xDB is a spell id that is swung.
 *
 * 0xE1 is aimed twice over - once by hand and then again by its aim nibble,
 * because the test that follows is a second `if` rather than an `else`.
 *
 * 0xE0 is the one that is not aimed at anybody in particular: it rolls one of
 * thirty-two outcomes and keeps it on the record. Nought does nothing at all,
 * one turns the whole enemy side on and takes the acting fighter's own slot
 * back out of it - answering nothing if that leaves nobody - and the rest aim
 * at a single member like everything else.
 *
 * The two halves of the pickable pair go together: BtlSetPartyPickable with
 * BtlPickRandomMember for the party, BtlSetPickable with BtlPickRandomEnemy
 * for the enemies.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/battle.h>
#include <persona/common/spell.h>

/* Move ids that are spells: this many from the first, plus the free ones. */
#define MOVE_SPELL_FIRST 0xA3
#define MOVE_SPELL_COUNT 0x41
#define SPELL_FREE_COUNT 0x17

/* The three moves aimed by hand. 0xDB is inside the spell range and swung
   anyway; 0xE1 picks a member; 0xE0 rolls for what it does. */
#define MOVE_SWUNG_SPELL 0xDB
#define MOVE_ROLLED      0xE0
#define MOVE_AIMED_ONE   0xE1

/* What 0xE0 rolls between, and the two outcomes it handles itself. */
#define ROLL_OUTCOMES 32
#define ROLL_NOTHING  0
#define ROLL_SIDE     1

/* The nibble of SpellData.aim that says how the move is aimed. */
#define AIM_MASK  0xF
#define AIM_ONE   1
#define AIM_ONE2  8
#define AIM_SIDE  2
#define AIM_SIDE2 4

/* Which set of targets the record can reach - SpellData.target. */
#define REACH_ONE  0
#define REACH_SIDE 4

/* The four kinds of swing that reach across the fight rather than one
   fighter. The same four BtlChooseEnemyMove always counts as worth making. */
#define KIND_1A 0x1A
#define KIND_1C 0x1C
#define KIND_1E 0x1E
#define KIND_32 0x32

/* Swings that are aimed the ordinary way whatever their kind says. */
#define MOVE_PLAIN_FIRST 0x53
#define MOVE_PLAIN_COUNT 3
#define MOVE_PLAIN_ALSO  0x5B

/* The image reads the move byte again where an ordinary read would be folded
   into the one before it - twice in the range test at the top, and again at
   the head of the swing. Nothing can change it in between, so this is only a
   way of telling gcc not to keep it; the reads are load-bearing all the same,
   and without them the routine is four instructions short. */
#define BTL_MOVE(a) (*(volatile u_char *)&(a)->move)

extern int  BtlPickAiTarget(BtlActor *a, int reach);
extern int  BtlPickableMask(void);
extern void BtlSetPickable(void);
extern int  BtlFrontMemberOrder(BtlActor *by);
extern int  BtlPickRandomMember(void);
extern int  BtlPickRandomEnemy(void);
extern void func_80094A20(BtlActor *at, int reach);
extern int  func_80094C40(void);

void BtlAimMove(BtlActor *a)
{
    BtlActor *e;
    u_char    move;
    u_char    id;
    int       slot;
    int       roll;
    int       reach;
    int       i;

    /* The two ends of the range test are weighed against a local taken once,
       and the two in the middle against a second read; that is the image's
       pair of loads rather than the one gcc would otherwise keep. */
    move = a->move;
    if (move != 0
        && (u_int)((id = BTL_MOVE(a)) - SPELL_FREE_FIRST) >= SPELL_FREE_COUNT) {
        if ((u_int)(u_char)(id - MOVE_SPELL_FIRST) >= MOVE_SPELL_COUNT
            || move == MOVE_SWUNG_SPELL) {
            goto swing;
        }
    }

    if (a->move == MOVE_AIMED_ONE) {
        BtlSetPartyPickable();
        slot = BtlPickRandomMember();
        a->order = slot;
        a->targets = 1 << slot;
    }
    if (a->move == MOVE_ROLLED) {
        roll = rand() % ROLL_OUTCOMES;
        a->unkD8 = roll;
        if (roll == ROLL_NOTHING) {
            return;
        }
        if (roll == ROLL_SIDE) {
            BtlSetPickable();
            a->order = 0;
            i = BTL_PARTY;
            e = g_btl_enemies;
            do {
                if (e->pickable != 0) {
                    if (a->obj->mark_num != i) {
                        a->order = i;
                    } else {
                        e->pickable = 0;
                    }
                }
                i++;
                e++;
            } while (i < BTL_PARTY + BTL_ENEMIES);
            if (a->order == 0) {
                a->unkD8 = 0;
                return;
            }
            a->targets = BtlPickableMask();
            return;
        }
        BtlSetPartyPickable();
        a->order = BtlPickRandomMember();
        a->targets = func_80094C40();
        return;
    }

    switch (g_spell_data[a->move].aim & AIM_MASK) {
    case AIM_ONE:
    case AIM_ONE2:
        BtlPickAiTarget(a, g_spell_data[a->move].target);
        slot = BtlPickRandomMember();
        a->order = slot;
        a->targets = 1 << slot;
        break;
    case AIM_SIDE:
    case AIM_SIDE2:
        BtlPickAiTarget(a, g_spell_data[a->move].target);
        a->order = BtlFrontMemberOrder(a);
        a->targets = func_80094C40();
        break;
    }
    return;

swing:
    move = BTL_MOVE(a);
    switch (g_spell_data[move].kind & SPELL_KIND_MASK) {
    case KIND_1A:
    case KIND_1C:
    case KIND_1E:
    case KIND_32:
        if ((u_int)(BTL_MOVE(a) - MOVE_PLAIN_FIRST) < MOVE_PLAIN_COUNT) {
            break;
        }
        move = a->move;
        if (move == MOVE_PLAIN_ALSO) {
            break;
        }
        switch (g_spell_data[move].target) {
        case REACH_ONE:
            BtlSetPickable();
            slot = BtlPickRandomEnemy();
            a->order = slot;
            a->targets = 1 << slot;
            return;
        case REACH_SIDE:
            BtlSetPickable();
            a->order = BtlPickRandomEnemy();
            a->targets = BtlPickableMask();
            return;
        }
        return;
    }

    move = a->move;
    reach = g_spell_data[move].target;
    if (move == MOVE_SWUNG_SPELL) {
        reach = 0;
    }
    switch (reach) {
    case REACH_ONE:
        BtlSetPartyPickable();
        slot = BtlPickRandomMember();
        a->order = slot;
        a->targets = 1 << slot;
        break;
    case REACH_SIDE:
        BtlSetPartyPickable();
        a->order = BtlPickRandomMember();
        a->targets = func_80094C40();
        break;
    default:
        BtlSetPartyPickable();
        slot = BtlPickRandomMember();
        func_80094A20(&g_btl_actors[slot], g_spell_data[a->move].target);
        a->order = slot;
        a->targets = func_80094C40();
        break;
    }
}
