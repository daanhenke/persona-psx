/* Persona 1 (JP) - a fighter picked off a side at random.  BTLP only.
 *   0x800B631C BtlPickRandomMember  0x800B6428 BtlPickRandomEnemy
 *
 * What a move that reaches a side rather than a fighter is aimed at. Each
 * walks its own five or nine slots, writes down every one that could be hit,
 * and answers one of them off a single call to rand(). Both share the same
 * scratch list, which is why only one of them can be part-way through at a
 * time - and neither ever is, because neither turns a frame over.
 *
 * A slot counts if it is occupied, is not down, is not flagged out and was
 * made pickable before the call. The party walk asks one thing more: a member
 * whose ailment has just been lifted is skipped, so the white flash is not
 * interrupted. The enemy walk has no equivalent.
 *
 * An empty list is answered with the first slot of that side rather than with
 * nothing, so a caller that does not check still has a slot it can write down.
 *
 * The ailment codes are held in locals of their own and the record table is
 * walked by a byte offset rather than by the slot, both so the set-up comes
 * out in the image's order; see the macro below.
 */
#include <decomp/types.h>
#include <rand.h>
#include <persona/btlp/actor.h>

/* Where the candidates are written down: at most nine slots, and the two
   walks take turns with it. */
extern u_char g_btl_pick_slots[];

/* One field of the record at a byte offset into the table, rather than of
   g_btl_actors[slot]. gcc works the same offset out for itself from the slot,
   but it sets it up after the list pointer where the image sets it up before,
   and no order of the four statements moves it. Spelt out it is an ordinary
   local and lands where the source puts it. */
#define ACTOR_FIELD(type, off, field)     (*(type *)((char *)g_btl_actors + (off) + (int)&((BtlActor *)0)->field))

int BtlPickRandomMember(void)
{
    u_char *p;
    int     status;
    int     down;
    int     lifted;
    int     slot;
    int     off;
    int     n;

    slot   = 0;
    n      = 0;
    down   = BTL_STATUS_DOWN;
    lifted = BTL_STATUS_LIFTED;
    off    = 0;
    p      = g_btl_pick_slots;
    do {
        if (ACTOR_FIELD(u_char, off, c.key) != 0) {
            status = ACTOR_FIELD(signed char, off, c.status);
            if (status != down
                && (ACTOR_FIELD(u_long, off, flags) & BTL_ACTOR_OUT) == 0
                && ACTOR_FIELD(u_char, off, pickable) != 0
                && status != lifted) {
                *p = slot;
                p++;
                n++;
            }
        }
        slot++;
        off += sizeof(BtlActor);
    } while (slot < BTL_PARTY);

    if (n == 0) {
        return 0;
    }
    return g_btl_pick_slots[rand() % n];
}

int BtlPickRandomEnemy(void)
{
    u_char *p;
    int     down;
    int     slot;
    int     off;
    int     n;

    slot = BTL_PARTY;
    n    = 0;
    down = BTL_STATUS_DOWN;
    off  = BTL_PARTY * sizeof(BtlActor);
    p    = g_btl_pick_slots;
    do {
        if (ACTOR_FIELD(u_char, off, c.key) != 0
            && ACTOR_FIELD(signed char, off, c.status) != down
            && (ACTOR_FIELD(u_long, off, flags) & BTL_ACTOR_OUT) == 0
            && ACTOR_FIELD(u_char, off, pickable) != 0) {
            *p = slot;
            p++;
            n++;
        }
        slot++;
        off += sizeof(BtlActor);
    } while (slot < BTL_PARTY + BTL_ENEMIES);

    if (n == 0) {
        return BTL_PARTY;
    }
    return g_btl_pick_slots[rand() % n];
}
