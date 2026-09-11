/* Persona 1 (JP) - is anybody still moving.  BTLP only.
 *   0x800C5ECC BtlActorsIdle  0x800C5F60 BtlActorSlotByKey
 *
 * BtlActorsIdle is what every wait in the overlay turns the frame over on. It
 * walks both sides - fourteen records, the party and the enemies together -
 * and answers no the moment it finds a fighter whose object is on a motion.
 * A slot nobody is in, a fighter who is down and one flagged out of the fight
 * are all skipped rather than counted, so a field of corpses reads as still.
 *
 * BtlActorSlotByKey is the other way round the party is reached: given the
 * key a character record carries, it answers which of the five places that
 * character is standing in. A key nobody has answers place zero, not a
 * failure - every caller has already established that the character is there.
 */
#include <decomp/types.h>
#include <persona/btlp/actor.h>
#include <persona/btlp/object.h>

/* Party members, against both sides at once. */
#define BTL_PARTY 5

int BtlActorsIdle(void)
{
    int i;

    i = 0;
    do {
        if (g_btl_actors[i].c.key != 0
            && (signed char)g_btl_actors[i].c.status != BTL_STATUS_DOWN
            && (g_btl_actors[i].flags & BTL_ACTOR_OUT) == 0
            && g_btl_actors[i].obj->motion != 0) {
            return 0;
        }
        i++;
    } while (i < BTL_ACTORS);
    return 1;
}

int BtlActorSlotByKey(u_int key)
{
    int i;

    i = 0;
    do {
        if (g_btl_actors[i].c.key == key) {
            return i;
        }
        i++;
    } while (i < BTL_PARTY);
    return 0;
}
