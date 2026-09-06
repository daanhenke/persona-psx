/* Persona 1 (JP) - the hero's name in a message.  BTLP only.
 *   0x80068F8C BtlInsertHeroName
 *
 * Messages leave a hole for a name, and this fills it with the party member
 * whose Char key is 1. The key identifies the record rather than the slot, so
 * the search is over the party's five slots and stops at the first match; a
 * party without that member simply leaves the previous insert in place.
 */
#include <types.h>
#include <persona/btlp/actor.h>

/* The key the hero's record carries. */
#define BTL_HERO_KEY 1

/* BtlSetInsert's kinds: a number, or a string copied in. */
#define BTL_INSERT_TEXT 1

extern void BtlSetInsert(int kind, const u_char *text);

void BtlInsertHeroName(void)
{
    u_char *name;
    int     off;
    int     want;
    int     i;

    /* Three variables where one would do: the count, a byte offset for the key
       and a pointer for the name. Walking them separately is what the original
       does, and the count moves first. */
    i = 0;
    want = BTL_HERO_KEY;
    name = g_btl_actors[0].c.name;
    off = 0;
    do {
        i++;
        if (((BtlActor *)((char *)g_btl_actors + off))->c.key == want) {
            BtlSetInsert(BTL_INSERT_TEXT, name);
            return;
        }
        name += sizeof(BtlActor);
        off += sizeof(BtlActor);
    } while (i < BTL_PARTY);
}
