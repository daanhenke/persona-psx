/* Persona 1 (JP) - whether an item is worth using on anybody at all.
 *   DNG 0x8008F1E4
 *
 * One unit in the image with ItemUsableOn (common/game/status.c) ahead of it
 * and the party loops (common/game/partystatus.c) and CharBelowMax
 * (charbelowmax.c) behind; the objects here follow the older cut. ADV carries
 * the same routine at 0x80094910, still in asm; once it takes this source the
 * unit belongs in src/common, with ADV's reads of the three flag bytes (which
 * it names) pinned free of the names.
 */
#include <decomp/types.h>
#include <persona/common/status.h>

/* Picks which of the two over-the-clock events runs. */
#define CLOCK_EVENT_ALT (*(u_char *)0x801F29A8)
/* The steps left on the walking effect an item started. */
#define EFFECT_STEPS    (*(u_char *)0x801F29A9)
/* The moon's phase; 0 is the new moon. */
#define MOON_PHASE      (*(u_char *)0x801F2B30)

/* Ailments the two cure items clear (as in ItemUsableOn). */
#define STATUS_CURE_67 STATUS_POISON
#define STATUS_CURE_6A STATUS_SICK

/* The item menu's greying test: 0x5F..0x64 restore HP, so they are useful
   while anybody is short of it; 0x67 and 0x6A each cure one ailment. 0x6F
   needs a moon in the sky and the first clock event, and 0x73 cannot be
   used while the effect of an earlier one is still counting down. */
u_char ItemUsableAny(short item)
{
    u_char ok;

    ok = 0;
    switch (item) {
    case 0x5F:
    case 0x60:
    case 0x61:
    case 0x62:
    case 0x63:
    case 0x64:
        ok = PartyAnyBelowMax(BELOW_HP);
        break;
    case 0x67:
        ok = PartyAnyStatus(STATUS_CURE_67);
        break;
    case 0x6A:
        ok = PartyAnyStatus(STATUS_CURE_6A);
        break;
    case 0x6F:
        if (MOON_PHASE != 0 && CLOCK_EVENT_ALT == 0) {
            ok++;
        }
        break;
    case 0x73:
        if (EFFECT_STEPS == 0) {
            ok++;
        }
        break;
    }
    return ok;
}
