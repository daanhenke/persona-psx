/* Persona 1 (JP) - does this ailment stop the actor?  BTLP only.
 *   0x80093A58 BtlStatusStops
 *
 * One 32-bit mask per kind of fighter, with a bit for each ailment code that
 * still lets it take its turn. An actor is stopped when its own status is not
 * among them - the caller clears its turn on the answer.
 *
 * Both bytes are read signed. The shift takes only the low five bits of the
 * ailment, so a code past 31 would wrap round onto another bit; nothing in the
 * game gets near that.
 */
#include <types.h>
#include <persona/btlp/actor.h>

extern const u_long g_btl_status_allowed[];

int BtlStatusStops(const BtlActor *a)
{
    return (g_btl_status_allowed[(signed char)a->c.kind] &
            1 << (signed char)a->c.status) == 0;
}
