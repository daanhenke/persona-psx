/* Persona 1 (JP) - what a demon leaves behind.  BTLP only.
 *   0x800977BC BtlRollDefeatDrop
 *
 * Rolled once, as a demon goes down. Every PersonaData record carries the
 * item it can drop and how freely it parts with it in one halfword: the item
 * id in the low nine bits, and above them one of five rates. The rate is a
 * multiplier on sixteen plus however much luckier the fighter that landed the
 * blow is than the demon - so four times that at the most generous, an eighth
 * of it at the meanest, and a rate of four drops every time.
 *
 * The odds are a byte weighed against a byte of rand(), so the whole of the
 * luck difference past sixty is wasted at the top rate. The drop itself is
 * the one thing the roll leaves: g_btl_drop_item is cleared as the fight
 * opens and read back once it is over, which is why a second kill overwrites
 * the first one's prize.
 *
 * The item is taken from the actor the fight is resolving rather than from
 * the record the odds were worked out of. They are the same demon.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/round.h>
#include <persona/common/char.h>
#include <persona/common/persona.h>

/* How the record packs it: the item below, the rate above. */
#define DROP_RATE_SHIFT 9
#define DROP_ITEM_MASK  0x1FF

/* What the luck difference is weighed from, and the widest a roll can be. */
#define DROP_LUCK_BASE 16
#define DROP_ODDS_MAX  255

/* Which actor the fight is resolving. */
extern short g_btl_hit_slot;

void BtlRollDefeatDrop(BtlActor *by, BtlActor *dead)
{
    int odds;
    int chance;
    int luck;

    odds = g_persona_data[dead->c.key].drop >> DROP_RATE_SHIFT;
    luck = by->stat[STAT_LUCK] - dead->stat[STAT_LUCK];
    if (luck < 0) {
        luck = 0;
    }

    switch (odds) {
    case 0:
        odds = (luck + DROP_LUCK_BASE) * 4;
        break;
    case 1:
        odds = (luck + DROP_LUCK_BASE) * 2;
        break;
    case 2:
        odds = luck + DROP_LUCK_BASE;
        break;
    case 3:
        odds = (luck + DROP_LUCK_BASE) / 8;
        break;
    case 4:
        odds = DROP_ODDS_MAX;
        break;
    }

    if (odds >= 0) {
        chance = odds;
        if (chance > DROP_ODDS_MAX) {
            chance = DROP_ODDS_MAX;
        }
    } else {
        chance = 0;
    }
    odds = chance;

    if (odds >= (rand() & 0xFF)) {
        g_btl_drop_item =
            g_persona_data[g_btl_actors[g_btl_hit_slot].c.key].drop & DROP_ITEM_MASK;
    }
}
